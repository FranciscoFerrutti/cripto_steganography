#include "extraction.h"

/**
 * @brief Decodes hidden data from a BMP file using the LSB4 (Least Significant Nibble) method.
 *
 * This function extracts data hidden in the least significant nibbles of the BMP image's pixel
 * color components.
 *
 * @param bmp A pointer to a BMP_FILE structure containing the BMP image data.
 * @param dataSize A pointer to a size_t variable where the size of the extracted data will be
 * stored.
 * @param encrypted An integer flag indicating whether the extracted data is encrypted (non-zero
 * value indicates encrypted data).
 * @return A pointer to the buffer containing the extracted data, or NULL if an error occurs.
 */
unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted) {
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

    if (!stop) {
        printerr("End of image data reached before end of hidden data\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}