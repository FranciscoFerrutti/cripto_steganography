#include "steg_utils.h"

/**
 * @brief Prepare the data to be embedded into a BMP file
 *
 * @param messageFile Path to the file containing the message to embed
 * @param totalDataSize Pointer to store the total size of the embedding data in bytes
 *
 * @return Pointer to the embedding data
 */
unsigned char* prepare_embedding_data(const char* messageFile, size_t* totalDataSize) {
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
    /**
     * @todo averiguar si se puede hacer algo que lo lea de los headers directamente
     */
    const char* extension = strrchr(messageFile, '.');
    if (!extension) {
        extension = ".txt";  // Default extension if none is found
    }

    size_t extensionLength = strlen(extension) + 1;  // Include the null terminator

    // totalDataSize = size of the file size + size of the file data + size of the extension
    *totalDataSize               = sizeof(size_t) + fileSize + extensionLength;
    unsigned char* embeddingData = malloc(*totalDataSize);
    if (!embeddingData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(fileData);
        return NULL;
    }

    // Copy the size, data, and extension into the embeddingData buffer
    memcpy(embeddingData, &fileSize, sizeof(size_t));
    memcpy(embeddingData + sizeof(size_t), fileData, fileSize);
    memcpy(embeddingData + sizeof(size_t) + fileSize, extension, extensionLength);

    free(fileData);
    return embeddingData;
}

/**
 * @brief Extract the embedded data from a buffer and write it to a file
 *
 * @param dataBuffer Buffer containing the embedded data
 * @param dataSize Size of the data buffer
 * @param outputFile Path to the output file
 *
 */
void extract_embedded_data(const unsigned char* dataBuffer,
                           size_t               dataSize,
                           const char*          outputFilePath) {
    size_t offset = 0;

    if (dataSize < sizeof(size_t)) {
        fprintf(stderr, "Error: Data size is too small to contain a valid message\n");
        return;
    }

    // Extract the size of the original message
    size_t messageSize;
    memcpy(&messageSize, dataBuffer, sizeof(size_t));
    offset += sizeof(size_t);

    if (messageSize > dataSize - offset) {
        fprintf(stderr, "Error: Invalid message size\n");
        return;
    }

    // Extract the message data
    const unsigned char* messageData = dataBuffer + offset;
    offset += messageSize;

    if (offset >= dataSize) {
        fprintf(stderr, "Error: No file extension found\n");
        return;
    }

    // Extract the file extension
    const char* extension       = (const char*) (dataBuffer + offset);
    size_t      extensionLength = strlen(extension);
    if (extensionLength == 0 || extension[0] != '.') {
        fprintf(stderr, "Error: Invalid file extension\n");
        return;
    }

    // Open the specified output file path
    FILE* outputFile = fopen(outputFilePath, "wb");
    if (!outputFile) {
        fprintf(stderr, "Error: Could not open output file %s\n", outputFilePath);
        return;
    }

    // Write the extracted data to the output file
    fwrite(messageData, 1, messageSize, outputFile);
    fclose(outputFile);
}
