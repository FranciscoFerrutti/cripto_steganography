#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "encryption.h"
#include "misc.h"
#include "steganography.h"

unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize);
unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize);

int extract_embedded_data(const unsigned char *dataBuffer,
                          const char          *outputFilePath,
                          const char          *pass,
                          encryption           a,
                          mode                 m);

void extract(const char *carrierFile,
             const char *outputfile,
             steg        method,
             encryption  a,
             mode        m,
             const char *pass) {
    BMP_FILE *bmp;

    /* Read the carrier BMP file */
    bmp = read_bmp(carrierFile);

    if (!bmp) {
        printerr("Could not read BMP file %s\n", carrierFile);
        exit(1);
    }

    unsigned char *extractedData = NULL;
    size_t         dataSize      = 0;
    /* Select the steganography extraction method */
    switch (method) {
        case LSB1:
            extractedData = lsb1_decode(bmp, &dataSize);
            break;
        case LSB4:
            extractedData = lsb4_decode(bmp, &dataSize);
            break;

        default:
            printerr("Invalid steganography method\n");
            free_bmp(bmp);
            exit(1);
    }

    /* Free the BMP as we dont need it anymore*/
    free_bmp(bmp);

    if (!extractedData) {
        printerr("Error extracting data\n");
        exit(1);
    }

    /* Process the extracted data */
    printf("dataSize: %zu\n", dataSize);
    if (extract_embedded_data(extractedData, outputfile, pass, a, m) != 0) {
        printerr("Error processing extracted data\n");
        free(extractedData);
        exit(1);
    }

    /* Free the extracted data buffer */
    free(extractedData);

    /*******************************************************************/

    char dataSizeStr[20];
    snprintf(dataSizeStr, sizeof(dataSizeStr), "%zu", dataSize);
    print_table("Successfully extracted hidden data from BMP file",
                0xa6da95,
                "Output file",
                outputfile,
                "Steganography method",
                steg_str[method],
                "Size (bytes)",
                dataSizeStr,
                "Encryption Algorithm",
                encryption_str[a],
                "Encryption Mode",
                mode_str[m],
                "Password",
                pass,
                NULL);
}

unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize) {
    size_t width          = bmp->infoHeader.biWidth;
    size_t height         = bmp->infoHeader.biHeight;
    size_t max_data_bytes = (width * height * 3) / 8;  // Each pixel has 3 color components

    unsigned char *dataBuffer = malloc(max_data_bytes);
    if (!dataBuffer) {
        printerr("Memory allocation failed\n");
        return NULL;
    }

    size_t  dataBufferIndex = 0;  // Index of the next byte to write in dataBuffer
    uint8_t currentByte     = 0;  // Current byte being written to
    int     bitIndex        = 0;  // Index of the next bit to write in currentByte
    int     stop            = 0;  // Flag to stop the extraction process

    for (size_t i = 0; i < height && !stop; i++) {
        for (size_t j = 0; j < width && !stop; j++) {
            PIXEL   pixel     = bmp->pixels[i][j];
            uint8_t colors[3] = {pixel.blue, pixel.green, pixel.red};

            for (int k = 0; k < 3 && !stop; k++) {
                uint8_t bit = colors[k] & 1;
                currentByte |= bit << (7 - bitIndex);
                bitIndex++;

                if (bitIndex == 8) {
                    dataBuffer[dataBufferIndex++] = currentByte;
                    currentByte                   = 0;
                    bitIndex                      = 0;

                    // First 4 bytes contain the size of the hidden data
                    if (dataBufferIndex == 4) {
                        uint32_t size;
                        memcpy(&size, dataBuffer, 4);
                        size      = ntohl(size);
                        *dataSize = size;

                        if (4 + size + 1 > max_data_bytes) {  // +1 for at least '\0'
                            printerr("Size missmatch, this size is too big for this image\n");
                            free(dataBuffer);
                            return NULL;
                        }
                    }
                    else if (dataBufferIndex >= 4 + *dataSize) {
                        if (dataBuffer[dataBufferIndex - 1] == '\0') {
                            stop = 1;
                        }
                    }
                }
            }
        }
    }

    if (!stop) {
        printerr("End of image data reached before end of hidden data\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}

unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize) {
    size_t width          = bmp->infoHeader.biWidth;
    size_t height         = bmp->infoHeader.biHeight;
    size_t max_data_bytes = (width * height * 3) / 2;  // Each pixel has 3 color components

    unsigned char *dataBuffer = malloc(max_data_bytes);
    if (!dataBuffer) {
        printerr("Memory allocation failed\n");
        return NULL;
    }

    size_t  dataBufferIndex = 0;  // Index of the next byte to write in dataBuffer
    uint8_t currentByte     = 0;  // Current byte being written to
    int     bitIndex        = 0;  // Index of the next bit to write in currentByte
    int     stop            = 0;  // Flag to stop the extraction process

    for (size_t i = 0; i < height && !stop; i++) {
        for (size_t j = 0; j < width && !stop; j++) {
            PIXEL   pixel     = bmp->pixels[i][j];
            uint8_t colors[3] = {pixel.blue, pixel.green, pixel.red};

            for (int k = 0; k < 3 && !stop; k++) {
                uint8_t nibble = colors[k] & 0x0F;
                currentByte |= nibble << (4 - bitIndex);
                bitIndex += 4;

                if (bitIndex == 8) {
                    dataBuffer[dataBufferIndex++] = currentByte;
                    currentByte                   = 0;
                    bitIndex                      = 0;

                    // First 4 bytes contain the size of the hidden data
                    if (dataBufferIndex == 4) {
                        uint32_t size;
                        memcpy(&size, dataBuffer, 4);
                        size      = ntohl(size);
                        *dataSize = size;

                        if (4 + size + 1 > max_data_bytes) {  // +1 for at least '\0'
                            printerr("Size missmatch, this size is too big for this image\n");
                            free(dataBuffer);
                            return NULL;
                        }
                    }
                    else if (dataBufferIndex >= 4 + *dataSize) {
                        if (dataBuffer[dataBufferIndex - 1] == '\0') {
                            stop = 1;
                        }
                    }
                }
            }
        }
    }

    if (!stop) {
        printerr("End of image data reached before end of hidden data\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}

int extract_embedded_data(const unsigned char *dataBuffer,
                          const char          *outputFilePath,
                          const char          *pass,
                          encryption           a,
                          mode                 m) {
    printf("pass: %s\n", pass);
    printf("a: %d\n", a);
    printf("m: %d\n", m);

    // Read the real size from the first 4 bytes
    uint32_t realSize;
    memcpy(&realSize, dataBuffer, 4);
    realSize = ntohl(realSize);

    const unsigned char *fileData  = dataBuffer + 4;
    const char          *extension = (const char *) (fileData + realSize);

    // Validate the extension
    if (extension[0] != '.') {
        printerr("Invalid file extension\n");
        return -1;
    }

    // Ensure the extension string is null-terminated within the buffer
    size_t max_extension_len = strlen((const char *) dataBuffer) - (4 + realSize);
    size_t extension_len     = strnlen(extension, max_extension_len);
    if (extension_len == max_extension_len) {
        printerr("File extension is not null-terminated\n");
        return -1;
    }

    // Construct the full output file path
    size_t fullPathLen        = strlen(outputFilePath) + extension_len + 1;  // +1 for '\0'
    char  *fullOutputFilePath = malloc(fullPathLen);
    if (!fullOutputFilePath) {
        printerr("Memory allocation failed\n");
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
        return -1;
    }

    size_t bytesWritten = fwrite(fileData, 1, realSize, outFile);
    if (bytesWritten != realSize) {
        printerr("Failed to write all data to output file\n");
        fclose(outFile);
        free(fullOutputFilePath);
        return -1;
    }

    fclose(outFile);
    free(fullOutputFilePath);

    return 0;
}