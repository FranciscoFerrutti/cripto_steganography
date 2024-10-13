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
 * @param a Encryption algorithm to use
 * @param m Encryption mode to use
 * @param pass Password to use for encryption
 *
 * Extracts information from a BMP file using steganography and encryption methods then writes it to
 * an output file
 */
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
        print_table("Error: Could not read BMP file", 0xFF0000, "BMP file", carrierFile, NULL);
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
            print_table("Invalid steganography method", 0xFF0000, "Error", "unknown method", NULL);
            free_bmp(bmp);
            exit(1);
    }

    /* Free the BMP as we dont need it anymore*/
    free_bmp(bmp);

    if (!extractedData) {
        print_table("Error: Could not extract data", 0xFF0000, "Error", "no extracted data", NULL);
        exit(1);
    }

    /* Process the extracted data */
    if (extract_embedded_data(extractedData, dataSize, outputfile, pass, a, m) != 0) {
        print_table("Error: Could not extract data", 0xFF0000, "Error", "extraction failed", NULL);
        free(extractedData);
        exit(1);
    }

    /* Free the extracted data buffer */
    free(extractedData);

    /*******************************************************************/

    char dataSizeStr[20];
    snprintf(dataSizeStr, sizeof(dataSizeStr), "%zu", dataSize);
    print_table("Successfully extracted",
                0x00FF00,
                "Output file",
                outputfile,
                "Steganography method",
                steg_str[method],
                "Size (bytes)",
                dataSizeStr,
                "Encryption Algorithm",
                encryption_str[a],
                "Mode",
                mode_str[m],
                "Password",
                pass,
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
    size_t maxBits       = bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3 * 8;
    size_t totalBitsRead = 0;

    // Initial allocation for extractedData
    size_t         bufferSize    = 1024;  // Initial reasonable size
    unsigned char *extractedData = malloc(bufferSize);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    size_t   dataIndex = 0;
    int      bitIndex  = 7;  // Start with the most significant bit
    uint32_t i, j;
    uint32_t embeddedDataSize = 0;  // Size of the embedded data after the size field
    int      sizeFieldRead    = 0;  // Flag to indicate if the size field has been read

    // Extract the message from the BMP file pixel by pixel, color channel by color channel
    for (i = 0; i < bmp->infoHeader.biHeight && totalBitsRead < maxBits; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && totalBitsRead < maxBits; j++) {
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].red, &bmp->pixels[i][j].green, &bmp->pixels[i][j].blue};

            for (int k = 0; k < 3 && totalBitsRead < maxBits; k++) {
                uint8_t lsb = *colors[k] & 0x01;  // Extract LSB

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

                // Set the current bit in the extracted data buffer
                extractedData[dataIndex] &= ~(1 << bitIndex);  // Clear the bit at bitIndex
                extractedData[dataIndex] |= lsb << bitIndex;   // Set the bit

                if (bitIndex == 0) {
                    bitIndex = 7;
                    dataIndex++;
                }
                else {
                    bitIndex--;
                }

                totalBitsRead++;

                // If we've read 4 bytes (32 bits), extract the size field
                if (!sizeFieldRead && dataIndex == sizeof(uint32_t)) {
                    memcpy(&embeddedDataSize, extractedData, sizeof(uint32_t));
                    sizeFieldRead = 1;
                }

                // If we've read all the embedded data, stop reading
                if (sizeFieldRead && dataIndex >= sizeof(uint32_t) + embeddedDataSize) {
                    goto extraction_done;
                }
            }
        }
    }

