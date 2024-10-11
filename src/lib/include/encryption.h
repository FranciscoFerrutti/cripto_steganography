#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <openssl/evp.h>

#include "common_libs.h"

typedef enum { AES128, AES192, AES256, DES3 } encryption;
typedef enum { ECB, CBC, CFB, OFB } mode;

void encrypt(FILE *in, uint8_t *out, size_t size, const char *password, encryption a, mode m);
void decrypt(FILE *in, uint8_t *out, size_t size, const char *password, encryption a, mode m);

// Helper functions
int generate_key_iv(
    const char *password, unsigned char *key, unsigned char *iv, int key_len, int iv_len);
int process_encryption(const uint8_t       *input,
                       uint8_t             *output,
                       size_t               input_len,
                       const unsigned char *key,
                       const unsigned char *iv,
                       const EVP_CIPHER    *cipher,
                       int                  encrypt);

#endif
