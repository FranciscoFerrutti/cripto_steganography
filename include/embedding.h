#ifndef EMBEDDING_H
#define EMBEDDING_H

#include "common_libs.h"
#include "encryption.h"

typedef enum steg { STEG_NONE, LSB1, LSB4, LSBI } steg;
static const char* steg_str[]
    __attribute__((unused)) = {"None", "LSB1", "LSB4", "LSBI"};  // ignore unused warning

unsigned char* prepare_embedding_data(
    const char* messageFile, size_t* totalDataSize, const char* pass, encryption a, mode m);

int extract_embedded_data(const unsigned char* dataBuffer,
                          size_t               dataSize,
                          const char*          outputFilePath,
                          const char*          pass,
                          encryption           a,
                          mode                 m);

#endif