#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/wazan.h"

Variable vars[MAX_VARS];
int varCount = 0;

Function funcs[MAX_FUNCS];
int funcCount = 0;

// === 漢数字変換 ===
int kanjiToInt(const char *word)
{
    if (strcmp(word, "零") == 0)
        return 0;
    if (strcmp(word, "一") == 0)
        return 1;
    if (strcmp(word, "二") == 0)
        return 2;
    if (strcmp(word, "三") == 0)
        return 3;
    if (strcmp(word, "四") == 0)
        return 4;
    if (strcmp(word, "五") == 0)
        return 5;
    if (strcmp(word, "六") == 0)
        return 6;
    if (strcmp(word, "七") == 0)
        return 7;
    if (strcmp(word, "八") == 0)
        return 8;
    if (strcmp(word, "九") == 0)
        return 9;
    return atoi(word);
}

// === 変数関連 ===
int findVar(const char *name)
{
    for (int i = 0; i < varCount; i++)
        if (strcmp(vars[i].name, name) == 0)
            return i;
    return -1;
}

void setVar(const char *name, int value)
{
    int idx = findVar(name);
    if (idx == -1)
    {
        strcpy(vars[varCount].name, name);
        vars[varCount].value = value;
        varCount++;
    }
    else
    {
        vars[idx].value = value;
    }
}

int getVar(const char *name)
{
    int idx = findVar(name);
    if (idx != -1)
        return vars[idx].value;
    return kanjiToInt(name);
}

// === 四則演算解析 ===
int eval_expr(char *expr)
{
    int a, b;
    char op;
    if (sscanf(expr, "%d %c %d", &a, &op, &b) == 3)
    {
        switch (op)
        {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            return b != 0 ? a / b : 0;
        }
    }
    return getVar(expr);
}

// === 関数定義・実行 ===
void defineFunc(char *name, char *p1, char *p2, char *body)
{
    strcpy(funcs[funcCount].name, name);
    strcpy(funcs[funcCount].param1, p1);
    strcpy(funcs[funcCount].param2, p2);
    strcpy(funcs[funcCount].body, body);
    funcCount++;
}

int callFunc(char *name, int a, int b)
{
    for (int i = 0; i < funcCount; i++)
    {
        if (strcmp(funcs[i].name, name) == 0)
        {
            int result = 0;
            if (strstr(funcs[i].body, "和"))
                result = a + b;
            else if (strstr(funcs[i].body, "差"))
                result = a - b;
            else if (strstr(funcs[i].body, "積"))
                result = a * b;
            else if (strstr(funcs[i].body, "商"))
                result = b != 0 ? a / b : 0;
            return result;
        }
    }
    return 0;
}

