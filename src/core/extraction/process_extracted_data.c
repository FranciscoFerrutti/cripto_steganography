#include "extraction.h"

/**
 * @brief Process the extracted data by decrypting it if needed and writing it to a file
 *
 * @param dataBuffer Buffer containing the extracted data
 * @param outputFilePath Path to the output file
 * @param pass Password to decrypt the data
 * @param a Encryption algorithm to use
 * @param m Encryption mode to use
 *
 * @return 0 on success, -1 on failure
 *
 */
int process_extracted_data(const unsigned char *dataBuffer,
                           const char          *outputFilePath,
                           const char          *pass,
                           encryption           a,
                           mode                 m) {
    uint32_t             realSize;
    unsigned char       *decryptedData   = NULL;        // Pointer for decrypted data
    const unsigned char *finalDataBuffer = dataBuffer;  // Pointer to use for final data

    // Read the real size of the hidden data
    memcpy(&realSize, dataBuffer, sizeof(realSize));
    realSize = ntohl(realSize);

    // If a password is provided, decrypt the data
    if (pass != NULL) {
        size_t checkSize = 0;
        decryptedData =
            decrypt_data(dataBuffer + sizeof(realSize), realSize, pass, a, m, &checkSize);
        if (!decryptedData) {
            printerr("Error decrypting data\n");
            return -1;
        }

        // Update the data pointer to use the decrypted data
        finalDataBuffer = decryptedData;
        memcpy(&realSize, finalDataBuffer, sizeof(realSize));
        realSize = ntohl(realSize);
    }

    const unsigned char *fileData  = finalDataBuffer + sizeof(realSize);
    const char          *extension = (const char *) (fileData + realSize);

    // Ensure the file extension is null-terminated within the buffer
    size_t maxExtensionLen = strlen((const char *) finalDataBuffer) - (sizeof(realSize) + realSize);
    size_t extensionLen    = strnlen(extension, maxExtensionLen);
    if (extensionLen == maxExtensionLen) {
        printerr("File extension is not null-terminated\n");
        free(decryptedData);
        return -1;
    }

    // Construct the full output file path
    size_t fullPathLen        = strlen(outputFilePath) + extensionLen + 1;  // +1 for '\0'
    char  *fullOutputFilePath = malloc(fullPathLen);
    if (!fullOutputFilePath) {
        printerr("Memory allocation failed\n");
        free(decryptedData);
        return -1;
    }
    strcpy(fullOutputFilePath, outputFilePath);
    strncat(fullOutputFilePath, extension, extensionLen);
    fullOutputFilePath[fullPathLen - 1] = '\0';  // Ensure null-termination

    // Write the file data to the output file
    FILE *outFile = fopen(fullOutputFilePath, "wb");
    if (!outFile) {
        printerr("Failed to open output file %s\n", fullOutputFilePath);
        free(fullOutputFilePath);
        free(decryptedData);
        return -1;
    }

    size_t bytesWritten = fwrite(fileData, 1, realSize, outFile);
    if (bytesWritten != realSize) {
        printerr("Failed to write all data to output file\n");
        fclose(outFile);
        free(fullOutputFilePath);
        free(decryptedData);
        return -1;
    }

    fclose(outFile);
    free(fullOutputFilePath);
    free(decryptedData);

    return 0;
}
