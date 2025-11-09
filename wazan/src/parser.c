#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

Parser* parser_create(Lexer *lexer) {
    Parser *parser = (Parser*)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->currentToken = lexer_next_token(lexer);
    return parser;
}

void parser_free(Parser *parser) {
    if (parser) {
        free(parser);
    }
}

void ast_free(ASTNode *node) {
    if (!node) return;
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->condition);
    ast_free(node->thenBranch);
    ast_free(node->elseBranch);
    ast_free(node->args[0]);
    ast_free(node->args[1]);
    free(node);
}

static ASTNode* create_node(ASTNodeType type) {
    ASTNode *node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = type;
    return node;
}

// 簡易パーサー：行全体を解析
ASTNode* parser_parse_line(Parser *parser) {
    char line[512];
    int pos = 0;
    
    // 行全体を文字列として取得
    while (parser->currentToken.type != TOKEN_EOF) {
        if (strlen(parser->currentToken.value) > 0) {
            if (pos > 0) line[pos++] = ' ';
            strcpy(&line[pos], parser->currentToken.value);
            pos += strlen(parser->currentToken.value);
        }
        parser->currentToken = lexer_next_token(parser->lexer);
    }
    line[pos] = '\0';
    
    if (strlen(line) == 0) return NULL;
    
    // 変数宣言: 数 x は 二 と 三 の和 と定む
    if (strstr(line, "数") && strstr(line, "は")) {
        ASTNode *node = create_node(AST_VAR_DECL);
        
        char varName[64], v1[64], v2[64], op[64];
        
        // 関数呼び出し: 数 z は 和(二、三)
        if (strstr(line, "(") && strstr(line, "、") && strstr(line, ")")) {
            // 簡易的なパース
            char temp[512];
            strcpy(temp, line);
            
            // "数 " を探す
            char *numPos = strstr(temp, "数 ");
            if (numPos) {
                numPos += strlen("数 ");
                
                // 変数名を取得（次の空白まで）
                int i = 0;
                while (numPos[i] && numPos[i] != ' ' && i < 63) {
                    varName[i] = numPos[i];
                    i++;
                }
                varName[i] = '\0';
                
                // "(" を探す
                char *openParen = strstr(temp, "(");
                if (openParen) {
                    // 関数名を取得（"は " の後から "(" の前まで）
                    char *hasPos = strstr(temp, "は ");
                    if (hasPos && openParen > hasPos) {
                        char *funcStart = hasPos + strlen("は ");
                        while (*funcStart == ' ') funcStart++;
                        
                        char funcName[64] = {0};
                        i = 0;
                        while (funcStart < openParen && i < 63) {
                            if (*funcStart != ' ') {
                                funcName[i++] = *funcStart;
                            }
                            funcStart++;
                        }
                        funcName[i] = '\0';
                        
                        // 引数を取得
                        char *arg1Start = openParen + 1;
                        char *commaPos = strstr(arg1Start, "、");
                        char *closeParen = strstr(arg1Start, ")");
                        
                        if (commaPos && closeParen && commaPos < closeParen) {
                            char arg1[64] = {0}, arg2[64] = {0};
                            
                            // 引数1（空白を除去）
                            i = 0;
                            while (arg1Start < commaPos && i < 63) {
                                if (*arg1Start != ' ') {
                                    arg1[i++] = *arg1Start;
                                }
                                arg1Start++;
                            }
                            arg1[i] = '\0';
                            
                            // 引数2（、の後から、空白を除去）
                            char *arg2Start = commaPos + 3; // UTF-8の、は3バイト
                            i = 0;
                            while (arg2Start < closeParen && i < 63) {
                                if (*arg2Start != ' ') {
                                    arg2[i++] = *arg2Start;
                                }
                                arg2Start++;
                            }
                            arg2[i] = '\0';
                            
                            // ノード作成
                            strcpy(node->name, varName);
                            node->right = create_node(AST_FUNC_CALL);
                            strcpy(node->right->name, funcName);
                            
                            // 引数1
                            node->right->args[0] = create_node(AST_NUMBER);
                            if (is_kanji_number(arg1)) {
                                node->right->args[0]->value = kanji_to_int(arg1);
                            } else {
                                node->right->args[0]->type = AST_IDENTIFIER;
                                strcpy(node->right->args[0]->name, arg1);
                            }
                            
                            // 引数2
                            node->right->args[1] = create_node(AST_NUMBER);
                            if (is_kanji_number(arg2)) {
                                node->right->args[1]->value = kanji_to_int(arg2);
                            } else {
                                node->right->args[1]->type = AST_IDENTIFIER;
                                strcpy(node->right->args[1]->name, arg2);
                            }
                            
                            return node;
                        }
                    }
                }
            }
        }
        
        // 二項演算: 数 x は 二 と 三 の和 と定む
        if (sscanf(line, "数 %s は %s と %s の %s", varName, v1, v2, op) == 4) {
            strcpy(node->name, varName);
            
            // 二項演算ノード作成
            node->right = create_node(AST_BINARY_OP);
            strcpy(node->right->name, op);
            
            // 左オペランド
            node->right->left = create_node(AST_IDENTIFIER);
            strcpy(node->right->left->name, v1);
            if (is_kanji_number(v1)) {
                node->right->left->type = AST_NUMBER;
                node->right->left->value = kanji_to_int(v1);
            }
            
            // 右オペランド
            node->right->right = create_node(AST_IDENTIFIER);
            strcpy(node->right->right->name, v2);
            if (is_kanji_number(v2)) {
                node->right->right->type = AST_NUMBER;
                node->right->right->value = kanji_to_int(v2);
            }
            
            return node;
        }
        
        // 単純代入: 数 x は 五 と定む
        if (sscanf(line, "数 %s は %s", varName, v1) == 2) {
            strcpy(node->name, varName);
            node->right = create_node(AST_NUMBER);
            if (is_kanji_number(v1)) {
                node->right->value = kanji_to_int(v1);
            } else {
                node->right->type = AST_IDENTIFIER;
                strcpy(node->right->name, v1);
            }
            return node;
        }
    }
    
    // 関数宣言: 術「二倍」(数 n、数 m) は n と m の 和 と定む
    if (strstr(line, "術「")) {
        ASTNode *node = create_node(AST_FUNC_DECL);
        
        char *start = strstr(line, "術「") + strlen("術「");
        char *end = strstr(start, "」");
        if (end) {
            strncpy(node->name, start, end - start);
            node->name[end - start] = '\0';
        }
        
        // パラメータ解析
        char *paramStart = strstr(line, "(");
        char *paramEnd = strstr(line, ")");
        if (paramStart && paramEnd) {
            char params[128];
            strncpy(params, paramStart + 1, paramEnd - paramStart - 1);
            params[paramEnd - paramStart - 1] = '\0';
            
            // 数 n、数 m を解析
            char *token = strtok(params, "、");
            int paramIdx = 0;
            while (token && paramIdx < 2) {
                char *p = strstr(token, "数 ");
                if (p) {
                    char *paramName = p + strlen("数 ");
                    // 空白を除去してコピー
                    int j = 0;
                    for (int k = 0; paramName[k] && j < 63; k++) {
                        if (paramName[k] != ' ') {
                            node->params[paramIdx][j++] = paramName[k];
                        }
                    }
                    node->params[paramIdx][j] = '\0';
                    paramIdx++;
                }
                token = strtok(NULL, "、");
            }
        }
        
        // 関数本体
        char *bodyStart = strstr(line, "は ");
        if (bodyStart) {
            strcpy(node->body, bodyStart + strlen("は "));
            char *dot = strstr(node->body, "と定む");
            if (dot) *dot = '\0';
        }
        
        return node;
    }
    
    // 条件分岐: もし x 二より大なれば 申す「大なり」 さらずば 申す「小なり」
    if (strstr(line, "もし")) {
        ASTNode *node = create_node(AST_IF);
        
        char varName[64], cmpValue[64], cmpOp[64];
        char *thenPart = strstr(line, "なれば");
        char *elsePart = strstr(line, "さらずば");
        
        // 条件部分を解析: "もし x 二より大なれば"
        if (sscanf(line, "もし %s %[^よ]より%[^な]なれば", varName, cmpValue, cmpOp) == 3) {
            // 条件ノード作成
            node->condition = create_node(AST_BINARY_OP);
            strcpy(node->condition->name, cmpOp); // "大" or "小"
            
            node->condition->left = create_node(AST_IDENTIFIER);
            strcpy(node->condition->left->name, varName);
            
            node->condition->right = create_node(AST_NUMBER);
            if (is_kanji_number(cmpValue)) {
                node->condition->right->value = kanji_to_int(cmpValue);
            } else {
                node->condition->right->type = AST_IDENTIFIER;
                strcpy(node->condition->right->name, cmpValue);
            }
            
            // then部分を解析
            if (thenPart) {
                char thenStmt[256];
                char *thenStart = thenPart + strlen("なれば ");
                if (elsePart) {
                    strncpy(thenStmt, thenStart, elsePart - thenStart - 1);
                    thenStmt[elsePart - thenStart - 1] = '\0';
                } else {
                    strcpy(thenStmt, thenStart);
                }
                
                // then文を再帰的にパース
                Lexer *thenLexer = lexer_create(thenStmt);
                Parser *thenParser = parser_create(thenLexer);
                node->thenBranch = parser_parse_line(thenParser);
                parser_free(thenParser);
                lexer_free(thenLexer);
            }
            
            // else部分を解析
            if (elsePart) {
                char elseStmt[256];
                strcpy(elseStmt, elsePart + strlen("さらずば "));
                
                Lexer *elseLexer = lexer_create(elseStmt);
                Parser *elseParser = parser_create(elseLexer);
                node->elseBranch = parser_parse_line(elseParser);
                parser_free(elseParser);
                lexer_free(elseLexer);
            }
            
            return node;
        }
    }
    
    // ループ: いざ i を 一とし、五未満なる間、ひとつずつ加え行かん。
    if (strstr(line, "いざ")) {
        ASTNode *node = create_node(AST_LOOP);
        
        char varName[64], startVal[64], endVal[64];
        
        // "いざ i を 一とし、五未満なる間、ひとつずつ加え行かん。"
        if (sscanf(line, "いざ %s を %[^と]とし、%[^未]未満なる間", varName, startVal, endVal) == 3) {
            strcpy(node->name, varName);
            
            // 初期値
            node->left = create_node(AST_NUMBER);
            if (is_kanji_number(startVal)) {
                node->left->value = kanji_to_int(startVal);
            } else {
                node->left->type = AST_IDENTIFIER;
                strcpy(node->left->name, startVal);
            }
            
            // 終了条件
            node->right = create_node(AST_NUMBER);
            if (is_kanji_number(endVal)) {
                node->right->value = kanji_to_int(endVal);
            } else {
                node->right->type = AST_IDENTIFIER;
                strcpy(node->right->name, endVal);
            }
            
            return node;
        }
    }
    
    // 出力: 申す x
    if (strstr(line, "申す")) {
        ASTNode *node = create_node(AST_PRINT);
        
        char *start = strstr(line, "申す");
        if (start) {
            start += strlen("申す");
            // 空白をスキップ
            while (*start == ' ') start++;
            
            // 文字列リテラル
            if (strstr(start, "「")) {
                char *strStart = strstr(start, "「");
                strStart += 3; // UTF-8の「は3バイト
                char *strEnd = strstr(strStart, "」");
                if (strEnd) {
                    node->right = create_node(AST_IDENTIFIER);
                    strncpy(node->right->name, strStart, strEnd - strStart);
                    node->right->name[strEnd - strStart] = '\0';
                }
            } else {
                // 変数または数値
                sscanf(start, "%s", node->name);
            }
        }
        
        return node;
    }
    
    return NULL;
}
