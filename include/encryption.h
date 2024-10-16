#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <openssl/err.h>
#include <openssl/evp.h>

#include "misc.h"
#include "std_libs.h"

typedef enum { ENC_NONE, AES128, AES192, AES256, DES3 } encryption;
typedef enum { MODE_NONE, ECB, CBC, CFB, OFB } mode;

static const char* encryption_str[]
    __attribute__((unused)) = {"None", "AES128", "AES192", "AES256", "3DES"};

static const char* mode_str[] __attribute__((unused)) = {"None", "ECB", "CBC", "CFB", "OFB"};

unsigned char* encrypt_data(const unsigned char* plaintext,
                            size_t               plaintext_len,
                            const char*          pass,
                            encryption           a,
                            mode                 m,
                            size_t*              encrypted_len);
unsigned char* decrypt_data(const unsigned char* ciphertext,
                            size_t               ciphertext_len,
                            const char*          pass,
                            encryption           a,
                            mode                 m,
                            size_t*              decrypted_len);

#endif
