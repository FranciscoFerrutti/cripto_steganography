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
        printerr("Memory allocation failed\n");
        return NULL;
    }

    // Step 1: Read the 4-bit inversion map from the first 4 color components
    uint8_t mapBits  = 0;
    int     bitsRead = 0;
    size_t  idx      = 0;  // Index of the current color component
    size_t  i, j, k, temp;

    while (bitsRead < 4 && idx < totalComponents) {
        i    = idx / (width * 3);
        temp = idx % (width * 3);
        j    = temp / 3;
        k    = temp % 3;

        if (i >= height || j >= width)
            break;

        PIXEL   pixel     = bmp->pixels[i][j];
        uint8_t colors[3] = {pixel.blue, pixel.green, pixel.red};

        uint8_t bit = colors[k] & 1;
        mapBits |= bit << (3 - bitsRead);
        bitsRead++;
        idx++;
    }

    if (bitsRead < 4) {
        printerr("Failed to read map bits\n");
        free(dataBuffer);
        return NULL;
    }

    // Step 2: Decode the hidden data using the inversion map
    size_t  dataBufferIndex = 0;
    uint8_t currentByte     = 0;
    int     bitIndex        = 0;
    int     stop            = 0;

    while (idx < totalComponents && !stop) {
        // Skip the red channel (k == 2)
        if ((idx % 3) == 2) {
            idx++;
            continue;
        }

        i    = idx / (width * 3);
        temp = idx % (width * 3);
        j    = temp / 3;
        k    = temp % 3;

        if (i >= height || j >= width)
            break;

        PIXEL   pixel      = bmp->pixels[i][j];
        uint8_t colors[3]  = {pixel.blue, pixel.green, pixel.red};
        uint8_t colorValue = colors[k];

        // Extract the pattern from the 2nd and 3rd least significant bits (LSBs)
        uint8_t pattern  = (colorValue >> 1) & 0x3;         // 2nd and 3rd LSBs
        uint8_t inverted = (mapBits >> (3 - pattern)) & 1;  // Determine if the bit is inverted

        // Extract the LSB and invert if necessary
        uint8_t bit = colorValue & 1;
        if (inverted)
            bit ^= 1;

        // Collect bits to form bytes
        currentByte |= bit << (7 - bitIndex);
        bitIndex++;

        if (bitIndex == 8) {
            dataBuffer[dataBufferIndex++] = currentByte;
            currentByte                   = 0;  // Reset for the next byte
            bitIndex                      = 0;

            // Check for data size after reading the first 4 bytes
            if (dataBufferIndex == 4) {
                uint32_t size;
                memcpy(&size, dataBuffer, 4);
                size      = ntohl(size);  // Convert from network byte order
                *dataSize = size;

                // Ensure the reported size fits within the maximum capacity
                if (4 + size + 1 > maxDataBytes) {
                    printerr("Size mismatch: hidden data is too large for this image\n");
                    free(dataBuffer);
                    return NULL;
                }
            }
            // Stop extraction when data size is reached
            else if (dataBufferIndex >= 4 + *dataSize) {
                if (!encrypted && dataBuffer[dataBufferIndex - 1] == '\0') {
                    stop = 1;
                }
                else if (encrypted) {
                    stop = 1;
                }
            }
        }

        idx++;
    }

    // Check if extraction was successful
    if (!stop) {
        printerr("End of image data reached before completing extraction\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}
