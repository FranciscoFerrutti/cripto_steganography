#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "misc.h"
#include "steganography.h"

unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize);
unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize);
unsigned char *lsbi_decode(BMP_FILE *bmp, size_t *dataSize);

/**
 * @brief Extract a message from a BMP file using steganography
 *
 * @param carrierFile Path to the BMP file to extract the message from
 * @param outputfile Path to the output file to write the extracted message
 * @param method Steganography method to use
 *
 * Extracts information from a BMP file using steganography methods
 */
void extract(const char *carrierFile, const char *outputfile, steg method) {
    BMP_FILE *bmp;

    /* Read the carrier BMP file */
    bmp = read_bmp(carrierFile);
    if (!bmp) {
        fprintf(stderr, "Error: Could not read BMP file %s\n", carrierFile);
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
        case LSBI:
            extractedData = lsbi_decode(bmp, &dataSize);
            break;
        default:
            fprintf(stderr, "Error: Invalid steganography method\n");
            free_bmp(bmp);
            exit(1);
    }

    /* Free the BMP as we dont need it anymore*/
    free_bmp(bmp);

    if (!extractedData) {
        fprintf(stderr, "Error: Failed to extract data\n");
        exit(1);
    }

    /* Process the extracted data */
    extract_embedded_data(extractedData, dataSize, outputfile);

    /* Free the extracted data buffer */
    free(extractedData);

    /*******************************************************************/

    char dataSizeStr[20];
    snprintf(dataSizeStr, sizeof(dataSizeStr), "%zu", dataSize);
    print_table("Successfully extracted!!",
                "Output file",
                outputfile,
                "Steganography method",
                steg_str[method],
                "Size (bytes)",
                dataSizeStr,
                NULL);
}

/**
 * @brief Decode a message from a BMP file using the LSB1 method
 *
 * @param bmp BMP file structure to extract the message from
 * @param dataSize Pointer to store the size of the extracted data
 *
 * @return Pointer to the extracted data
 *
 * Decodes the message from the least significant bit of the red channel of each pixel
 */
unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize) {
    size_t maxBits = bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3 * 8;

    // Initial allocation for extractedData
    size_t         bufferSize    = 1024;  // random reasonable size
    unsigned char *extractedData = malloc(bufferSize);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    size_t   dataIndex = 0;
    int      bitIndex  = 7;  // Start with the most significant bit
    size_t   bitCount  = 0;  // Counter for total bits decoded
    uint32_t i, j;
    int      foundNullTerminator = 0;
    size_t   fileSize            = 0;
    int      fileSizeExtracted   = 0;

    // Extract the message from the BMP file pixel by pixel, color channel by color channel
    for (i = 0; i < bmp->infoHeader.biHeight && !foundNullTerminator && bitCount < maxBits; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && !foundNullTerminator && bitCount < maxBits;
             j++) {
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].red, &bmp->pixels[i][j].green, &bmp->pixels[i][j].blue};

            for (int k = 0; k < 3 && !foundNullTerminator && bitCount < maxBits; k++) {
                uint8_t lsb = *colors[k] & 0x01;  // Extract LSB
                // Set the current bit in the extracted data buffer
                extractedData[dataIndex] &= ~(1 << bitIndex);  // Clear the bit at bitIndex
                extractedData[dataIndex] |= lsb << bitIndex;   // Set the bit

                // Move to the next bit
                if (bitIndex == 0) {
                    bitIndex = 7;
                    dataIndex++;

                    // Ensure we have enough space in the buffer
                    if (dataIndex >= bufferSize) {
                        bufferSize *= 2;
                        unsigned char *temp = realloc(extractedData, bufferSize);
                        if (!temp) {
                            fprintf(stderr, "Error: Memory allocation failed\n");
                            free(extractedData);
                            return NULL;
                        }
                        extractedData = temp;
                    }

                    // If we've just read the fileSize
                    if (!fileSizeExtracted && dataIndex == sizeof(size_t)) {
                        memcpy(&fileSize, extractedData, sizeof(size_t));
                        fileSizeExtracted = 1;
                    }

                    // After reading fileData, start checking for null terminator
                    if (fileSizeExtracted && dataIndex >= sizeof(size_t) + fileSize) {
                        // We're now reading the extension
                        if (extractedData[dataIndex - 1] == '\0') {
                            foundNullTerminator = 1;
                        }
                    }
                }
                else {
                    bitIndex--;
                }

                bitCount++;
            }
        }
    }

    if (!foundNullTerminator) {
        fprintf(stderr, "Error: Null terminator after extension not found\n");
        free(extractedData);
        return NULL;
    }

    // Adjust the size of the buffer to the actual data size
    extractedData = realloc(extractedData, dataIndex);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed during final adjustment\n");
        return NULL;
    }

    *dataSize = dataIndex;  // Final size of the extracted data (number of bytes)
    return extractedData;
}

