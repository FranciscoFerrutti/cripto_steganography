#ifndef EXTRACTION_H
#define EXTRACTION_H

#include "steganography.h"

/* Public function that needs to be accessed by main.c */
void extract(const char *carrierFile,
             const char *outputFile,
             steg        method,
             encryption  a,
             mode        m,
             const char *pass);

/* Function used internally by extract.c */
unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted);
unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted);
unsigned char *lsbi_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted);

/* Process extracted data (used internally by extract.c) */
int process_extracted_data(const unsigned char *dataBuffer,
                           const char          *outputFilePath,
                           const char          *pass,
                           encryption           a,
                           mode                 m);

#endif