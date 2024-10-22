#include "extraction.h"

/**
 * @brief Extract hidden data from a BMP file using the LSB4 steganography method
 *
 * @param bmp BMP file structure to extract data from
 * @param dataSize Pointer to store the size of the extracted data
 * @param encrypted Flag to indicate if the data is encrypted
 *
 * @return Pointer to the extracted data buffer
 *
 * @note The caller is responsible for freeing the returned buffer
 */
unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted) {
    size_t width        = bmp->infoHeader.biWidth;
    size_t height       = bmp->infoHeader.biHeight;
    size_t maxDataBytes = (width * height * 3) / 2;  // Each pixel has 3 color components

    // Allocate memory for the extracted data buffer
    unsigned char *dataBuffer = malloc(maxDataBytes);
    if (!dataBuffer) {
        printerr("Memory allocation failed\n");
        return NULL;
    }

    size_t  dataBufferIndex = 0;  // Index for writing bytes into the data buffer
    uint8_t currentByte     = 0;  // Byte currently being constructed from nibbles
    int     bitIndex        = 0;  // Tracks position within the current byte (0 or 4)
    int     stop            = 0;  // Flag to stop extraction when done

    // Iterate over each pixel in the BMP image
    for (size_t i = 0; i < height && !stop; i++) {
        for (size_t j = 0; j < width && !stop; j++) {
            PIXEL   pixel     = bmp->pixels[i][j];
            uint8_t colors[3] = {pixel.blue, pixel.green, pixel.red};

            // Extract the LSB nibble from each color component
            for (int k = 0; k < 3 && !stop; k++) {
                uint8_t nibble = colors[k] & 0x0F;
                currentByte |= nibble << (4 - bitIndex);  // Set the nibble in the correct position
                bitIndex += 4;

                // If we have a full byte, store it in the data buffer
                if (bitIndex == 8) {
                    dataBuffer[dataBufferIndex++] = currentByte;
                    currentByte                   = 0;  // Reset for the next byte
                    bitIndex                      = 0;

                    // The first 4 bytes represent the size of the hidden data
                    if (dataBufferIndex == 4) {
                        uint32_t size;
                        memcpy(&size, dataBuffer, 4);
                        size      = ntohl(size);  // Convert from network byte order
                        *dataSize = size;

                        // Ensure the reported size fits within the maximum capacity
                        if (4 + size + 1 > maxDataBytes) {  // +1 for at least the null terminator
                            printerr(
                                "Size mismatch: read size too large to be embedded in this file\n");
                            free(dataBuffer);
                            return NULL;
                        }
                    }
                    // Stop extraction when the data size is reached
                    else if (dataBufferIndex >= 4 + *dataSize) {
                        if (!encrypted && dataBuffer[dataBufferIndex - 1] == '\0') {
                            stop = 1;
                        }
                        else if (encrypted) {
                            stop = 1;
                        }
                    }
                }
            }
        }
    }

    // If the loop ended without extracting all data, report an error
    if (!stop) {
        printerr("End of image data reached before completing extraction\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}
