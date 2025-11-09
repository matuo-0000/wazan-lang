#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

#define MAX_VARIABLES 256
#define MAX_FUNCTIONS 128
#define MAX_ARRAY_SIZE 100

typedef struct {
    char name[64];
    int value;
    int isArray;
    int arraySize;
    int arrayValues[MAX_ARRAY_SIZE];
} Variable;

typedef struct {
    char name[64];
    char params[2][64];
    char body[256];
    int paramCount;
} Function;

typedef struct {
    Variable variables[MAX_VARIABLES];
    int varCount;
    Function functions[MAX_FUNCTIONS];
    int funcCount;
} Interpreter;

Interpreter* interpreter_create();
void interpreter_free(Interpreter *interp);
int interpreter_execute(Interpreter *interp, ASTNode *node);
void interpreter_run_file(Interpreter *interp, const char *filename);

#endif
