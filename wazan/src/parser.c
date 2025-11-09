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
    ast_free(node->index);
    for (int i = 0; i < node->arraySize && i < 100; i++) {
        ast_free(node->arrayElements[i]);
    }
    free(node);
}

static ASTNode* create_node(ASTNodeType type) {
    ASTNode *node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = type;
    return node;
}

// 簡易パーサー：元のソースを直接使用
ASTNode* parser_parse_line(Parser *parser) {
    // 元のソースを直接使用
    const char *line = parser->lexer->source;
    
    if (strlen(line) == 0) return NULL;
    
    // 配列宣言: 列 arr は 一、二、三、四、五 と定む。
    if (strstr(line, "列 ") && strstr(line, "は") && strstr(line, "、")) {
        ASTNode *node = create_node(AST_ARRAY_DECL);
        
        char varName[64] = {0};
        // "列 " の後から "は" の前まで取得
        char *start = strstr(line, "列 ") + strlen("列 ");
        char *hasPosArray = strstr(line, "は");
        if (hasPosArray && start < hasPosArray) {
            int i = 0;
            while (start < hasPosArray && i < 63) {
                if (*start != ' ') {
                    varName[i++] = *start;
                }
                start++;
            }
            varName[i] = '\0';
        }
        strcpy(node->name, varName);
        
        // "は " の後から要素を取得
        if (hasPosArray) {
            char *elemStart = hasPosArray + 4; // "は " の後（UTF-8）
            while (*elemStart == ' ') elemStart++;
            
            // 要素を解析
            node->arraySize = 0;
            char *current = elemStart;
            char element[64];
            int elemIdx = 0;
            
            while (*current && node->arraySize < 100) {
                // 、または「と定む」まで読む
                if (strstr(current, "、") == current) {
                    // 要素を保存
                    element[elemIdx] = '\0';
                    if (elemIdx > 0) {
                        node->arrayElements[node->arraySize] = create_node(AST_NUMBER);
                        if (is_kanji_number(element)) {
                            node->arrayElements[node->arraySize]->value = kanji_to_int(element);
                        } else {
                            node->arrayElements[node->arraySize]->type = AST_IDENTIFIER;
                            strcpy(node->arrayElements[node->arraySize]->name, element);
                        }
                        node->arraySize++;
                    }
                    current += 3; // UTF-8の、は3バイト
                    elemIdx = 0;
                    while (*current == ' ') current++;
                } else if (strstr(current, "と定む") == current) {
                    // 最後の要素を保存
                    element[elemIdx] = '\0';
                    if (elemIdx > 0) {
                        node->arrayElements[node->arraySize] = create_node(AST_NUMBER);
                        if (is_kanji_number(element)) {
                            node->arrayElements[node->arraySize]->value = kanji_to_int(element);
                        } else {
                            node->arrayElements[node->arraySize]->type = AST_IDENTIFIER;
                            strcpy(node->arrayElements[node->arraySize]->name, element);
                        }
                        node->arraySize++;
                    }
                    break;
                } else {
                    if (*current != ' ' && elemIdx < 63) {
                        element[elemIdx++] = *current;
                    }
                    current++;
                }
            }
        }
        
        return node;
    }
    
    // 配列アクセス: 申す arr の 二番目
    if (strstr(line, "の") && strstr(line, "番目")) {
        char varName[64], indexStr[64];
        char *noPos = strstr(line, "の ");
        char *banPos = strstr(line, "番目");
        
        if (noPos && banPos && banPos > noPos) {
            // 変数名を取得
            int nameLen = noPos - line;
            if (nameLen > 0 && nameLen < 63) {
                // "申す " の後から変数名を取得
                char *start = strstr(line, "申す ");
                if (start) {
                    start += strlen("申す ");
                    while (*start == ' ') start++;
                    
                    int i = 0;
                    while (start < noPos && i < 63) {
                        if (*start != ' ') {
                            varName[i++] = *start;
                        }
                        start++;
                    }
                    varName[i] = '\0';
                    
                    // インデックスを取得
                    char *idxStart = noPos + 3; // "の " の後
                    while (*idxStart == ' ') idxStart++;
                    
                    i = 0;
                    while (idxStart < banPos && i < 63) {
                        if (*idxStart != ' ') {
                            indexStr[i++] = *idxStart;
                        }
                        idxStart++;
                    }
                    indexStr[i] = '\0';
                    
                    // 配列アクセスノードを作成
                    ASTNode *node = create_node(AST_PRINT);
                    node->right = create_node(AST_ARRAY_ACCESS);
                    strcpy(node->right->name, varName);
                    
                    node->right->index = create_node(AST_NUMBER);
                    if (is_kanji_number(indexStr)) {
                        node->right->index->value = kanji_to_int(indexStr);
                    } else {
                        node->right->index->type = AST_IDENTIFIER;
                        strcpy(node->right->index->name, indexStr);
                    }
                    
                    return node;
                }
            }
        }
    }
    
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
        // または: 数 total は prices の 一番目 と prices の 二番目 の和 と定む
        // 演算子の「の」を探す（最後の「の」）
        char *lastNoPos = NULL;
        char *searchPos = line;
        while ((searchPos = strstr(searchPos, "の")) != NULL) {
            lastNoPos = searchPos;
            searchPos += 3; // UTF-8の「の」は3バイト
        }
        
        // 「と定む」で終わる場合は配列アクセスの代入なので、スキップ
        if (lastNoPos && !strstr(line, "番目 と定む")) {
            // 演算子を取得（最後の「の」の後）
            char *opStart = lastNoPos + 3; // 「の」の後
            while (*opStart == ' ') opStart++;
            
            char opStr[64] = {0};
            int i = 0;
            while (opStart[i] && opStart[i] != ' ' && i < 63) {
                opStr[i] = opStart[i];
                i++;
            }
            opStr[i] = '\0';
            
            // 演算子が有効かチェック
            if (strcmp(opStr, "和") == 0 || strcmp(opStr, "差") == 0 ||
                strcmp(opStr, "積") == 0 || strcmp(opStr, "商") == 0) {
                
                // 変数名を取得
                sscanf(line, "数 %s", varName);
                strcpy(node->name, varName);
                
                // 二項演算ノード作成
                node->right = create_node(AST_BINARY_OP);
                strcpy(node->right->name, opStr);
                
                // 「は」と「と」の位置を取得
                char *hasPos = strstr(line, "は");
                
                // 「と」を探す（演算子の「の」の前にある「と」）
                char *toPos = NULL;
                searchPos = hasPos;
                while (searchPos && searchPos < lastNoPos) {
                    char *nextTo = strstr(searchPos, "と");
                    if (nextTo && nextTo < lastNoPos) {
                        toPos = nextTo;
                        searchPos = nextTo + 3; // UTF-8の「と」は3バイト
                    } else {
                        break;
                    }
                }
                
                if (!toPos) {
                    // 「と」が見つからない場合は、単純な形式
                    return node;
                }
                
                // 左オペランドを解析（「は」の後から「と」の前まで）
                char *leftStart = hasPos + 3; // 「は」の後
                while (*leftStart == ' ') leftStart++;
                
                // 左オペランドが配列アクセスかチェック
                if (strstr(leftStart, "の") && strstr(leftStart, "番目")) {
                    char leftArray[64] = {0}, leftIndex[64] = {0};
                    char *leftNoPos = strstr(leftStart, "の");
                    if (leftNoPos && leftNoPos < toPos) {
                        // 配列名を取得
                        i = 0;
                        while (leftStart < leftNoPos && i < 63) {
                            if (*leftStart != ' ') {
                                leftArray[i++] = *leftStart;
                            }
                            leftStart++;
                        }
                        leftArray[i] = '\0';
                        
                        // インデックスを取得
                        char *leftIndexStart = leftNoPos + 3;
                        while (*leftIndexStart == ' ') leftIndexStart++;
                        char *leftBanPos = strstr(leftIndexStart, "番目");
                        if (leftBanPos && leftBanPos < toPos) {
                            i = 0;
                            while (leftIndexStart < leftBanPos && i < 63) {
                                if (*leftIndexStart != ' ') {
                                    leftIndex[i++] = *leftIndexStart;
                                }
                                leftIndexStart++;
                            }
                            leftIndex[i] = '\0';
                            
                            node->right->left = create_node(AST_ARRAY_ACCESS);
                            strcpy(node->right->left->name, leftArray);
                            node->right->left->index = create_node(AST_NUMBER);
                            if (is_kanji_number(leftIndex)) {
                                node->right->left->index->value = kanji_to_int(leftIndex);
                            }
                        }
                    }
                } else {
                    // 通常の変数または数値
                    char leftVal[64] = {0};
                    i = 0;
                    while (leftStart < toPos && i < 63) {
                        if (*leftStart != ' ') {
                            leftVal[i++] = *leftStart;
                        }
                        leftStart++;
                    }
                    leftVal[i] = '\0';
                    
                    node->right->left = create_node(AST_IDENTIFIER);
                    strcpy(node->right->left->name, leftVal);
                    if (is_kanji_number(leftVal)) {
                        node->right->left->type = AST_NUMBER;
                        node->right->left->value = kanji_to_int(leftVal);
                    }
                }
                
                // 右オペランドを解析（「と」の後から演算子の「の」の前まで）
                char *rightStart = toPos + 3; // 「と」の後
                while (*rightStart == ' ') rightStart++;
                
                // 右オペランドが配列アクセスかチェック
                if (strstr(rightStart, "の") && strstr(rightStart, "番目")) {
                    char rightArray[64] = {0}, rightIndex[64] = {0};
                    char *rightNoPos = strstr(rightStart, "の");
                    if (rightNoPos && rightNoPos < lastNoPos) {
                        // 配列名を取得
                        i = 0;
                        while (rightStart < rightNoPos && i < 63) {
                            if (*rightStart != ' ') {
                                rightArray[i++] = *rightStart;
                            }
                            rightStart++;
                        }
                        rightArray[i] = '\0';
                        
                        // インデックスを取得
                        char *rightIndexStart = rightNoPos + 3;
                        while (*rightIndexStart == ' ') rightIndexStart++;
                        char *rightBanPos = strstr(rightIndexStart, "番目");
                        if (rightBanPos && rightBanPos < lastNoPos) {
                            i = 0;
                            while (rightIndexStart < rightBanPos && i < 63) {
                                if (*rightIndexStart != ' ') {
                                    rightIndex[i++] = *rightIndexStart;
                                }
                                rightIndexStart++;
                            }
                            rightIndex[i] = '\0';
                            
                            node->right->right = create_node(AST_ARRAY_ACCESS);
                            strcpy(node->right->right->name, rightArray);
                            node->right->right->index = create_node(AST_NUMBER);
                            if (is_kanji_number(rightIndex)) {
                                node->right->right->index->value = kanji_to_int(rightIndex);
                            }
                        }
                    }
                } else {
                    // 通常の変数または数値
                    char rightVal[64] = {0};
                    i = 0;
                    while (rightStart < lastNoPos && i < 63) {
                        if (*rightStart != ' ') {
                            rightVal[i++] = *rightStart;
                        }
                        rightStart++;
                    }
                    rightVal[i] = '\0';
                    
                    node->right->right = create_node(AST_IDENTIFIER);
                    strcpy(node->right->right->name, rightVal);
                    if (is_kanji_number(rightVal)) {
                        node->right->right->type = AST_NUMBER;
                        node->right->right->value = kanji_to_int(rightVal);
                    }
                }
                
                return node;
            }
        }
        
        // 配列アクセスを変数に代入: 数 a は prices の 二番目 と定む
        if (strstr(line, "の") && strstr(line, "番目")) {
            char arrayName[64];
            // "数 varName は arrayName の" までを解析
            char *hasPos = strstr(line, "は ");
            char *noPos = strstr(line, " の ");
            
            if (hasPos && noPos && noPos > hasPos) {
                sscanf(line, "数 %s", varName);
                
                // 配列名を取得
                char *arrayStart = hasPos + strlen("は ");
                while (*arrayStart == ' ') arrayStart++;
                int i = 0;
                while (arrayStart < noPos && i < 63) {
                    if (*arrayStart != ' ') {
                        arrayName[i++] = *arrayStart;
                    }
                    arrayStart++;
                }
                arrayName[i] = '\0';
                
                // インデックスを取得（「の」の後から「番目」の前まで）
                char *indexStart = noPos + strlen(" の ");
                while (*indexStart == ' ') indexStart++;
                char *banPos = strstr(indexStart, "番目");
                
                if (banPos) {
                    char indexStr[64] = {0};
                    i = 0;
                    while (indexStart < banPos && i < 63) {
                        if (*indexStart != ' ') {
                            indexStr[i++] = *indexStart;
                        }
                        indexStart++;
                    }
                    indexStr[i] = '\0';
                    
                    strcpy(node->name, varName);
                    
                    // 配列アクセスノード作成
                    node->right = create_node(AST_ARRAY_ACCESS);
                    strcpy(node->right->name, arrayName);
                    
                    node->right->index = create_node(AST_NUMBER);
                    if (is_kanji_number(indexStr)) {
                        node->right->index->value = kanji_to_int(indexStr);
                    } else {
                        node->right->index->type = AST_IDENTIFIER;
                        strcpy(node->right->index->name, indexStr);
                    }
                    
                    return node;
                }
            }
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
