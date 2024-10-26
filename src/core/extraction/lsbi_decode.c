#include "extraction.h"

/**
 * @brief Extract hidden data from a BMP file using the LSBI steganography method
 *
 * @param bmp BMP file structure to extract data from
 * @param dataSize Pointer to store the size of the extracted data
 * @param encrypted Flag to indicate if the data is encrypted
 *
 * @return Pointer to the extracted data buffer
 *
 * @note The caller is responsible for freeing the returned buffer
 */

unsigned char *lsbi_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted) {
    size_t width  = bmp->infoHeader.biWidth;
    size_t height = bmp->infoHeader.biHeight;

    size_t totalComponents = width * height * 3;  // Total color components (R, G, B)
    size_t maxDataBits     = width * height * 2;  // Only green and blue channels are used
    size_t maxDataBytes    = maxDataBits / 8;     // Maximum bytes that can be extracted

    // Allocate memory for the extracted data buffer
    unsigned char *dataBuffer = malloc(maxDataBytes);
    if (!dataBuffer) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Step 1: Read the 4-bit inversion map from the first 4 color components
    uint8_t inversionMap = 0;
    int     bitsRead     = 0;
    size_t  idx          = 0;  // Index of the current color component

    while (bitsRead < 4 && idx < totalComponents) {
        size_t i    = idx / (width * 3);
        size_t temp = idx % (width * 3);
        size_t j    = temp / 3;
        size_t k    = temp % 3;

        if (i >= height || j >= width)
            break;

        PIXEL   pixel     = bmp->pixels[i][j];
        uint8_t colors[3] = {pixel.blue, pixel.green, pixel.red};

        uint8_t bit = colors[k] & 1;
        inversionMap |= bit << (3 - bitsRead);  // Build the inversion map
        bitsRead++;
        idx++;
    }

    if (bitsRead < 4) {
        fprintf(stderr, "Failed to read inversion map bits\n");
        free(dataBuffer);
        return NULL;
    }

    // Step 2: Decode the hidden data using the inversion map
    size_t  dataBufferIndex    = 0;
    uint8_t currentByte        = 0;
    int     bitIndex           = 0;
    int     extractionComplete = 0;
    int     extensionFound     = 0;

    while (idx < totalComponents && !extractionComplete) {
        // Skip the red channel (k == 2)
        if ((idx % 3) == 2) {
            idx++;
            continue;
        }

        size_t i    = idx / (width * 3);
        size_t temp = idx % (width * 3);
        size_t j    = temp / 3;
        size_t k    = temp % 3;

        if (i >= height || j >= width)
            break;

        PIXEL   pixel      = bmp->pixels[i][j];
        uint8_t colors[3]  = {pixel.blue, pixel.green, pixel.red};
        uint8_t colorValue = colors[k];

        // Extract the pattern from the 2nd and 3rd least significant bits (LSBs)
        uint8_t pattern  = (colorValue >> 1) & 0x03;             // 2nd and 3rd LSBs
        uint8_t inverted = (inversionMap >> (3 - pattern)) & 1;  // Determine if the bit is inverted

        // Extract the LSB and invert if necessary
        uint8_t bit = colorValue & 1;
        if (inverted)
            bit ^= 1;

        // Collect bits to form bytes
        currentByte = (currentByte << 1) | bit;
        bitIndex++;

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
                    fprintf(stderr, "Size mismatch: hidden data is too large for this image\n");
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

                // Prevent buffer overflow
                if (dataBufferIndex >= maxDataBytes) {
                    extractionComplete = 1;
                }
            }
        }

        idx++;
    }

    // Check if extraction was successful
    if (!extractionComplete) {
        fprintf(stderr, "End of image data reached before completing extraction\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}