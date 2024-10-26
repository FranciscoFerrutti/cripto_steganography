#include <ctype.h>

#include "extraction.h"
/**
 * @brief Extract hidden data from a BMP file using the LSB1 steganography method
 *
 * @param bmp BMP file structure to extract data from
 * @param dataSize Pointer to store the size of the extracted data
 * @param encrypted Flag to indicate if the data is encrypted
 *
 * @return Pointer to the extracted data buffer
 *
 * @note The caller is responsible for freeing the returned buffer
 */
unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted) {
    size_t width        = bmp->infoHeader.biWidth;
    size_t height       = bmp->infoHeader.biHeight;
    size_t maxDataBytes = (width * height * 3) / 8;  // Each pixel has 3 color components

    // Allocate memory for the extracted data buffer
    unsigned char *dataBuffer = malloc(maxDataBytes);
    if (!dataBuffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    size_t  dataBufferIndex    = 0;  // Index for writing bytes into the data buffer
    uint8_t currentByte        = 0;  // Byte currently being constructed from bits
    int     bitIndex           = 0;  // Tracks position within the current byte (0-7)
    int     extractionComplete = 0;  // Flag to stop extraction when done
    int     extensionFound     = 0;  // Flag to indicate if the file extension start has been found

    // Iterate over each pixel in the BMP image
    for (size_t i = 0; i < height && !extractionComplete; ++i) {
        for (size_t j = 0; j < width && !extractionComplete; ++j) {
            PIXEL   pixel     = bmp->pixels[i][j];
            uint8_t colors[3] = {pixel.blue, pixel.green, pixel.red};

            // Extract the LSB from each color component
            for (int k = 0; k < 3 && !extractionComplete; ++k) {
                uint8_t bit = colors[k] & 1;
                currentByte = (currentByte << 1) | bit;
                ++bitIndex;

                // If we have a full byte, store it in the data buffer
                if (bitIndex == 8) {
                    dataBuffer[dataBufferIndex++] = currentByte;
                    currentByte                   = 0;
                    bitIndex                      = 0;

                    // The first 4 bytes represent the size of the hidden data
                    if (dataBufferIndex == 4) {
                        uint32_t size;
                        memcpy(&size, dataBuffer, 4);
                        size      = ntohl(size);  // Convert from network byte order
                        *dataSize = size;

                        // Ensure the reported size fits within the maximum capacity
                        if (4 + *dataSize > maxDataBytes) {
                            fprintf(stderr, "Size mismatch: read size too large\n");
                            free(dataBuffer);
                            return NULL;
                        }
                    }

                    // Check if extraction should be stopped
                    if (dataBufferIndex >= 4 + *dataSize) {
                        if (!encrypted) {
                            if (!extensionFound && dataBuffer[dataBufferIndex - 1] == '.') {
                                extensionFound = 1;
                            }
                            else if (extensionFound && dataBuffer[dataBufferIndex - 1] == '\0') {
                                extractionComplete = 1;
                            }
                        }
                        else {
                            extractionComplete = 1;
                        }
                    }

                    // Prevent buffer overflow
                    if (dataBufferIndex >= maxDataBytes) {
                        extractionComplete = 1;
                    }
                }
            }
        }
    }

    // If the loop ended without extracting all data, report an error
    if (!extractionComplete) {
        fprintf(stderr, "End of image data reached before completing extraction\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}