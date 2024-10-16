#include "extraction.h"

int process_extracted_data(const unsigned char *dataBuffer,
                           const char          *outputFilePath,
                           const char          *pass,
                           encryption           a,
                           mode                 m) {
    uint32_t             realSize;
    unsigned char       *decryptedData   = NULL;        // Pointer for decrypted data
    const unsigned char *finalDataBuffer = dataBuffer;  // Pointer to handle both cases

    // Read the real size
    memcpy(&realSize, dataBuffer, 4);
    realSize = ntohl(realSize);
    // If a password is provided, decrypt the data
    if (pass != NULL) {
        size_t checkSize = 0;
        decryptedData    = decrypt_data(dataBuffer + 4, realSize, pass, a, m, &checkSize);
        if (!decryptedData) {
            printerr("Error decrypting data\n");
            return -1;
        }

        // Update pointer to use decrypted data
        finalDataBuffer = decryptedData;
        memcpy(&realSize, finalDataBuffer, 4);
        realSize = ntohl(realSize);
    }

    const unsigned char *fileData  = finalDataBuffer + 4;
    const char          *extension = (const char *) (fileData + realSize);

    if (extension[0] != '.') {
        printerr("Invalid file extension\n");
        free(decryptedData);  // Free if allocated
        return -1;
    }

    // Ensure the extension string is null-terminated within the buffer
    size_t max_extension_len = strlen((const char *) finalDataBuffer) - (4 + realSize);
    size_t extension_len     = strnlen(extension, max_extension_len);
    if (extension_len == max_extension_len) {
        printerr("File extension is not null-terminated\n");
        free(decryptedData);  // Free if allocated
        return -1;
    }

    // Construct the full output file path
    size_t fullPathLen        = strlen(outputFilePath) + extension_len + 1;  // +1 for '\0'
    char  *fullOutputFilePath = malloc(fullPathLen);
    if (!fullOutputFilePath) {
        printerr("Memory allocation failed\n");
        free(decryptedData);  // Free if allocated
        return -1;
    }
    strcpy(fullOutputFilePath, outputFilePath);
    strncat(fullOutputFilePath, extension, extension_len);
    fullOutputFilePath[fullPathLen - 1] = '\0';  // Ensure null-termination

    // Write the file data to the output file
    FILE *outFile = fopen(fullOutputFilePath, "wb");
    if (!outFile) {
        printerr("Failed to open output file %s\n", fullOutputFilePath);
        free(fullOutputFilePath);
        free(decryptedData);  // Free if allocated
        return -1;
    }

    size_t bytesWritten = fwrite(fileData, 1, realSize, outFile);
    if (bytesWritten != realSize) {
        printerr("Failed to write all data to output file\n");
        fclose(outFile);
        free(fullOutputFilePath);
        free(decryptedData);  // Free if allocated
        return -1;
    }

    fclose(outFile);
    free(fullOutputFilePath);
    free(decryptedData);  // Free if allocated

    return 0;
}