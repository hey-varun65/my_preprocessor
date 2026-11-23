#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_MACROS 100
#define MAX_NAME 100
#define MAX_VALUE 500
#define MAX_LINE 2048

typedef struct
{
    char name[MAX_NAME];
    char params[10][MAX_NAME];
    int param_count;
    int function_like;
    char value[MAX_VALUE];
} Macro;

extern Macro macros[MAX_MACROS];
extern int macro_count;

char *remove_comments(const char *source);

void define_macro(char *line);
void expand_macros(char *line, char *output);

int process_file(const char *filename, FILE *out, int depth);

#endif
