#include "embedding.h"

/**
 * @brief Encode data into a BMP file using the LSBI method
 *
 * @param bmp       BMP file to embed the data into
 * @param data      Pointer to the data to embed
 * @param dataSize  Size of the data in bytes
 *
 * LSBI hides data in two least significant bits of each byte in the image's color channels (RGB).
 */
void lsbi_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize) {
    uint32_t i, j;
    uint8_t  twoBits;
    size_t   dataIndex    = 0;             // Index into the data buffer
    int      bitIndex     = 0;             // Bit index within the current byte (increments by 2)
    size_t   totalTwoBits = dataSize * 4;  // Total two-bit units to embed
    size_t   maxTwoBits =
        bmp->infoHeader.biHeight * bmp->infoHeader.biWidth * 3;  // Max capacity in two-bit units

    // Check if the BMP has enough capacity to hold the data
    if (totalTwoBits > maxTwoBits) {
        printerr("Data size exceeds BMP capacity\n");
        return;
    }

    size_t twoBitsCount = 0;  // Counter for total two-bit units embedded

    // Start embedding
    for (i = 0; i < bmp->infoHeader.biHeight && twoBitsCount < totalTwoBits; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth && twoBitsCount < totalTwoBits; j++) {
            // For each pixel, embed into the RGB channels
            uint8_t *colors[3] = {
                &bmp->pixels[i][j].red, &bmp->pixels[i][j].green, &bmp->pixels[i][j].blue};

            for (int k = 0; k < 3 && twoBitsCount < totalTwoBits; k++) {
                // Get the current two bits to embed from the data buffer
                uint8_t c = data[dataIndex];
                twoBits   = (c >> (6 - bitIndex)) & 0x03;

                // Embed the two bits into the least significant bits of the color channel
                *colors[k] = (*colors[k] & 0xFC) | twoBits;

                // Move to the next two bits
                bitIndex += 2;
                if (bitIndex == 8) {
                    bitIndex = 0;
                    dataIndex++;
                    if (dataIndex >= dataSize) {
                        // All data has been embedded
                        return;
                    }
                }

                twoBitsCount++;
            }
        }
    }

    // Check if all data was embedded
    if (twoBitsCount < totalTwoBits) {
        printerr("Not all data was embedded\n");
    }
}
