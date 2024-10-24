#ifndef EMBEDDING_H
#define EMBEDDING_H

#include "steganography.h"

void embed(const char *carrierFile,
           const char *messageFile,
           const char *outputFile,
           steg        method,
           encryption  a,
           mode        m,
           const char *pass);

int lsb1_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize);
int lsb4_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize);
int lsbi_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize);

unsigned char *prepare_embedding_data(
    const char *messageFile, size_t *totalDataSize, const char *pass, encryption a, mode m);

#endif