#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/lexer.h"

// 基本的な漢数字を数値に変換
static int get_basic_kanji(const char *str) {
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
    return -1;
}

// 位を取得
static int get_unit_value(const char *str) {
    if (strcmp(str, "十") == 0) return 10;
    if (strcmp(str, "百") == 0) return 100;
    if (strcmp(str, "千") == 0) return 1000;
    if (strcmp(str, "万") == 0) return 10000;
    if (strcmp(str, "億") == 0) return 100000000;
    return -1;
}

// 漢数字を整数に変換（億まで対応）
int kanji_to_int(const char *str) {
    // 単純な一桁の数字
    int basic = get_basic_kanji(str);
    if (basic >= 0) return basic;
    
    // 単純な位
    int unit = get_unit_value(str);
    if (unit >= 0) return unit;
    
    // 複合数字の解析（例：二十、三百、五千）
    // UTF-8で3バイト×2文字 = 6バイト程度
    size_t len = strlen(str);
    
    // 二十、三十などの解析
    if (len >= 6) {
        // 最初の3バイトを取得（一文字目）
        char first[4] = {str[0], str[1], str[2], '\0'};
        int firstNum = get_basic_kanji(first);
        
        // 次の3バイトを取得（二文字目）
        char second[4] = {str[3], str[4], str[5], '\0'};
        int secondUnit = get_unit_value(second);
        
        if (firstNum > 0 && secondUnit > 0) {
            return firstNum * secondUnit;
        }
    }
    
    return -1;
}

int is_kanji_number(const char *str) {
    // 基本数字チェック
    if (get_basic_kanji(str) >= 0) return 1;
    
    // 位のチェック
    if (get_unit_value(str) >= 0) return 1;
    
    // 複合数字のチェック（二十、三百など）
    size_t len = strlen(str);
    if (len >= 6) {
        char first[4] = {str[0], str[1], str[2], '\0'};
        char second[4] = {str[3], str[4], str[5], '\0'};
        if (get_basic_kanji(first) > 0 && get_unit_value(second) > 0) {
            return 1;
        }
    }
    
    return 0;
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
    while (lexer->position < lexer->length) {
        // 半角スペース
        if (isspace((unsigned char)lexer->source[lexer->position])) {
            lexer->position++;
        }
        // 全角スペース（UTF-8: 0xE38080）
        else if (lexer->position + 2 < lexer->length &&
                 (unsigned char)lexer->source[lexer->position] == 0xE3 &&
                 (unsigned char)lexer->source[lexer->position + 1] == 0x80 &&
                 (unsigned char)lexer->source[lexer->position + 2] == 0x80) {
            lexer->position += 3;
        }
        else {
            break;
        }
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
                unsigned char next = (unsigned char)lexer->source[lexer->position];
                if (isspace(next) || next == 0xE3 || 
                    next == '(' || next == ')') {
                    break;
                }
                // 、のチェック（UTF-8: 0xE38081）
                if (next == 0xE3 && lexer->position + 2 < lexer->length &&
                    (unsigned char)lexer->source[lexer->position + 1] == 0x80 &&
                    (unsigned char)lexer->source[lexer->position + 2] == 0x81) {
                    break;
                }
                // 全角スペースのチェック（UTF-8: 0xE38080）
                if (next == 0xE3 && lexer->position + 2 < lexer->length &&
                    (unsigned char)lexer->source[lexer->position + 1] == 0x80 &&
                    (unsigned char)lexer->source[lexer->position + 2] == 0x80) {
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
            strcmp(token.value, "を") == 0 || strcmp(token.value, "定む") == 0 ||
            strcmp(token.value, "列") == 0) {
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
