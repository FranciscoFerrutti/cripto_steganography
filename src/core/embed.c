#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "steganography.h"

void lsb1_encode(BMP_FILE *bmp, const char *messageFile);
void lsb4_encode(BMP_FILE *bmp, const char *messageFile);
void lsbi_encode(BMP_FILE *bmp, const char *messageFile);

/**
 * @brief Embed a message into a BMP file using steganography
 *
 * @param carrierFile Path to the BMP file to embed the message into
 * @param messageFile Path to the file containing the message to embed
 * @param outputFile Path to the output BMP file with the embedded message
 * @param method Steganography method to use
 *
 */
void embed(const char *carrierFile, const char *messageFile, const char *outputFile, steg method) {
    BMP_FILE *bmp;

    /* read carrier into bmp structs*/
    bmp = read_bmp(carrierFile);
    if (!bmp) {
        fprintf(stderr, "Error: Could not read BMP file %s\n", carrierFile);
        exit(1);
    }

    /* Select the steganography method and embed the message */
    if (method == LSB1) {
        lsb1_encode(bmp, messageFile);
    }
    else if (method == LSB4) {
        lsb4_encode(bmp, messageFile);
    }
    else if (method == LSBI) {
        lsbi_encode(bmp, messageFile);
    }
    else {
        fprintf(stderr, "Error: Invalid steganography method\n");
        free_bmp(bmp);
        exit(1);
    }

    /* Write the new bmp to outputfile*/
    if (write_bmp(outputFile, bmp) != 0) {
        fprintf(stderr, "Error: Could not write BMP file %s\n", outputFile);
        free_bmp(bmp);
        exit(1);
    }

    free_bmp(bmp);
}

/**
 * @brief Encode a message into a BMP file using the LSB1 method
 *
 * @param bmp BMP file to embed the message into
 * @param messageFile Path to the file containing the message to embed
 *
 * LSB1 hides data in the least significant bit of each byte in the images color channels (RGB)
 */
void lsb1_encode(BMP_FILE *bmp, const char *messageFile) {
    FILE    *message;  // file pointer to the message file
    int      c;        // character to read from the message file
    uint32_t i, j, k;
    uint8_t  bit;           // bit to embed
    int      bitIndex = 0;  // index of the bit to embed

    // read the message file into the message file pointer
    message = fopen(messageFile, "rb");
    if (!message) {
        fprintf(stderr, "Error: Could not open message file %s\n", messageFile);
        return;
    }

    // read the first char from the message file
    c = fgetc(message);
    if (c == EOF) {
        fprintf(stderr, "Error: Message file is empty\n");
        fclose(message);
        return;
    }

    // we want to embed the message into the bmp file so we will recreate it pixel by pixel but with
    // the message embedded
    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth; j++) {
            for (k = 0; k < 3; k++) {                // iterate over the RGB channels
                bit = (c >> (7 - bitIndex)) & 0x01;  // get the bit to embed from c

                // embed the bit into the pixel
                if (k == 0) {
                    bmp->pixels[i][j].red = (bmp->pixels[i][j].red & 0xFE) | bit;
                }
                else if (k == 1) {
                    bmp->pixels[i][j].green = (bmp->pixels[i][j].green & 0xFE) | bit;
                }
                else if (k == 2) {
                    bmp->pixels[i][j].blue = (bmp->pixels[i][j].blue & 0xFE) | bit;
                }

                bitIndex++;
                // if we have embedded 8 bits we read the next char from the message file
                if (bitIndex == 8) {
                    c = fgetc(message);
                    if (c == EOF) {
                        fclose(message);
                        return;
                    }
                    bitIndex = 0;
                }
            }
        }
    }

    fclose(message);
}

/**
 * @brief Embed a message into a BMP file using the LSB4 method
 *
 * @param bmp BMP file to embed the message into
 * @param messageFile Path to the file containing the message to embed
 *
 * LSB4 hides data in the least significant 4 bits of each byte in the image's color channels (RGB)
 * This allows for bigger messages to be embedded
 */
void lsb4_encode(BMP_FILE *bmp, const char *messageFile) {
    FILE    *message;  // file pointer to the message file
    int      c;        // character to read from the message file
    uint32_t i, j, k;
    uint8_t  nibble;           // nibble (4 bits) to embed
    int      nibbleIndex = 0;  // index to track which nibble (higher or lower) is being embedded

    // Open the message file in binary read mode
    message = fopen(messageFile, "rb");
    if (!message) {
        fprintf(stderr, "Error: Could not open message file %s\n", messageFile);
        return;
    }

    // Read the first character from the message file
    c = fgetc(message);
    if (c == EOF) {
        fprintf(stderr, "Error: Message file is empty\n");
        fclose(message);
        return;
    }

    // Iterate over the BMP pixels
    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth; j++) {
            for (k = 0; k < 3; k++) {                            // Iterate over the RGB channels
                nibble = (c >> (4 * (1 - nibbleIndex))) & 0x0F;  // Get the nibble to embed

                // Embed the nibble into the corresponding color channel
                if (k == 0) {
                    bmp->pixels[i][j].red = (bmp->pixels[i][j].red & 0xF0) | nibble;
                }
                else if (k == 1) {
                    bmp->pixels[i][j].green = (bmp->pixels[i][j].green & 0xF0) | nibble;
                }
                else if (k == 2) {
                    bmp->pixels[i][j].blue = (bmp->pixels[i][j].blue & 0xF0) | nibble;
                }

                nibbleIndex++;
                if (nibbleIndex == 2) {   // If we have embedded 2 nibbles (8 bits)
                    c = fgetc(message);   // Read the next character from the message file
                    if (c == EOF) {       // If we have reached the end of the message
                        fclose(message);  // Close the message file
                        return;           // Return from the function
                    }
                    nibbleIndex = 0;  // Reset the nibble index
                }
            }
        }
    }

    fclose(message);
}

void lsbi_encode(BMP_FILE *bmp, const char *messageFile) {
    FILE    *message;
    int      c;
    uint32_t i, j;
    uint8_t  twoBits;
    int      bitIndex = 0;

    message = fopen(messageFile, "rb");
    if (!message) {
        fprintf(stderr, "Error: Could not open message file %s\n", messageFile);
        return;
    }

    c = fgetc(message);
    if (c == EOF) {
        fprintf(stderr, "Error: Message file is empty\n");
        fclose(message);
        return;
    }

    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        for (j = 0; j < bmp->infoHeader.biWidth; j++) {
            twoBits               = (c >> (6 - bitIndex)) & 0x03;
            bmp->pixels[i][j].red = (bmp->pixels[i][j].red & 0xFC) | twoBits;

            bitIndex += 2;
            if (bitIndex == 8) {
                c = fgetc(message);
                if (c == EOF) {
                    fclose(message);
                    return;
                }
                bitIndex = 0;
            }

            twoBits                 = (c >> (6 - bitIndex)) & 0x03;
            bmp->pixels[i][j].green = (bmp->pixels[i][j].green & 0xFC) | twoBits;

            bitIndex += 2;
            if (bitIndex == 8) {
                c = fgetc(message);
                if (c == EOF) {
                    fclose(message);
                    return;
                }
                bitIndex = 0;
            }

            twoBits                = (c >> (6 - bitIndex)) & 0x03;
            bmp->pixels[i][j].blue = (bmp->pixels[i][j].blue & 0xFC) | twoBits;

            bitIndex += 2;
            if (bitIndex == 8) {
                c = fgetc(message);
                if (c == EOF) {
                    fclose(message);
                    return;
                }
                bitIndex = 0;
            }
        }
    }

    fclose(message);
}
