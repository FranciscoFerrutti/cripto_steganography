#include "embedding.h"

/**
 * @brief Embed a message into a BMP file using the LSB4 steganography method
 *
 * @param bmp BMP file structure to embed the message into
 * @param data Data to embed
 * @param dataSize Size of the data to embed
 *
 */
void lsb4_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize) {
    size_t totalNibbles = dataSize * 2;  // total bytes * 2 nibbles per byte to embed
    // there are 3 effective nibbles per pixel (1 per channel) so 12 bits per pixel
    size_t maxBits   = bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3 * 4;
    size_t totalBits = dataSize * 8;  // Total bits to embed

    // Check if the BMP has enough capacity to hold the data
    if (totalBits > maxBits) {
        printerr(
            "Data size exceeds the maximum embedding capacity. You are trying to embed %zu bytes, "
            "but the maximum capacity is %zu bytes.\n",
            dataSize,
            maxBits / 8);
        return;
    }

    size_t dataIndex   = 0;  // Index into the data array
    int    nibbleIndex = 1;  // nibble index for curr byte
    size_t nibbleCount = 0;  // Counter for total nibbles embedded

    // Iterate over each pixel in the BMP image
    for (uint32_t i = 0; i < bmp->infoHeader.biHeight && nibbleCount < totalNibbles; i++) {
        for (uint32_t j = 0; j < bmp->infoHeader.biWidth && nibbleCount < totalNibbles; j++) {
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].blue, &bmp->pixels[i][j].green, &bmp->pixels[i][j].red};

            // Similar if not same as lsb1
            for (int k = 0; k < 3 && nibbleCount < totalNibbles; k++) {
                // Get the current nibble to embed from the data buffer
                uint8_t nibble = (data[dataIndex] >> (nibbleIndex * 4)) & 0x0F;

                // Embed the nibble into the least significant 4 bits of the color channel
                *colors[k] = (*colors[k] & 0xF0) | nibble;

                // Move to the next nibble
                if (nibbleIndex == 0) {
                    nibbleIndex = 1;
                    dataIndex++;
                }
                else {
                    nibbleIndex = 0;
                }

                nibbleCount++;
            }
        }
    }

    // Check if all data was embedded
    if (nibbleCount < totalNibbles) {
        printerr("Not all data was embedded\n");
    }
}
