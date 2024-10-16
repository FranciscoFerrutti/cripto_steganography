#include "embedding.h"

/**
 * @brief Encode data into a BMP file using the LSBI method
 *
 * @param bmp       BMP file to embed the data into
 * @param data      Pointer to the data to embed
 * @param dataSize  Size of the data in bytes
 */
void lsbi_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize) {
    uint32_t i, j, k;
    size_t   width            = bmp->infoHeader.biWidth;
    size_t   height           = bmp->infoHeader.biHeight;
    size_t   total_components = width * height * 3;
    size_t   max_data_bits    = width * height * 2;  // Only green and blue channels used
    size_t   max_data_bytes   = max_data_bits / 8;

    // Check if the BMP has enough capacity to hold the data
    if (dataSize + 4 > max_data_bytes) {
        printerr("Data size exceeds BMP capacity\n");
        return;
    }

    // Buffer to store inversion map
    uint8_t map_bits     = 0;
    int     bits_written = 0;

    // Embed the 4-bit pattern map into the first 4 color components using LSB1
    size_t idx = 0;
    for (bits_written = 0; bits_written < 4 && idx < total_components; idx++) {
        i = idx / (width * 3);
        j = (idx % (width * 3)) / 3;
        k = idx % 3;

        if (i >= height || j >= width)
            break;

        PIXEL  *pixel     = &bmp->pixels[i][j];
        uint8_t colors[3] = {pixel->blue, pixel->green, pixel->red};

        uint8_t bit = (map_bits >> (3 - bits_written)) & 1;
        colors[k]   = (colors[k] & 0xFE) | bit;

        pixel->blue  = colors[0];
        pixel->green = colors[1];
        pixel->red   = colors[2];

        bits_written++;
    }

    // Step 2: Embed the data using LSBI and inversion pattern
    size_t dataIndex    = 0;
    int    bitIndex     = 0;
    size_t twoBitsCount = 0;
    int    stop         = 0;

    for (; idx < total_components && !stop; idx++) {
        // Skip the red channel (k == 2)
        if ((idx % 3) == 2)
            continue;

        i = idx / (width * 3);
        j = (idx % (width * 3)) / 3;
        k = idx % 3;

        if (i >= height || j >= width)
            break;

        PIXEL  *pixel       = &bmp->pixels[i][j];
        uint8_t colors[3]   = {pixel->blue, pixel->green, pixel->red};
        uint8_t color_value = colors[k];

        // Get the next bit to embed from the data buffer
        uint8_t pattern  = (color_value >> 1) & 0x3;
        uint8_t inverted = (map_bits >> (3 - pattern)) & 1;

        uint8_t bit = (data[dataIndex] >> (7 - bitIndex)) & 1;
        if (inverted)
            bit ^= 1;

        // Embed the bit into the color channel
        colors[k]    = (colors[k] & 0xFE) | bit;
        pixel->blue  = colors[0];
        pixel->green = colors[1];
        pixel->red   = colors[2];

        bitIndex++;
        if (bitIndex == 8) {
            dataIndex++;
            bitIndex = 0;

            // Stop if we've embedded all the data
            if (dataIndex >= dataSize) {
                stop = 1;
            }
        }

        twoBitsCount++;
    }

    // Check if all data was embedded
    if (twoBitsCount < dataSize * 8) {
        printerr("Not all data was embedded\n");
    }
}