extraction_done:

    if (!sizeFieldRead) {
        fprintf(stderr, "Error: Failed to read the size field from the image\n");
        free(extractedData);
        return NULL;
    }

    // Adjust the size of the buffer to the actual data size
    size_t totalDataSize = sizeof(uint32_t) + embeddedDataSize;  // Size field + embedded data
    extractedData        = realloc(extractedData, totalDataSize);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed during final adjustment\n");
        return NULL;
    }

    *dataSize = totalDataSize;  // Final size of the extracted data
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
    size_t maxNibbles       = bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3 * 2;
    size_t totalNibblesRead = 0;

    // Initial allocation for extractedData
    size_t         bufferSize    = 1024;  // Initial reasonable size
    unsigned char *extractedData = malloc(bufferSize);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    size_t   dataIndex   = 0;
    int      nibbleIndex = 1;  // Start with high nibble
    uint32_t i, j;
    uint32_t embeddedDataSize = 0;  // Size of the embedded data after the size field
    int      sizeFieldRead    = 0;  // Flag to indicate if the size field has been read

    // Initialize extractedData to zero
    memset(extractedData, 0, bufferSize);

    // Extract nibbles from the image
    for (i = 0; i < bmp->infoHeader.biHeight && totalNibblesRead < maxNibbles; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && totalNibblesRead < maxNibbles; j++) {
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].red, &bmp->pixels[i][j].green, &bmp->pixels[i][j].blue};

            for (int k = 0; k < 3 && totalNibblesRead < maxNibbles; k++) {
                uint8_t nibble = *colors[k] & 0x0F;  // Extract lower nibble

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
                }

                totalNibblesRead++;

                // If we've read 4 bytes (8 nibbles), extract the size field
                if (!sizeFieldRead && dataIndex == sizeof(uint32_t) && nibbleIndex == 1) {
                    memcpy(&embeddedDataSize, extractedData, sizeof(uint32_t));
                    sizeFieldRead = 1;
                }

                // If we've read all the embedded data, stop reading
                if (sizeFieldRead && dataIndex >= sizeof(uint32_t) + embeddedDataSize &&
                    nibbleIndex == 1) {
                    goto extraction_done;
                }
            }
        }
    }

extraction_done:

    if (!sizeFieldRead) {
        fprintf(stderr, "Error: Failed to read the size field from the image\n");
        free(extractedData);
        return NULL;
    }

    // Adjust the size of the buffer to the actual data size
    size_t totalDataSize = sizeof(uint32_t) + embeddedDataSize;  // Size field + embedded data
    extractedData        = realloc(extractedData, totalDataSize);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed during final adjustment\n");
        return NULL;
    }

    *dataSize = totalDataSize;  // Final size of the extracted data
    return extractedData;
}

unsigned char *lsbi_decode(BMP_FILE *bmp, size_t *dataSize) {
    size_t maxTwoBits       = bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3;
    size_t totalTwoBitsRead = 0;

    // Initial allocation for extractedData
    size_t         bufferSize    = 1024;  // Initial reasonable size
    unsigned char *extractedData = malloc(bufferSize);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return NULL;
    }

    size_t   dataIndex = 0;
    int      bitIndex  = 6;  // Start from the most significant bits
    uint32_t i, j;
    uint32_t embeddedDataSize = 0;  // Size of the embedded data after the size field
    int      sizeFieldRead    = 0;  // Flag to indicate if the size field has been read

    // Initialize extractedData to zero
    memset(extractedData, 0, bufferSize);

    // Extract two bits from each color channel
    for (i = 0; i < bmp->infoHeader.biHeight && totalTwoBitsRead < maxTwoBits; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && totalTwoBitsRead < maxTwoBits; j++) {
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].red, &bmp->pixels[i][j].green, &bmp->pixels[i][j].blue};

            for (int k = 0; k < 3 && totalTwoBitsRead < maxTwoBits; k++) {
                uint8_t twoBits = *colors[k] & 0x03;  // Extract two LSBs

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

                // Place the two bits into the current byte in the extracted data buffer
                extractedData[dataIndex] |= twoBits << bitIndex;

                if (bitIndex == 0) {
                    bitIndex = 6;
                    dataIndex++;
                }
                else {
                    bitIndex -= 2;
                }

                totalTwoBitsRead += 2;

                // If we've read 4 bytes (16 two-bit units), extract the size field
                if (!sizeFieldRead && dataIndex == sizeof(uint32_t) && bitIndex == 6) {
                    memcpy(&embeddedDataSize, extractedData, sizeof(uint32_t));
                    sizeFieldRead = 1;
                }

                // If we've read all the embedded data, stop reading
                if (sizeFieldRead && dataIndex >= sizeof(uint32_t) + embeddedDataSize &&
                    bitIndex == 6) {
                    goto extraction_done;
                }
            }
        }
    }

extraction_done:

    if (!sizeFieldRead) {
        fprintf(stderr, "Error: Failed to read the size field from the image\n");
        free(extractedData);
        return NULL;
    }

    // Adjust the size of the buffer to the actual data size
    size_t totalDataSize = sizeof(uint32_t) + embeddedDataSize;  // Size field + embedded data
    extractedData        = realloc(extractedData, totalDataSize);
    if (!extractedData) {
        fprintf(stderr, "Error: Memory allocation failed during final adjustment\n");
        return NULL;
    }

    *dataSize = totalDataSize;  // Final size of the extracted data
    return extractedData;
}