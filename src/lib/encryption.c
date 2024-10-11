#include "include/encryption.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdlib.h>
#include <string.h>

#define AES128_KEY_LEN 16
#define AES192_KEY_LEN 24
#define AES256_KEY_LEN 32
#define DES3_KEY_LEN 24
#define AES_IV_LEN 16
#define DES3_IV_LEN 8

int generate_key_iv(
    const char *password, unsigned char *key, unsigned char *iv, int key_len, int iv_len) {
    if (!PKCS5_PBKDF2_HMAC(password, strlen(password), NULL, 0, 1000, EVP_sha256(), key_len, key)) {
        fprintf(stderr, "Key generation failed\n");
        return 0;
    }
    if (!PKCS5_PBKDF2_HMAC(password, strlen(password), NULL, 0, 1000, EVP_sha256(), iv_len, iv)) {
        fprintf(stderr, "IV generation failed\n");
        return 0;
    }
    return 1;
}

int process_encryption(const uint8_t       *input,
                       uint8_t             *output,
                       size_t               input_len,
                       const unsigned char *key,
                       const unsigned char *iv,
                       const EVP_CIPHER    *cipher,
                       int                  encrypt) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Failed to create context\n");
        return 0;
    }

    int ret = 0, outlen = 0, tmplen = 0;

    if (EVP_CipherInit_ex(ctx, cipher, NULL, key, iv, encrypt) != 1) {
        fprintf(
            stderr, "Cipher initialization failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        goto cleanup;
    }

    if (EVP_CipherUpdate(ctx, output, &outlen, input, input_len) != 1) {
        fprintf(stderr, "Cipher update failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        goto cleanup;
    }

    if (EVP_CipherFinal_ex(ctx, output + outlen, &tmplen) != 1) {
        fprintf(
            stderr, "Cipher finalization failed: %s\n", ERR_error_string(ERR_get_error(), NULL));
        goto cleanup;
    }

    ret = 1;

cleanup:
    EVP_CIPHER_CTX_free(ctx);
    return ret;
}

const EVP_CIPHER *get_cipher(encryption alg, mode m) {
    static const EVP_CIPHER *(*aes_ciphers[12])(void) = {EVP_aes_128_ecb,
                                                         EVP_aes_128_cbc,
                                                         EVP_aes_128_cfb,
                                                         EVP_aes_128_ofb,
                                                         EVP_aes_192_ecb,
                                                         EVP_aes_192_cbc,
                                                         EVP_aes_192_cfb,
                                                         EVP_aes_192_ofb,
                                                         EVP_aes_256_ecb,
                                                         EVP_aes_256_cbc,
                                                         EVP_aes_256_cfb,
                                                         EVP_aes_256_ofb};
    static const EVP_CIPHER *(*des3_ciphers[4])(void) = {
        EVP_des_ede3_ecb, EVP_des_ede3_cbc, EVP_des_ede3_cfb, EVP_des_ede3_ofb};

    switch (alg) {
        case AES128:
            return aes_ciphers[m];
        case AES192:
            return aes_ciphers[4 + m];
        case AES256:
            return aes_ciphers[8 + m];
        case DES3:
            return des3_ciphers[m];
        default:
            fprintf(stderr, "Unsupported mode or algorithm\n");
            return NULL;
    }
}

int perform_crypt(const uint8_t *input,
                  uint8_t       *output,
                  size_t         input_len,
                  const char    *password,
                  encryption     alg,
                  mode           m,
                  int            encrypt) {
    unsigned char key[32], iv[16];
    int           key_len = (alg == AES128)   ? AES128_KEY_LEN
                            : (alg == AES192) ? AES192_KEY_LEN
                            : (alg == AES256) ? AES256_KEY_LEN
                                              : DES3_KEY_LEN;
    int           iv_len  = (alg == DES3) ? DES3_IV_LEN : AES_IV_LEN;

    if (!generate_key_iv(password, key, iv, key_len, iv_len)) {
        return 0;
    }

    const EVP_CIPHER *cipher = get_cipher(alg, m);
    if (!cipher) {
        return 0;
    }

    int result = process_encryption(input, output, input_len, key, iv, cipher, encrypt);

    // Zeroize sensitive data
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(iv, sizeof(iv));

    return result;
}

void crypt_file(
    FILE *in, uint8_t *out, size_t size, const char *password, encryption a, mode m, int encrypt) {
    uint8_t *input = (uint8_t *) malloc(size);
    if (!input) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }

    size_t read_size = fread(input, 1, size, in);
    if (read_size != size) {
        fprintf(stderr, "File read failed\n");
        free(input);
        return;
    }

    if (!perform_crypt(input, out, size, password, a, m, encrypt)) {
        fprintf(stderr, "Encryption/Decryption failed\n");
    }

    free(input);
}

void encrypt(FILE *in, uint8_t *out, size_t size, const char *password, encryption a, mode m) {
    crypt_file(in, out, size, password, a, m, 1);
}

void decrypt(FILE *in, uint8_t *out, size_t size, const char *password, encryption a, mode m) {
    crypt_file(in, out, size, password, a, m, 0);
}
