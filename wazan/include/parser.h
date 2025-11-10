#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_VAR_DECL,      // 変数宣言
    AST_FUNC_DECL,     // 関数宣言
    AST_FUNC_CALL,     // 関数呼び出し
    AST_PRINT,         // 申す
    AST_IF,            // もし
    AST_LOOP,          // いざ
    AST_BINARY_OP,     // 二項演算
    AST_UNARY_OP,      // 単項演算
    AST_NUMBER,        // 数値
    AST_IDENTIFIER,    // 識別子
    AST_ARRAY_DECL,    // 配列宣言
    AST_ARRAY_ACCESS   // 配列アクセス
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char name[64];
    int value;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *condition;
    struct ASTNode *thenBranch;
    struct ASTNode *elseBranch;
    char params[2][64];
    struct ASTNode *args[2];
    char body[256];
    int arraySize;
    struct ASTNode *arrayElements[100];
    struct ASTNode *index;
} ASTNode;

typedef struct {
    Lexer *lexer;
    Token currentToken;
} Parser;

Parser* parser_create(Lexer *lexer);
void parser_free(Parser *parser);
ASTNode* parser_parse_line(Parser *parser);
void ast_free(ASTNode *node);

#endif
