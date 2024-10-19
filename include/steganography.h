#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H

#include <arpa/inet.h>

#include "bitmap.h"
#include "encryption.h"
#include "misc.h"
#include "std_libs.h"

typedef enum steg { STEG_NONE, LSB1, LSB4, LSBI } steg;
static const char *steg_str[]
    __attribute__((unused)) = {"None", "LSB1", "LSB4", "LSBI"};  // ignore unused warning

#endif