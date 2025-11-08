#ifndef WAZAN_H
#define WAZAN_H

#define MAX_VARS 128
#define MAX_FUNCS 128

typedef struct {
    char name[32];
    int value;
} Variable;

typedef struct {
    char name[32];
    char param1[32];
    char param2[32];
    char body[256];
} Function;

#endif
