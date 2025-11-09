#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/interpreter.h"

void print_usage(const char *progname) {
    printf("和算（Wazan）インタープリタ v2.0\n");
    printf("使用法: %s [ファイル名.wz]\n", progname);
    printf("  ファイル名を指定しない場合は対話モードで起動します\n");
}

void repl_mode() {
    Interpreter *interp = interpreter_create();
    char line[512];
    
    printf("和算（Wazan）対話モード\n");
    printf("終了するには「終わり」と入力してください\n\n");
    
    while (1) {
        printf(">> ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // 改行削除
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';
        
        if (strlen(line) == 0) continue;
        
        // 終了コマンド
        if (strcmp(line, "終わり") == 0 || strcmp(line, "exit") == 0) {
            break;
        }
        
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
    
    printf("\nさらば。\n");
    interpreter_free(interp);
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        // ファイル実行モード
        Interpreter *interp = interpreter_create();
        interpreter_run_file(interp, argv[1]);
        interpreter_free(interp);
    } else if (argc == 1) {
        // 対話モード
        repl_mode();
    } else {
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}
