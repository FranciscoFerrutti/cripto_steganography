#include "embedding.h"

#include "encryption.h"
/**
 * @brief Prepare the data to be embedded into a BMP file
 *
 * @param messageFile Path to the file containing the message to embed
 * @param totalDataSize Pointer to store the total size of the embedding data in bytes
 * @param pass Password to use for encryption
 * @param a Encryption algorithm to use
 * @param m Encryption mode to use
 *
 * @return Pointer to the embedding data
 */
unsigned char* prepare_embedding_data(
    const char* messageFile, size_t* totalDataSize, const char* pass, encryption a, mode m) {
    FILE* file = fopen(messageFile, "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open message file %s\n", messageFile);
        return NULL;
    }

    // Determine the file size and go to the beginning of the file
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Read the file data
    unsigned char* fileData = malloc(fileSize);
    if (!fileData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    fread(fileData, 1, fileSize, file);
    fclose(file);

    // Get the file extension
    const char* extension = strrchr(messageFile, '.');
    if (!extension) {
        extension = ".txt";  // Default extension if none is found
    }

    size_t extensionLength = strlen(extension) + 1;  // Include the null terminator

    // totalDataSize = size of the file size (4 bytes) + file size + extension size
    size_t         embeddingDataSize = sizeof(uint32_t) + fileSize + extensionLength;
    unsigned char* embeddingData     = malloc(embeddingDataSize);
    if (!embeddingData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
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
        unsigned char* encrypted_data =
            encrypt_data(embeddingData, embeddingDataSize, pass, a, m, &encrypted_len);
        if (!encrypted_data) {
            fprintf(stderr, "Error encrypting data\n");
            free(embeddingData);
            return NULL;
        }

        free(embeddingData);

        // Prepare the final data to embed: encrypted data size (uint32_t) + encrypted data
        *totalDataSize           = sizeof(uint32_t) + encrypted_len;
        unsigned char* finalData = malloc(*totalDataSize);
        if (!finalData) {
            fprintf(stderr, "Error: Memory allocation failed\n");
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
        unsigned char* finalData = malloc(*totalDataSize);
        if (!finalData) {
            fprintf(stderr, "Error: Memory allocation failed\n");
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

/**
 * @brief Extract the embedded data from a buffer and write it to a file
 *
 * @param dataBuffer Buffer containing the embedded data
 * @param dataSize Size of the data buffer
 * @param outputFilePath Path to the output file (without extension)
 * @param pass Password used for decryption
 * @param a Encryption algorithm used
 * @param m Encryption mode used
 *
 */
int extract_embedded_data(const unsigned char* dataBuffer,
                          size_t               dataSize,
                          const char*          outputFilePath,
                          const char*          pass,
                          encryption           a,
                          mode                 m) {
    size_t offset = 0;

    // Read the size of the encrypted or plain data (first 4 bytes)
    if (dataSize < sizeof(uint32_t)) {
        fprintf(stderr, "Error: Data size is too small to contain a valid size header\n");
        return -1;
    }

    uint32_t embeddedDataSize;
    memcpy(&embeddedDataSize, dataBuffer, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (embeddedDataSize > dataSize - offset) {
        fprintf(stderr, "Error: Invalid embedded data size\n");
        return -1;
    }

    const unsigned char* embeddedData = dataBuffer + offset;

    // If encryption is used, decrypt the data
    unsigned char* decryptedData = NULL;
    size_t         decryptedSize = 0;

    if (a != ENC_NONE) {
        // Decrypt the embedded data
        decryptedData = decrypt_data(embeddedData, embeddedDataSize, pass, a, m, &decryptedSize);
        if (!decryptedData) {
            fprintf(stderr, "Error decrypting data\n");
            return -1;
        }
    }
    else {
        // No encryption; use the data as is
        decryptedData = malloc(embeddedDataSize);
        if (!decryptedData) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            return -1;
        }
        memcpy(decryptedData, embeddedData, embeddedDataSize);
        decryptedSize = embeddedDataSize;
    }

    offset = 0;

    // At this point, decryptedData contains the original embedding data
    // Extract the size of the original file (next 4 bytes)
    if (decryptedSize < sizeof(uint32_t)) {
        fprintf(stderr, "Error: Decrypted data size is too small to contain a valid message\n");
        free(decryptedData);
        return -1;
    }

    uint32_t messageSize;
    memcpy(&messageSize, decryptedData, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    if (messageSize > decryptedSize - offset) {
        fprintf(stderr, "Error: Invalid message size\n");
        free(decryptedData);
        return -1;
    }

    // Extract the message data
    const unsigned char* messageData = decryptedData + offset;
    offset += messageSize;

    if (offset >= decryptedSize) {
        fprintf(stderr, "Error: No file extension found\n");
        free(decryptedData);
        return -1;
    }

    // Extract the file extension
    const char* extension       = (const char*) (decryptedData + offset);
    size_t      extensionLength = strlen(extension) + 1;  // Include null terminator

    if (extensionLength <= 1 || extension[0] != '.') {
        fprintf(stderr, "Error: Invalid file extension\n");
        free(decryptedData);
        return -1;
    }

    // Build the output file name with the extracted extension
    char outputFileWithExtension[512];
    snprintf(outputFileWithExtension,
             sizeof(outputFileWithExtension),
             "%s%s",
             outputFilePath,
             extension);

    FILE* outputFile = fopen(outputFileWithExtension, "wb");
    if (!outputFile) {
        fprintf(stderr, "Error: Could not open output file %s\n", outputFileWithExtension);
        free(decryptedData);
        return -1;
    }

    // Write the extracted message data to the output file
    fwrite(messageData, 1, messageSize, outputFile);
    fclose(outputFile);

    free(decryptedData);

    return 0;
}