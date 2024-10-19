#include "extraction.h"

/**
 * @brief Decodes hidden data from a BMP file using the LSBI (Least Significant Bit Inversion)
 * method.
 *
 * This function extracts data hidden in the least significant bits of the BMP image's pixel color
 * components. The data is encoded using an inversion map that is hidden in the first 4 color
 * components of the image.
 *
 * @param bmp A pointer to a BMP_FILE structure containing the BMP image data.
 * @param dataSize A pointer to a size_t variable where the size of the extracted data will be
 * stored.
 * @param encrypted An integer flag indicating whether the extracted data is encrypted (non-zero
 * value indicates encrypted data).
 * @return A pointer to the buffer containing the extracted data, or NULL if an error occurs.
 */
unsigned char *lsbi_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted) {
    size_t width  = bmp->infoHeader.biWidth;
    size_t height = bmp->infoHeader.biHeight;

    size_t total_components = width * height * 3;  // Total color components in the image
    size_t max_data_bits    = width * height * 2;  // Only green and blue channels used
    size_t max_data_bytes   = max_data_bits / 8;   // Maximum bytes that can be extracted

    unsigned char *dataBuffer = malloc(max_data_bytes);
    if (!dataBuffer) {
        printerr("Memory allocation failed\n");
        return NULL;
    }

    // Step 1: Read the 4-bit pattern map from the first 4 color components
    uint8_t map_bits  = 0;
    int     bits_read = 0;
    size_t  idx       = 0;  // Index of the current color component
    size_t  i, j, k, temp;

    for (; bits_read < 4 && idx < total_components; idx++) {
        i    = idx / (width * 3);
        temp = idx % (width * 3);
        j    = temp / 3;
        k    = temp % 3;

        if (i >= height || j >= width)
            break;

        PIXEL   pixel     = bmp->pixels[i][j];
        uint8_t colors[3] = {pixel.blue, pixel.green, pixel.red};

        uint8_t bit = colors[k] & 1;
        map_bits |= bit << (3 - bits_read);
        bits_read++;
    }

    if (bits_read < 4) {
        printerr("Failed to read map bits\n");
        free(dataBuffer);
        return NULL;
    }

    // Step 2: Decode the hidden data using the inversion map
    size_t  dataBufferIndex = 0;
    uint8_t currentByte     = 0;
    int     bitIndex        = 0;
    int     stop            = 0;

    for (; idx < total_components && !stop; idx++) {
        // Skip the red channel (k == 2)
        if ((idx % 3) == 2)
            continue;

        i    = idx / (width * 3);
        temp = idx % (width * 3);
        j    = temp / 3;
        k    = temp % 3;

        if (i >= height || j >= width)
            break;

        PIXEL   pixel       = bmp->pixels[i][j];
        uint8_t colors[3]   = {pixel.blue, pixel.green, pixel.red};
        uint8_t color_value = colors[k];

        // Identify the pattern of the 2nd and 3rd LSBs
        uint8_t pattern  = (color_value >> 1) & 0x3;         // grab the 2nd and 3rd LSBs
        uint8_t inverted = (map_bits >> (3 - pattern)) & 1;  // check if the pattern is inverted

        // Extract the LSB, invert if necessary
        uint8_t bit = color_value & 1;
        if (inverted)
            bit ^= 1;

        // Collect bits to form bytes
        currentByte |= bit << (7 - bitIndex);
        bitIndex++;

        if (bitIndex == 8) {
            dataBuffer[dataBufferIndex++] = currentByte;
            currentByte                   = 0;
            bitIndex                      = 0;

            // Check for data size after reading the first 4 bytes
            if (dataBufferIndex == 4) {
                uint32_t size;
                memcpy(&size, dataBuffer, 4);
                size      = ntohl(size);
                *dataSize = size;

                if (4 + size + 1 > max_data_bytes) {
                    printerr("Size mismatch, hidden data is too large for this image\n");
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

    if (!stop) {
        printerr("End of image data reached before end of hidden data\n");
        free(dataBuffer);
        return NULL;
    }

    return dataBuffer;
}
