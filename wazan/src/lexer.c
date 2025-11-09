#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/lexer.h"

int kanji_to_int(const char *str) {
    if (strcmp(str, "零") == 0) return 0;
    if (strcmp(str, "一") == 0) return 1;
    if (strcmp(str, "二") == 0) return 2;
    if (strcmp(str, "三") == 0) return 3;
    if (strcmp(str, "四") == 0) return 4;
    if (strcmp(str, "五") == 0) return 5;
    if (strcmp(str, "六") == 0) return 6;
    if (strcmp(str, "七") == 0) return 7;
    if (strcmp(str, "八") == 0) return 8;
    if (strcmp(str, "九") == 0) return 9;
    if (strcmp(str, "十") == 0) return 10;
    return -1;
}

int is_kanji_number(const char *str) {
    return kanji_to_int(str) >= 0;
}

Lexer* lexer_create(const char *source) {
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = strdup(source);
    lexer->position = 0;
    lexer->length = strlen(source);
    return lexer;
}

void lexer_free(Lexer *lexer) {
    if (lexer) {
        free(lexer->source);
        free(lexer);
    }
}

static void skip_whitespace(Lexer *lexer) {
    while (lexer->position < lexer->length && 
           isspace((unsigned char)lexer->source[lexer->position])) {
        lexer->position++;
    }
}

static int is_utf8_start(unsigned char c) {
    return (c & 0x80) != 0;
}

static int utf8_char_len(unsigned char c) {
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

Token lexer_next_token(Lexer *lexer) {
    Token token;
    memset(&token, 0, sizeof(Token));
    
    skip_whitespace(lexer);
    
    if (lexer->position >= lexer->length) {
        token.type = TOKEN_EOF;
        return token;
    }
    
    char c = lexer->source[lexer->position];
    
    // 文字列リテラル「...」
    if (c == '\xe3' && lexer->position + 2 < lexer->length &&
        lexer->source[lexer->position + 1] == '\x80' &&
        lexer->source[lexer->position + 2] == '\x8c') { // 「
        lexer->position += 3;
        int start = lexer->position;
        while (lexer->position < lexer->length) {
            if (lexer->source[lexer->position] == '\xe3' &&
                lexer->position + 2 < lexer->length &&
                lexer->source[lexer->position + 1] == '\x80' &&
                lexer->source[lexer->position + 2] == '\x8d') { // 」
                strncpy(token.value, &lexer->source[start], lexer->position - start);
                token.value[lexer->position - start] = '\0';
                lexer->position += 3;
                token.type = TOKEN_STRING;
                return token;
            }
            lexer->position++;
        }
    }
    
    // 数字
    if (isdigit((unsigned char)c)) {
        int start = lexer->position;
        while (lexer->position < lexer->length && 
               isdigit((unsigned char)lexer->source[lexer->position])) {
            lexer->position++;
        }
        strncpy(token.value, &lexer->source[start], lexer->position - start);
        token.value[lexer->position - start] = '\0';
        token.type = TOKEN_NUMBER;
        token.numValue = atoi(token.value);
        return token;
    }
    
    // UTF-8文字（日本語）
    if (is_utf8_start((unsigned char)c)) {
        int start = lexer->position;
        int charLen = utf8_char_len((unsigned char)c);
        
        // 複数文字を読む
        while (lexer->position < lexer->length) {
            unsigned char ch = lexer->source[lexer->position];
            if (!is_utf8_start(ch) && lexer->position > start) break;
            if (is_utf8_start(ch)) {
                charLen = utf8_char_len(ch);
                lexer->position += charLen;
            } else {
                break;
            }
            
            // 区切り文字チェック
            if (lexer->position < lexer->length) {
                unsigned char next = lexer->source[lexer->position];
                if (isspace(next) || next == '\xe3' || 
                    next == '(' || next == ')') {
                    break;
                }
                // 、のチェック（UTF-8: 0xE38081）
                if (next == 0xE3 && lexer->position + 2 < lexer->length &&
                    lexer->source[lexer->position + 1] == 0x80 &&
                    lexer->source[lexer->position + 2] == 0x81) {
                    break;
                }
            }
        }
        
        int len = lexer->position - start;
        strncpy(token.value, &lexer->source[start], len);
        token.value[len] = '\0';
        
        // キーワードチェック
        if (strcmp(token.value, "数") == 0 || strcmp(token.value, "申す") == 0 ||
            strcmp(token.value, "術") == 0 || strcmp(token.value, "もし") == 0 ||
            strcmp(token.value, "いざ") == 0 || strcmp(token.value, "は") == 0 ||
            strcmp(token.value, "と") == 0 || strcmp(token.value, "の") == 0 ||
            strcmp(token.value, "を") == 0 || strcmp(token.value, "定む") == 0) {
            token.type = TOKEN_KEYWORD;
        }
        // 演算子チェック
        else if (strcmp(token.value, "和") == 0 || strcmp(token.value, "差") == 0 ||
                 strcmp(token.value, "積") == 0 || strcmp(token.value, "商") == 0) {
            token.type = TOKEN_OPERATOR;
        }
        // 漢数字チェック
        else if (is_kanji_number(token.value)) {
            token.type = TOKEN_NUMBER;
            token.numValue = kanji_to_int(token.value);
        }
        // 識別子
        else {
            token.type = TOKEN_IDENTIFIER;
        }
        
        return token;
    }
    
    // その他の文字
    token.value[0] = c;
    token.value[1] = '\0';
    token.type = TOKEN_UNKNOWN;
    lexer->position++;
    
    return token;
}
