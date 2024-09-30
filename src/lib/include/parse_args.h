#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <stdio.h>

#include "common_libs.h"
#include "encryption.h"
#include "steg.h"
// struct for arguments

typedef enum { EMBED, EXTRACT } action;

typedef struct args
{
    action      action;
    const char *in;
    const char *p;
    const char *out;
    steg        steg;
    encryption  a;
    mode        m;
    const char *pass;
} args;

void print_args(const int argc, const char *argv[]);
void parse_args(const int argc, const char *argv[], args *args);

#endif
