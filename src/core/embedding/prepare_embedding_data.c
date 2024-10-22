#include "embedding.h"

#define UINT32_SIZE sizeof(uint32_t)  // Size of a 32-bit unsigned integer = 4 bytes
#define NULL_TERMINATOR_SIZE 1        // Null terminator for strings
#define DEFAULT_EXTENSION ".txt"      // Default extension if none is found
#define SEEK_START 0                  // Seek start position for fseek
#define SEEK_END 2                    // Seek end position for fseek
#define EXTENSION_SEPARATOR '.'       // Magic string for file extension separator


/**
 * @brief Prepare the data to be embedded into the carrier file
 * 
 * @param message_file Path to the message file
 * @param total_data_size Pointer to store the total size of the embedding data
 * @param password Password to encrypt the data
 * @param encryption_type Encryption algorithm to use
 * @param mode_type Encryption mode to use
 * 
 * @return Pointer to the embedding data
 * 
 * @note The caller is responsible for freeing the returned pointer
 * @note To ensure encryption a password must be provided
 */
unsigned char *prepare_embedding_data(const char *message_file,
                                      size_t     *total_data_size,
                                      const char *password,
                                      encryption  encryption_type,
                                      mode        mode_type) {
    // Open file and handle error if unable to open
    FILE *file = fopen(message_file, "rb");
    if (!file) {
        printerr("Could not open message file: %s\n", message_file);
        return NULL;
    }

    // Get file size efficiently
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    // Allocate memory to read the file data
    unsigned char *file_data = malloc(file_size);
    if (!file_data) {
        printerr("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read file data in one go
    fread(file_data, 1, file_size, file);
    fclose(file);

    // Get the file extension, or use the default if none is found
    const char *extension = strrchr(message_file, EXTENSION_SEPARATOR);
    if (!extension) {
        extension = DEFAULT_EXTENSION;
    }

    size_t extension_length = strlen(extension) + NULL_TERMINATOR_SIZE;

    // Calculate embedding data size (file size + file data + extension)
    size_t         embedding_data_size = UINT32_SIZE + file_size + extension_length;
    unsigned char *embedding_data      = malloc(embedding_data_size);
    if (!embedding_data) {
        printerr("Memory allocation failed\n");
        free(file_data);
        return NULL;
    }

    // Store file size in network byte order
    uint32_t file_size_32 = htonl((uint32_t) file_size);
    memcpy(embedding_data, &file_size_32, UINT32_SIZE);          // Copy file size
    memcpy(embedding_data + UINT32_SIZE, file_data, file_size);  // Copy file data
    memcpy(
        embedding_data + UINT32_SIZE + file_size, extension, extension_length);  // Copy extension

    free(file_data);  // Free the original file data since it's already copied

    // Handle encryption if necessary
    if (password != NULL) {
        size_t         encrypted_size;
        unsigned char *encrypted_data = encrypt_data(embedding_data,
                                                     embedding_data_size,
                                                     password,
                                                     encryption_type,
                                                     mode_type,
                                                     &encrypted_size);

        if (!encrypted_data) {
            printerr("Error encrypting data\n");
            free(embedding_data);
            return NULL;
        }

        // Resize `embedding_data` buffer to fit the encrypted data
        embedding_data = realloc(embedding_data, UINT32_SIZE + encrypted_size);
        if (!embedding_data) {
            printerr("Memory reallocation failed\n");
            free(encrypted_data);
            return NULL;
        }

        // Copy encrypted size and encrypted data back into `embedding_data`
        uint32_t encrypted_size_32 = htonl((uint32_t) encrypted_size);
        memcpy(embedding_data, &encrypted_size_32, UINT32_SIZE);  // Copy encrypted size
        memcpy(
            embedding_data + UINT32_SIZE, encrypted_data, encrypted_size);  // Copy encrypted data

        free(encrypted_data);  // Free temporary encrypted data buffer

        *total_data_size = UINT32_SIZE + encrypted_size;
        return embedding_data;
    }

    // If no encryption, return the embedding data directly
    *total_data_size = embedding_data_size;
    return embedding_data;
}
