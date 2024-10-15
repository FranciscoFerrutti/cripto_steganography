#ifndef STEGANOGRAPHY_H
#define STEGANOGRAPHY_H

#include "bitmap.h"
#include "common_libs.h"
#include "encryption.h"

typedef enum steg { STEG_NONE, LSB1, LSB4, LSBI } steg;
static const char *steg_str[]
    __attribute__((unused)) = {"None", "LSB1", "LSB4", "LSBI"};  // ignore unused warning

void embed(const char *carrierFile,
           const char *messageFile,
           const char *outputFile,
           steg        method,
           encryption  a,
           mode        m,
           const char *pass);
void extract(const char *carrierFile,
             const char *outputFile,
             steg        method,
             encryption  a,
             mode        m,
             const char *pass);

#endif