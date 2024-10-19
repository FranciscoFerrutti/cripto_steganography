#include "embedding.h"

/**
 * @brief Encode data into a BMP file using the LSB1 method
 *
 * @param bmp       BMP file to embed the data into
 * @param data      Pointer to the data to embed
 * @param dataSize  Size of the data in bytes
 *
 * LSB1 hides data in the least significant bit of each byte in the image's color channels (RGB)
 */
void lsb1_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize) {
    size_t totalBits = dataSize * 8;  // Total bits to embed
    // there are 3 effective bits per pixel (1 per channel)
    size_t maxBits = bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3;

    if (totalBits > maxBits) {
        printerr(
            "Data size exceeds the maximum embedding capacity. You are trying to embed "
            "%zu bytes, but the maximum capacity is %zu bytes.\n",
            dataSize,
            maxBits / 8);
        return;
    }

    size_t   dataIndex = 0;  // Index into the data array
    int      bitIndex  = 7;  // Bit index within the current byte (start from MSB)
    size_t   bitCount  = 0;  // Counter for total bits embedded
    uint32_t i, j;           // Loop counters

    // Embed the message into the BMP file pixel by pixel, color channel by color channel
    for (i = 0; i < bmp->infoHeader.biHeight && bitCount < totalBits; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && bitCount < totalBits; j++) {
            // get the color channels of the current pixel
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].blue, &bmp->pixels[i][j].green, &bmp->pixels[i][j].red};

            for (int k = 0; k < 3 && bitCount < totalBits; k++) {
                // Get the current bit to embed from the data buffer
                uint8_t bit = (data[dataIndex] >> bitIndex) & 0x01;

                // Embed the bit into the least significant bit of the color channel
                *colors[k] = (*colors[k] & 0xFE) | bit;

                if (bitIndex == 0) {
                    bitIndex = 7;
                    dataIndex++;
                }
                else {
                    bitIndex--;
                }

                bitCount++;
            }
        }
    }

    // Check if all data was embedded
    if (bitCount < totalBits) {
        printerr("Not all data was embedded\n");
    }
}
