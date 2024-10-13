#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "steganography.h"

void lsb1_decode(BMP_FILE *bmp, const char *outputFile);
void lsb4_decode(BMP_FILE *bmp, const char *outputFile);
void lsbi_decode(BMP_FILE *bmp, const char *outputFile);

void extract(const char *carrierFile, const char *outputFile, steg method) {
    BMP_FILE *bmp;

    /* Read the carrier BMP file */
    bmp = read_bmp(carrierFile);
    if (!bmp) {
        fprintf(stderr, "Error: Could not read BMP file %s\n", carrierFile);
        exit(1);
    }

    /* Select the steganography extraction method */
    if (method == LSB1) {
        lsb1_decode(bmp, outputFile);
    }
    else if (method == LSB4) {
        lsb4_decode(bmp, outputFile);
    }
    else if (method == LSBI) {
        lsbi_decode(bmp, outputFile);
    }
    else {
        fprintf(stderr, "Error: Invalid steganography method\n");
        free_bmp(bmp);
        exit(1);
    }

    /* Free the BMP*/
    free_bmp(bmp);
}

/**
 * @brief Decode a message from a BMP file using the LSB1 method
 *
 * @param bmp BMP file structure to extract the message from
 * @param outputFile Path to the output file to write the extracted message
 *
 * Decodes the message from the least significant bit of the red channel of each pixel
 */
void lsb1_decode(BMP_FILE *bmp, const char *outputFile) {
    FILE    *output;        // output file where the message will be written
    int      byte = 0;      // byte to construct from the bits
    uint32_t i, j, k;       // loop counters
    int      bitIndex = 0;  // index to track the current bit being extracted

    /* Open the output file */
    output = fopen(outputFile, "wb");
    if (!output) {
        fprintf(stderr, "Error: Could not open output file %s\n", outputFile);
        return;
    }

    // Extract the message from the BMP file
    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth; j++) {
            for (k = 0; k < 3; k++) {  // Iterate over the RGB channels
                uint8_t lsb = 0;

                // Get the least significant bit from the pixel
                if (k == 0) {
                    lsb = bmp->pixels[i][j].red & 0x01;
                }
                else if (k == 1) {
                    lsb = bmp->pixels[i][j].green & 0x01;
                }
                else if (k == 2) {
                    lsb = bmp->pixels[i][j].blue & 0x01;
                }

                // Construct the byte from the extracted bit
                byte = (byte << 1) | lsb;
                bitIndex++;

                // When a byte is complete, write it to the output file
                if (bitIndex == 8) {
                    fputc(byte, output);
                    byte     = 0;
                    bitIndex = 0;
                }
            }
        }
    }

    fclose(output);
}

void lsb4_decode(BMP_FILE *bmp, const char *outputFile) {
    FILE    *output;           // Output file where the decoded message will be written
    int      byte = 0;         // Byte to construct from the nibbles
    uint32_t i, j, k;          // Loop counters
    int      nibbleIndex = 0;  // Index to track the current nibble being extracted

    /* Open the output file */
    output = fopen(outputFile, "wb");
    if (!output) {
        fprintf(stderr, "Error: Could not open output file %s\n", outputFile);
        return;
    }

    // Extract the message from the BMP file
    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth; j++) {
            for (k = 0; k < 3; k++) {  // Iterate over the RGB channels
                uint8_t nibble = 0;

                // Get the least significant 4 bits (nibble) from the pixel's channel
                if (k == 0) {
                    nibble = bmp->pixels[i][j].red & 0x0F;
                }
                else if (k == 1) {
                    nibble = bmp->pixels[i][j].green & 0x0F;
                }
                else if (k == 2) {
                    nibble = bmp->pixels[i][j].blue & 0x0F;
                }

                // Construct the byte from the extracted nibbles
                if (nibbleIndex == 0) {
                    byte        = (nibble << 4);  // Store the upper nibble
                    nibbleIndex = 1;
                }
                else {
                    byte |= nibble;       // Complete the byte by adding the lower nibble
                    fputc(byte, output);  // Write the byte to the output file
                    byte        = 0;      // Reset byte
                    nibbleIndex = 0;      // Reset nibble index
                }
            }
        }
    }

    fclose(output);
}

void lsbi_decode(BMP_FILE *bmp, const char *outputFile) {
    FILE    *output;
    int      byte = 0;
    uint32_t i, j;
    int      bitIndex = 0;

    /* Open the output file */
    output = fopen(outputFile, "wb");
    if (!output) {
        fprintf(stderr, "Error: Could not open output file %s\n", outputFile);
        return;
    }

    /* Extract the message from the BMP file */
    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth; j++) {
            /* Extract 2 bits from the red channel */
            uint8_t twoBits = bmp->pixels[i][j].red & 0x03;
            byte            = (byte << 2) | twoBits;
            bitIndex += 2;

            if (bitIndex == 8) {
                fputc(byte, output);
                byte     = 0;
                bitIndex = 0;
            }

            /* Extract 2 bits from the green channel */
            twoBits = bmp->pixels[i][j].green & 0x03;
            byte    = (byte << 2) | twoBits;
            bitIndex += 2;

            if (bitIndex == 8) {
                fputc(byte, output);
                byte     = 0;
                bitIndex = 0;
            }

            /* Extract 2 bits from the blue channel */
            twoBits = bmp->pixels[i][j].blue & 0x03;
            byte    = (byte << 2) | twoBits;
            bitIndex += 2;

            if (bitIndex == 8) {
                fputc(byte, output);
                byte     = 0;
                bitIndex = 0;
            }
        }
    }

    /* Close the output file */
    fclose(output);
}
