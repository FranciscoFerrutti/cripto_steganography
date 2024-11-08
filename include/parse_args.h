#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include <getopt.h>

#include "encryption.h"
#include "misc.h"
#include "std_libs.h"
#include "steganography.h"

typedef enum { NONE, EMBED, EXTRACT } action;

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

void parse_args(const int argc, const char *argv[], args *args);
void print_help();

#endif
