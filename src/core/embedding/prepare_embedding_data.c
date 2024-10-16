#include "embedding.h"

unsigned char *prepare_embedding_data(
    const char *messageFile, size_t *totalDataSize, const char *pass, encryption a, mode m) {
    FILE *file = fopen(messageFile, "rb");
    if (!file) {
        printerr("Could not open message file %s\n", messageFile);
        return NULL;
    }

    // Determine the file size and go to the beginning of the file
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Read the file data
    unsigned char *fileData = malloc(fileSize);
    if (!fileData) {
        printerr("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    fread(fileData, 1, fileSize, file);
    fclose(file);

    // Get the file extension
    const char *extension = strrchr(messageFile, '.');
    if (!extension) {
        extension = ".txt";  // Default extension if none is found
    }

    size_t extensionLength = strlen(extension) + 1;  // Include the null terminator

    // totalDataSize = size of the file size (4 bytes) + file size + extension size
    size_t         embeddingDataSize = sizeof(uint32_t) + fileSize + extensionLength;
    unsigned char *embeddingData     = malloc(embeddingDataSize);
    if (!embeddingData) {
        printerr("Memory allocation failed\n");
        free(fileData);
        return NULL;
    }

    // Copy the size (uint32_t), data, and extension into the embeddingData buffer
    uint32_t fileSize32 = (uint32_t) fileSize;  // Ensure file size is 4 bytes
    memcpy(embeddingData, &fileSize32, sizeof(uint32_t));
    memcpy(embeddingData + sizeof(uint32_t), fileData, fileSize);
    memcpy(embeddingData + sizeof(uint32_t) + fileSize, extension, extensionLength);

    free(fileData);

    // Encrypt the embedding data if encryption is enabled
    if (a != ENC_NONE) {
        size_t         encrypted_len;
        unsigned char *encrypted_data =
            encrypt_data(embeddingData, embeddingDataSize, pass, a, m, &encrypted_len);
        if (!encrypted_data) {
            printerr("Error encrypting data\n");
            free(embeddingData);
            return NULL;
        }

        free(embeddingData);

        // Prepare the final data to embed: encrypted data size (uint32_t) + encrypted data
        *totalDataSize           = sizeof(uint32_t) + encrypted_len;
        unsigned char *finalData = malloc(*totalDataSize);
        if (!finalData) {
            printerr("Memory allocation failed\n");
            free(encrypted_data);
            return NULL;
        }

        uint32_t encrypted_len32 = (uint32_t) encrypted_len;  // Ensure size is 4 bytes
        memcpy(finalData, &encrypted_len32, sizeof(uint32_t));
        memcpy(finalData + sizeof(uint32_t), encrypted_data, encrypted_len);

        free(encrypted_data);

        return finalData;
    }
    else {
        // No encryption: prepare the final data to embed: data size (uint32_t) + data
        *totalDataSize           = sizeof(uint32_t) + embeddingDataSize;
        unsigned char *finalData = malloc(*totalDataSize);
        if (!finalData) {
            printerr("Memory allocation failed\n");
            free(embeddingData);
            return NULL;
        }

        uint32_t embeddingDataSize32 = (uint32_t) embeddingDataSize;  // Ensure size is 4 bytes
        memcpy(finalData, &embeddingDataSize32, sizeof(uint32_t));
        memcpy(finalData + sizeof(uint32_t), embeddingData, embeddingDataSize);

        free(embeddingData);

        return finalData;
    }
}