/**
 * @brief Decode a message from a BMP file using the LSB4 method
 *
 * @param bmp BMP file structure to extract the message from
 * @param dataSize Pointer to store the size of the extracted data
 *
 * @return Pointer to the extracted data
 *
 */
unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize) {
    size_t         bufferSize    = 1024;  // random reasonable size
    unsigned char *extractedData = malloc(bufferSize);

    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    size_t   dataIndex   = 0;
    int      nibbleIndex = 1;  // Start with high nibble
    size_t   nibbleCount = 0;
    uint32_t i, j;
    int      foundNullTerminator = 0;
    size_t   fileSize            = 0;
    int      fileSizeExtracted   = 0;
    size_t   maxNibbles          = bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3 * 2;

    // Initialize extractedData to zero
    memset(extractedData, 0, bufferSize);

    // Extract nibbles from the image
    for (i = 0; i < bmp->infoHeader.biHeight && !foundNullTerminator && nibbleCount < maxNibbles;
         i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && !foundNullTerminator && nibbleCount < maxNibbles;
             j++) {
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].red, &bmp->pixels[i][j].green, &bmp->pixels[i][j].blue};

            for (int k = 0; k < 3 && !foundNullTerminator && nibbleCount < maxNibbles; k++) {
                uint8_t nibble = *colors[k] & 0x0F;  // Extract lower nibble

                if (nibbleIndex == 1) {
                    extractedData[dataIndex] &= 0x0F;         // Clear high nibble
                    extractedData[dataIndex] |= nibble << 4;  // Set high nibble
                    nibbleIndex = 0;
                }
                else {
                    extractedData[dataIndex] &= 0xF0;    // Clear low nibble
                    extractedData[dataIndex] |= nibble;  // Set low nibble
                    nibbleIndex = 1;
                    dataIndex++;
                    // Ensure we have enough space in the buffer
                    if (dataIndex >= bufferSize) {
                        bufferSize *= 2;
                        unsigned char *temp = realloc(extractedData, bufferSize);
                        if (!temp) {
                            fprintf(stderr, "Error: Memory allocation failed\n");
                            free(extractedData);
                            return NULL;
                        }
                        extractedData = temp;
                    }
                    // If we've just read the fileSize
                    if (!fileSizeExtracted && dataIndex == sizeof(size_t)) {
                        memcpy(&fileSize, extractedData, sizeof(size_t));
                        fileSizeExtracted = 1;
                    }

                    // After reading fileData, start checking for null terminator
                    if (fileSizeExtracted && dataIndex >= sizeof(size_t) + fileSize) {
                        // We're now reading the extension
                        if (extractedData[dataIndex - 1] == '\0') {
                            foundNullTerminator = 1;
                        }
                    }
                }

                nibbleCount++;
            }
        }
    }

    if (!foundNullTerminator) {
        fprintf(stderr, "Error: Null terminator for extension not found\n");
        free(extractedData);
        return NULL;
    }
    // Resize to actual size
    extractedData = realloc(extractedData, dataIndex);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed during final adjustment\n");
        return NULL;
    }

    *dataSize = dataIndex;
    return extractedData;
}

unsigned char *lsbi_decode(BMP_FILE *bmp, size_t *dataSize) {
    uint32_t i, j;
    size_t   maxTwoBits =
        bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3;  // Total two-bit units in the image
    size_t         maxBytes      = maxTwoBits / 4;  // Each byte contains 4 two-bit units
    unsigned char *extractedData = malloc(maxBytes);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    size_t dataIndex    = 0;
    int    bitIndex     = 6;  // Start from the most significant bits
    size_t twoBitsCount = 0;

    // Initialize extractedData to zero
    memset(extractedData, 0, maxBytes);

    // Extract two bits from each color channel
    for (i = 0; i < bmp->infoHeader.biHeight && twoBitsCount < maxTwoBits; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && twoBitsCount < maxTwoBits; j++) {
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].red, &bmp->pixels[i][j].green, &bmp->pixels[i][j].blue};

            for (int k = 0; k < 3 && twoBitsCount < maxTwoBits; k++) {
                uint8_t twoBits = *colors[k] & 0x03;

                // Place the two bits into the current byte in the extracted data buffer
                extractedData[dataIndex] |= twoBits << bitIndex;

                if (bitIndex == 0) {
                    bitIndex = 6;
                    dataIndex++;
                    // Ensure we don't exceed the buffer size
                    if (dataIndex >= maxBytes) {
                        extractedData = realloc(extractedData, maxBytes + 1024);
                        if (!extractedData) {
                            fprintf(stderr, "Error: Memory allocation failed\n");
                            return NULL;
                        }
                        memset(extractedData + maxBytes, 0, 1024);
                        maxBytes += 1024;
                    }
                }
                else {
                    bitIndex -= 2;
                }

                twoBitsCount++;
            }
        }
    }

    *dataSize = dataIndex;
    return extractedData;
}
