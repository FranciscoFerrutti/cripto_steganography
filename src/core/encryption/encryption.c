#include "encryption.h"

typedef const EVP_CIPHER* (*CipherFunction)();

typedef struct
{
    encryption     alg;
    mode           mod;
    CipherFunction cipher_func;
} CipherMap;

CipherMap cipher_map[] = {{AES128, ECB, EVP_aes_128_ecb},
                          {AES128, CBC, EVP_aes_128_cbc},
                          {AES128, CFB, EVP_aes_128_cfb8},  // CFB with 8 bits of feedback
                          {AES128, OFB, EVP_aes_128_ofb},
                          {AES192, ECB, EVP_aes_192_ecb},
                          {AES192, CBC, EVP_aes_192_cbc},
                          {AES192, CFB, EVP_aes_192_cfb8},
                          {AES192, OFB, EVP_aes_192_ofb},
                          {AES256, ECB, EVP_aes_256_ecb},
                          {AES256, CBC, EVP_aes_256_cbc},
                          {AES256, CFB, EVP_aes_256_cfb8},
                          {AES256, OFB, EVP_aes_256_ofb},
                          {DES3, ECB, EVP_des_ede3_ecb},
                          {DES3, CBC, EVP_des_ede3_cbc},
                          {DES3, CFB, EVP_des_ede3_cfb8},
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
    const unsigned char salt[8]   = {0};
    int                 total_len = key_len + iv_len;
    unsigned char*      key_iv    = malloc(total_len);

    if (!key_iv) {
        printerr("Memory allocation failed while generating key/IV\n");
        return 0;
    }

    /* Derive key and IV from password using PBKDF2 with SHA-256 */
    if (PKCS5_PBKDF2_HMAC(
            pass, strlen(pass), salt, sizeof(salt), 10000, EVP_sha256(), total_len, key_iv) != 1) {
        printerr("Error deriving key and IV from password using PBKDF2 with SHA-256\n");
        free(key_iv);
        return 0;
    }

    // Copy the derived key and IV to the output buffers
    memcpy(key, key_iv, key_len);
    if (iv_len > 0) {
        memcpy(iv, key_iv + key_len, iv_len);
    }

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
 *
 * @note The caller is responsible for freeing the returned data
 */
unsigned char* encrypt_data(const unsigned char* plaintext,
                            size_t               plaintext_len,
                            const char*          pass,
                            encryption           a,
                            mode                 m,
                            size_t*              encrypted_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        printerr("Error creating encryption context\n");
        return NULL;
    }

    const EVP_CIPHER* cipher_type = get_cipher(a, m);
    if (!cipher_type) {
        printerr("Invalid encryption algorithm or mode\n");
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
            printerr("Memory allocation failed for IV\n");
            EVP_CIPHER_CTX_free(ctx);
            free(key);
            return NULL;
        }
    }

    if (!generate_key_iv(pass, key, iv, key_len, iv_len)) {
        printerr("Error generating key/IV\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, cipher_type, NULL, key, iv) != 1) {
        printerr("Error initializing encryption\n");
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
        printerr("Memory allocation failed for ciphertext\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        printerr("Error during encryption\n");
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }
    *encrypted_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        printerr("Error during final encryption\n");
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
 * @note The caller is responsible for freeing the returned pointer
 */
unsigned char* decrypt_data(const unsigned char* ciphertext,
                            size_t               ciphertext_len,
                            const char*          pass,
                            encryption           a,
                            mode                 m,
                            size_t*              decrypted_len) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        printerr("Error creating decryption context\n");
        return NULL;
    }

    const EVP_CIPHER* cipher_type = get_cipher(a, m);
    if (!cipher_type) {
        printerr("Invalid encryption algorithm or mode\n");
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
            printerr("Memory allocation failed for IV\n");
            EVP_CIPHER_CTX_free(ctx);
            free(key);  // Free the key before returning
            return NULL;
        }
    }

    if (!generate_key_iv(pass, key, iv, key_len, iv_len)) {
        printerr("Error generating key/IV\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    if (EVP_DecryptInit_ex(ctx, cipher_type, NULL, key, iv) != 1) {
        printerr("Error initializing decryption\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    // Allocate memory for plaintext
    unsigned char* plaintext = malloc(ciphertext_len + EVP_CIPHER_block_size(cipher_type));
    if (!plaintext) {
        printerr("Memory allocation failed for plaintext\n");
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    int len;
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        printerr("Error during decryption\n");
        free(plaintext);  // Free the allocated plaintext on failure
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }

    *decrypted_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        printerr("Error during final decryption\n");
        free(plaintext);  // Free plaintext on failure
        EVP_CIPHER_CTX_free(ctx);
        free(key);
        if (iv)
            free(iv);
        return NULL;
    }
    *decrypted_len += len;

    // Clean up the resources
    EVP_CIPHER_CTX_free(ctx);
    free(key);
    if (iv)
        free(iv);

    return plaintext;  // Return the successfully decrypted plaintext
}
