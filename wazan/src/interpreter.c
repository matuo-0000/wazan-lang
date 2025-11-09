#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

Interpreter* interpreter_create() {
    Interpreter *interp = (Interpreter*)calloc(1, sizeof(Interpreter));
    return interp;
}

void interpreter_free(Interpreter *interp) {
    if (interp) {
        free(interp);
    }
}

static int find_variable(Interpreter *interp, const char *name) {
    for (int i = 0; i < interp->varCount; i++) {
        if (strcmp(interp->variables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void set_variable(Interpreter *interp, const char *name, int value) {
    int idx = find_variable(interp, name);
    if (idx >= 0) {
        interp->variables[idx].value = value;
    } else {
        if (interp->varCount < MAX_VARIABLES) {
            strcpy(interp->variables[interp->varCount].name, name);
            interp->variables[interp->varCount].value = value;
            interp->variables[interp->varCount].isArray = 0;
            interp->variables[interp->varCount].arraySize = 0;
            interp->varCount++;
        }
    }
}

static void set_array(Interpreter *interp, const char *name, int size, int *values) {
    int idx = find_variable(interp, name);
    if (idx >= 0) {
        interp->variables[idx].isArray = 1;
        interp->variables[idx].arraySize = size;
        for (int i = 0; i < size && i < MAX_ARRAY_SIZE; i++) {
            interp->variables[idx].arrayValues[i] = values[i];
        }
    } else {
        if (interp->varCount < MAX_VARIABLES) {
            strcpy(interp->variables[interp->varCount].name, name);
            interp->variables[interp->varCount].isArray = 1;
            interp->variables[interp->varCount].arraySize = size;
            for (int i = 0; i < size && i < MAX_ARRAY_SIZE; i++) {
                interp->variables[interp->varCount].arrayValues[i] = values[i];
            }
            interp->varCount++;
        }
    }
}

static int get_array_element(Interpreter *interp, const char *name, int index) {
    int idx = find_variable(interp, name);
    if (idx >= 0 && interp->variables[idx].isArray) {
        if (index >= 0 && index < interp->variables[idx].arraySize) {
            return interp->variables[idx].arrayValues[index];
        }
    }
    return 0;
}

static int get_variable(Interpreter *interp, const char *name) {
    int idx = find_variable(interp, name);
    if (idx >= 0) {
        return interp->variables[idx].value;
    }
    
    // 漢数字チェック
    int num = kanji_to_int(name);
    if (num >= 0) return num;
    
    return 0;
}

static int find_function(Interpreter *interp, const char *name) {
    for (int i = 0; i < interp->funcCount; i++) {
        if (strcmp(interp->functions[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void define_function(Interpreter *interp, const char *name, 
                           const char *param1, const char *param2, 
                           const char *body) {
    if (interp->funcCount < MAX_FUNCTIONS) {
        strcpy(interp->functions[interp->funcCount].name, name);
        strcpy(interp->functions[interp->funcCount].params[0], param1);
        strcpy(interp->functions[interp->funcCount].params[1], param2);
        strcpy(interp->functions[interp->funcCount].body, body);
        interp->functions[interp->funcCount].paramCount = 2;
        interp->funcCount++;
    }
}

static int evaluate_expression(Interpreter *interp, ASTNode *node) {
    if (!node) return 0;
    
    switch (node->type) {
        case AST_NUMBER:
            return node->value;
            
        case AST_IDENTIFIER:
            return get_variable(interp, node->name);
            
        case AST_ARRAY_ACCESS: {
            int index = evaluate_expression(interp, node->index);
            return get_array_element(interp, node->name, index - 1); // 1-based index
        }
            
        case AST_BINARY_OP: {
            int left = evaluate_expression(interp, node->left);
            int right = evaluate_expression(interp, node->right);
            
            if (strcmp(node->name, "和") == 0) {
                return left + right;
            } else if (strcmp(node->name, "差") == 0) {
                return left - right;
            } else if (strcmp(node->name, "積") == 0) {
                return left * right;
            } else if (strcmp(node->name, "商") == 0) {
                return right != 0 ? left / right : 0;
            }
            break;
        }
        
        case AST_FUNC_CALL: {
            // 関数を検索
            int funcIdx = find_function(interp, node->name);
            if (funcIdx >= 0) {
                Function *func = &interp->functions[funcIdx];
                
                // 引数を評価
                int arg1 = evaluate_expression(interp, node->args[0]);
                int arg2 = evaluate_expression(interp, node->args[1]);
                
                // 一時的に変数を保存
                int oldVarCount = interp->varCount;
                
                // パラメータを変数として設定
                set_variable(interp, func->params[0], arg1);
                set_variable(interp, func->params[1], arg2);
                
                // 関数本体を評価（簡易版：和、差、積、商のみ）
                int result = 0;
                if (strstr(func->body, "和")) {
                    result = arg1 + arg2;
                } else if (strstr(func->body, "差")) {
                    result = arg1 - arg2;
                } else if (strstr(func->body, "積")) {
                    result = arg1 * arg2;
                } else if (strstr(func->body, "商")) {
                    result = arg2 != 0 ? arg1 / arg2 : 0;
                }
                
                // 変数を復元
                interp->varCount = oldVarCount;
                
                return result;
            }
            break;
        }
        
        default:
            break;
    }
    
    return 0;
}

int interpreter_execute(Interpreter *interp, ASTNode *node) {
    if (!node) return 0;
    
    switch (node->type) {
        case AST_VAR_DECL: {
            int value = evaluate_expression(interp, node->right);
            set_variable(interp, node->name, value);
            break;
        }
        
        case AST_ARRAY_DECL: {
            int values[MAX_ARRAY_SIZE];
            for (int i = 0; i < node->arraySize; i++) {
                values[i] = evaluate_expression(interp, node->arrayElements[i]);
            }
            set_array(interp, node->name, node->arraySize, values);
            break;
        }
        
        case AST_FUNC_DECL: {
            define_function(interp, node->name, 
                          node->params[0], node->params[1], 
                          node->body);
            break;
        }
        
        case AST_PRINT: {
            if (node->right && node->right->type == AST_IDENTIFIER) {
                // 文字列リテラル
                printf("%s\n", node->right->name);
            } else if (node->right && node->right->type == AST_ARRAY_ACCESS) {
                // 配列アクセス
                int value = evaluate_expression(interp, node->right);
                printf("%d\n", value);
            } else if (strlen(node->name) > 0) {
                // 変数
                int value = get_variable(interp, node->name);
                printf("%d\n", value);
            }
            break;
        }
        
        case AST_IF: {
            // 条件を評価
            int condResult = 0;
            if (node->condition) {
                int left = evaluate_expression(interp, node->condition->left);
                int right = evaluate_expression(interp, node->condition->right);
                
                if (strcmp(node->condition->name, "大") == 0) {
                    condResult = (left > right);
                } else if (strcmp(node->condition->name, "小") == 0) {
                    condResult = (left < right);
                } else if (strcmp(node->condition->name, "等") == 0) {
                    condResult = (left == right);
                }
            }
            
            // 条件に応じて分岐
            if (condResult) {
                if (node->thenBranch) {
                    interpreter_execute(interp, node->thenBranch);
                }
            } else {
                if (node->elseBranch) {
                    interpreter_execute(interp, node->elseBranch);
                }
            }
            break;
        }
        
        case AST_LOOP: {
            // 初期値を設定
            int start = evaluate_expression(interp, node->left);
            int end = evaluate_expression(interp, node->right);
            
            // ループ実行
            for (int i = start; i < end; i++) {
                set_variable(interp, node->name, i);
                printf("%d\n", i);
            }
            break;
        }
        
        default:
            break;
    }
    
    return 0;
}

void interpreter_run_file(Interpreter *interp, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ファイルが開けません: %s\n", filename);
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // 改行削除
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';
        
        if (strlen(line) == 0) continue;
        
        // 全角スペースを半角スペースに変換
        char converted[512];
        int j = 0;
        for (int i = 0; line[i] && j < 510; i++) {
            // 全角スペース（UTF-8: 0xE38080）を検出
            if (i + 2 < (int)strlen(line) &&
                (unsigned char)line[i] == 0xE3 &&
                (unsigned char)line[i + 1] == 0x80 &&
                (unsigned char)line[i + 2] == 0x80) {
                converted[j++] = ' '; // 半角スペースに変換
                i += 2; // 3バイト分進める（forループで+1されるので+2）
            } else {
                converted[j++] = line[i];
            }
        }
        converted[j] = '\0';
        strcpy(line, converted);
        
        // 字句解析
        Lexer *lexer = lexer_create(line);
        Parser *parser = parser_create(lexer);
        
        // 構文解析
        ASTNode *ast = parser_parse_line(parser);
        
        // 実行
        if (ast) {
            interpreter_execute(interp, ast);
            ast_free(ast);
        }
        
        parser_free(parser);
        lexer_free(lexer);
    }
    
    fclose(file);
}
