#include "encryption.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdlib.h>
#include <string.h>

typedef const EVP_CIPHER* (*CipherFunction)();

typedef struct
{
    encryption     alg;
    mode           mod;
    CipherFunction cipher_func;
} CipherMap;

CipherMap cipher_map[] = {{AES128, ECB, EVP_aes_128_ecb},
                          {AES128, CBC, EVP_aes_128_cbc},
                          {AES128, CFB, EVP_aes_128_cfb128},  // CFB with 128 bits of feedback
                          {AES128, OFB, EVP_aes_128_ofb},
                          {AES192, ECB, EVP_aes_192_ecb},
                          {AES192, CBC, EVP_aes_192_cbc},
                          {AES192, CFB, EVP_aes_192_cfb128},
                          {AES192, OFB, EVP_aes_192_ofb},
                          {AES256, ECB, EVP_aes_256_ecb},
                          {AES256, CBC, EVP_aes_256_cbc},
                          {AES256, CFB, EVP_aes_256_cfb128},
                          {AES256, OFB, EVP_aes_256_ofb},
                          {DES3, CBC, EVP_des_ede3_cbc},
                          {DES3, CFB, EVP_des_ede3_cfb64},
                          {DES3, OFB, EVP_des_ede3_ofb}};

const EVP_CIPHER* get_cipher(encryption alg, mode mod) {
    int map_size = sizeof(cipher_map) / sizeof(CipherMap);
    for (int i = 0; i < map_size; i++) {
        if (cipher_map[i].alg == alg && cipher_map[i].mod == mod) {
            return cipher_map[i].cipher_func();
        }
    }
    return NULL;
}

int generate_key_iv(
    const char* pass, unsigned char* key, unsigned char* iv, int key_len, int iv_len) {
    // Fixed salt as specified in the task
    const unsigned char salt[8] = {0};

    int            total_len = key_len + iv_len;
    unsigned char* key_iv    = malloc(total_len);
    if (!key_iv) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }

    /* Derive key and IV from password using PBKDF2 */
    if (PKCS5_PBKDF2_HMAC_SHA1(pass, strlen(pass), salt, sizeof(salt), 10000, total_len, key_iv) !=
        1) {
        fprintf(stderr, "Error deriving key and IV from password\n");
        free(key_iv);
        return 0;
    }

    memcpy(key, key_iv, key_len);
    memcpy(iv, key_iv + key_len, iv_len);

    free(key_iv);
    return 1;
}

/**
 * @brief Encrypt data using the specified algorithm and mode
 *
 * @param plaintext The data to encrypt
 * @param plaintext_len The length of the data to encrypt
 * @param pass The password to use for encryption
 * @param a The encryption algorithm to use
 * @param m The encryption mode to use
 * @param encrypted_len The length of the encrypted data to be returned
 *
 * @return The encrypted data
 */
unsigned char* encrypt_data(const unsigned char* plaintext,
                            size_t               plaintext_len,
                            const char*          pass,
                            encryption           a,
                            mode                 m,
                            size_t*              encrypted_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Error creating encryption context\n");
        return NULL;
    }

    const EVP_CIPHER* cipher_type = get_cipher(a, m);
    if (!cipher_type) {
        fprintf(stderr, "Invalid encryption algorithm or mode\n");
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    int key_len = EVP_CIPHER_key_length(cipher_type);
    int iv_len  = EVP_CIPHER_iv_length(cipher_type);

    unsigned char* key = malloc(key_len);
    unsigned char* iv  = NULL;
    if (iv_len > 0) {
        iv = malloc(iv_len);
        if (!iv) {
            fprintf(stderr, "Memory allocation failed for IV\n");
            EVP_CIPHER_CTX_free(ctx);
            free(key);
            return NULL;
        }
    }

    if (!generate_key_iv(pass, key, iv, key_len, iv_len)) {
        fprintf(stderr, "Error generating key/IV\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, cipher_type, NULL, key, iv) != 1) {
        fprintf(stderr, "Error initializing encryption\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    // Encryption
    size_t         ciphertext_len = plaintext_len + EVP_CIPHER_block_size(cipher_type);
    unsigned char* ciphertext     = malloc(ciphertext_len);
    if (!ciphertext) {
        fprintf(stderr, "Memory allocation failed for ciphertext\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        fprintf(stderr, "Error during encryption\n");
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }
    *encrypted_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "Error during final encryption\n");
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }
    *encrypted_len += len;

    EVP_CIPHER_CTX_free(ctx);
    free(key);
    if (iv)
        free(iv);

    return ciphertext;
}

/**
 * @brief Decrypt data using the specified algorithm and mode
 *
 * @param ciphertext The data to decrypt
 * @param ciphertext_len The length of the data to decrypt
 * @param pass The password to use for decryption
 * @param a The encryption algorithm to use
 * @param m The encryption mode to use
 * @param decrypted_len The length of the decrypted data to be returned
 *
 * @return The decrypted data
 *
 * This is identical to the encryption function, but with the encryption functions replaced with
 * their decryption equivalents.
 */
unsigned char* decrypt_data(const unsigned char* ciphertext,
                            size_t               ciphertext_len,
                            const char*          pass,
                            encryption           a,
                            mode                 m,
                            size_t*              decrypted_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "Error creating decryption context\n");
        return NULL;
    }

    const EVP_CIPHER* cipher_type = get_cipher(a, m);
    if (!cipher_type) {
        fprintf(stderr, "Invalid encryption algorithm or mode\n");
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    int key_len = EVP_CIPHER_key_length(cipher_type);
    int iv_len  = EVP_CIPHER_iv_length(cipher_type);

    unsigned char* key = malloc(key_len);
    unsigned char* iv  = NULL;
    if (iv_len > 0) {
        iv = malloc(iv_len);
        if (!iv) {
            fprintf(stderr, "Memory allocation failed for IV\n");
            EVP_CIPHER_CTX_free(ctx);
            free(key);
            return NULL;
        }
    }

    if (!generate_key_iv(pass, key, iv, key_len, iv_len)) {
        fprintf(stderr, "Error generating key/IV\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    if (EVP_DecryptInit_ex(ctx, cipher_type, NULL, key, iv) != 1) {
        fprintf(stderr, "Error initializing decryption\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    // Decryption
    unsigned char* plaintext = malloc(ciphertext_len + EVP_CIPHER_block_size(cipher_type));
    if (!plaintext) {
        fprintf(stderr, "Memory allocation failed for plaintext\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    int len;
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        fprintf(stderr, "Error during decryption\n");
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }
    *decrypted_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        fprintf(stderr, "Error during final decryption\n");
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }
    *decrypted_len += len;

    EVP_CIPHER_CTX_free(ctx);
    free(key);
    if (iv)
        free(iv);

    return plaintext;
}