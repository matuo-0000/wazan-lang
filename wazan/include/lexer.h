#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_NUMBER,      // 数
    TOKEN_STRING,      // 「文字列」
    TOKEN_IDENTIFIER,  // 変数名・関数名
    TOKEN_KEYWORD,     // キーワード（数、申す、術、もし、いざ等）
    TOKEN_OPERATOR,    // 演算子（和、差、積、商）
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
    int numValue;
} Token;

typedef struct {
    char *source;
    int position;
    int length;
} Lexer;

Lexer* lexer_create(const char *source);
void lexer_free(Lexer *lexer);
Token lexer_next_token(Lexer *lexer);
int is_kanji_number(const char *str);
int kanji_to_int(const char *str);

#endif