// === 条件文・ループ文・出力・変数代入 ===
void interpret_line(char *line)
{
    if (strlen(line) == 0)
        return; // 空行は無視

    // 関数定義
    if (strstr(line, "術「"))
    {
        char name[32] = {0}, p1[32] = {0}, p2[32] = {0}, body[256] = {0};

        // --- 「術「名前」」を抽出 ---
        char *s = strstr(line, "術「");
        char *e = strstr(line, "」(");
        if (!s || !e)
            return;
        strncpy(name, s + strlen("術「"), e - (s + strlen("術「")));
        name[e - (s + strlen("術「"))] = '\0';

        // --- 「数 param1、数 param2」部分 ---
        char *p = strstr(e, "数 ");
        if (p)
        {
            char *c1 = strstr(p, "数 ");
            char *c2 = strstr(c1 + strlen("数 "), "、数 ");
            if (c1 && c2)
            {
                strncpy(p1, c1 + strlen("数 "), c2 - (c1 + strlen("数 ")));
                p1[c2 - (c1 + strlen("数 "))] = '\0';
                char *parenEnd = strstr(c2 + strlen("、数 "), ")");
                if (parenEnd)
                {
                    strncpy(p2, c2 + strlen("、数 "), parenEnd - (c2 + strlen("、数 ")));
                    p2[parenEnd - (c2 + strlen("、数 "))] = '\0';
                }
            }
        }

        // --- 「は ...」以降を body として抽出 ---
        char *b = strstr(line, "は ");
        if (b)
        {
            strncpy(body, b + strlen("は "), sizeof(body) - 1);
            // 「。」などの末尾句点を除去
            char *dot = strstr(body, "。");
            if (dot)
                *dot = '\0';
        }

        defineFunc(name, p1, p2, body);
        printf("（術定義）%s(%s,%s)\n", name, p1, p2);
    }
    if (strstr(line, "数") && strstr(line, "(") && strstr(line, "、"))
    {
        char varName[32], funcName[32], arg1[32], arg2[32];
        if (sscanf(line, "数 %s は %[^'(](%[^、]、%[^)])", varName, funcName, arg1, arg2) == 4)
        {
            int a = getVar(arg1);
            int b = getVar(arg2);
            int result = callFunc(funcName, a, b);
            setVar(varName, result);
            printf("（関数呼出）%s = %s(%d, %d) → %d\n", varName, funcName, a, b, result);
            return;
        }
    }
    // 変数定義・計算
    else if (strstr(line, "数"))
    {
        char name[32], v1[32], op[8], v2[32];
        if (sscanf(line, "数 %s は %s と %s の%s と定む。", name, v1, v2, op) == 4)
        {
            int a = getVar(v1), b = getVar(v2), r = 0;
            if (strcmp(op, "和") == 0)
                r = a + b;
            else if (strcmp(op, "差") == 0)
                r = a - b;
            else if (strcmp(op, "積") == 0)
                r = a * b;
            else if (strcmp(op, "商") == 0)
                r = b != 0 ? a / b : 0;
            setVar(name, r);
            printf("（定義）%s = %d\n", name, r);
        }
        else
        {
            char valStr[32];
            if (sscanf(line, "数 %s は %s", name, valStr) == 2)
            {
                int v = getVar(valStr);
                setVar(name, v);
                printf("（定義）%s = %d\n", name, v);
            }
        }
    }
    // 出力
    else if (strstr(line, "申す"))
    {
        char msg[128];
        char *s = strstr(line, "「");
        char *e = strstr(line, "」");
        if (s && e)
        {
            strncpy(msg, s + 3, e - s - 3);
            msg[e - s - 3] = '\0';
            printf("%s\n", msg);
        }
        else
        {
            char name[32];
            if (sscanf(line, "申す %s", name) == 1)
                printf("%d\n", getVar(name));
        }
    }
    // 条件分岐
    else if (strstr(line, "もし"))
    {
        char var[32], cmp[8], valStr[32];
        if (sscanf(line, "もし %s %[^よ]より%sなれば", var, valStr, cmp) == 3)
        {
            int v = getVar(var), ref = getVar(valStr);
            if (strstr(cmp, "大") && v > ref)
                printf("（条件真）%s > %d\n", var, ref);
            else if (strstr(cmp, "小") && v < ref)
                printf("（条件真）%s < %d\n", var, ref);
            else
                printf("（条件偽）%s = %d\n", var, v);
        }
    }
    // ループ
    else if (strstr(line, "いざ"))
    {
        char var[32], start[32], end[32];
        if (sscanf(line, "いざ %s を %[^と]とし、%[^な]未満なる間、ひとつずつ加え行かん。", var, start, end) == 3)
        {
            int a = getVar(start), b = getVar(end);
            for (int i = a; i < b; i++)
            {
                setVar(var, i);
                printf("（反復）%s = %d\n", var, i);
            }
        }
    }
    // 加算
    else if (strstr(line, "を ひとつ増す"))
    {
        char name[32];
        sscanf(line, "%s を ひとつ増す", name);
        int v = getVar(name);
        setVar(name, v + 1);
        printf("（加算）%s = %d\n", name, v + 1);
    }
    else
    {
        printf("（理解不能）%s\n", line);
    }
}

// === ファイル実行 ===
void run_file(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("ファイルが見つかりませぬ: %s\n", path);
        return;
    }
    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\n")] = '\0';
        interpret_line(line);
    }
    fclose(f);
}

// === メイン ===
int main(int argc, char *argv[])
{
    printf("和算（wazan）インタプリタ v1.0\n");

    if (argc == 2)
    {
        run_file(argv[1]);
        return 0;
    }

    printf("終了するには「終わり」と入力。\n");
    char line[512];
    while (1)
    {
        printf(">> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            printf("\n入力終了\n");
            break;
        }
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "終わり") == 0)
            break;
        interpret_line(line);
    }

    printf("さらば。\n");
    return 0;
}
