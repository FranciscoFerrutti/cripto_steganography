#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "misc.h"
#include "steganography.h"

void lsb1_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize);
void lsb4_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize);
void lsbi_encode(BMP_FILE *bmp, const unsigned char *data, size_t dataSize);

unsigned char *prepare_embedding_data(
    const char *messageFile, size_t *totalDataSize, const char *pass, encryption a, mode m);
/**
 * @brief Embed a message into a BMP file using steganography
 *
 * @param carrierFile Path to the BMP file to embed the message into
 * @param messageFile Path to the file containing the message to embed
 * @param outputFile Path to the output BMP file with the embedded message
 * @param method Steganography method to use
 * @param a Encryption algorithm to use
 * @param m Encryption mode to use
 * @param pass Password to use for encryption
 *
 * Embeds a message into a BMP file using steganography and encryption methods
 */
void embed(const char *carrierFile,
           const char *messageFile,
           const char *outputFile,
           steg        method,
           encryption  a,
           mode        m,
           const char *pass) {
    BMP_FILE *bmp = read_bmp(carrierFile);
    if (!bmp) {
        printerr("Could not read BMP file %s\n", carrierFile);
        exit(1);
    }

    /* dataSize | (embeddigData[data] | embeddingData[extension]) */
    size_t         dataSize;
    unsigned char *embeddingData = prepare_embedding_data(messageFile, &dataSize, pass, a, m);

    /* Select the steganography method and embed the message into bmp*/
    switch (method) {
        case LSB1:
            lsb1_encode(bmp, embeddingData, dataSize);
            break;
        case LSB4:
            lsb4_encode(bmp, embeddingData, dataSize);
            break;
        case LSBI:
            lsbi_encode(bmp, embeddingData, dataSize);
            break;
        default:
            printerr("Invalid steganography method\n");
            free_bmp(bmp);
            free(embeddingData);
            exit(1);
    }

    /* Write the new bmp to outputfile*/
    if (write_bmp(outputFile, bmp) != 0) {
        printerr("Could not write BMP file %s\n", outputFile);
        free_bmp(bmp);
        free(embeddingData);
        exit(1);
    }

    free_bmp(bmp);
    free(embeddingData);

    /*******************************************************************/
    char dataSizeStr[20];
    snprintf(dataSizeStr, sizeof(dataSizeStr), "%zu", dataSize);
    print_table("Successfully embedded data into BMP file",
                0xa6da95,
                "Output file",
                outputFile,
                "Stego Method",
                steg_str[method],
                "Size (bytes)",
                dataSizeStr,
                "Encryption Algorithm",
                encryption_str[a],
                "Enctryption Mode",
                mode_str[m],
                "Password",
                pass,
                NULL);
}

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

/**
 * @brief Encode data into a BMP file using the LSB4 method
 *
 * @param bmp       BMP file to embed the data into
 * @param data      Pointer to the data to embed
 * @param dataSize  Size of the data in bytes
 *
 * LSB4 hides data in the least significant 4 bits of each byte in the image's color channels (RGB)
 * This allows for larger messages to be embedded.
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

unsigned char *prepare_embedding_data(
    const char *messageFile, size_t *totalDataSize, const char *pass, encryption a, mode m) {
    FILE *file = fopen(messageFile, "rb");
    if (!file) {
        printerr("Could not open message file %s\n", messageFile);
        return NULL;
    }

    // Determine the file size and go to the beginning of the file
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Read the file data
    unsigned char *fileData = malloc(fileSize);
    if (!fileData) {
        printerr("Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    fread(fileData, 1, fileSize, file);
    fclose(file);

    // Get the file extension
    const char *extension = strrchr(messageFile, '.');
    if (!extension) {
        extension = ".txt";  // Default extension if none is found
    }

    size_t extensionLength = strlen(extension) + 1;  // Include the null terminator

    // totalDataSize = size of the file size (4 bytes) + file size + extension size
    size_t         embeddingDataSize = sizeof(uint32_t) + fileSize + extensionLength;
    unsigned char *embeddingData     = malloc(embeddingDataSize);
    if (!embeddingData) {
        printerr("Memory allocation failed\n");
        free(fileData);
        return NULL;
    }

    uint32_t fileSize32 = htonl((uint32_t) fileSize);
    memcpy(embeddingData, &fileSize32, sizeof(uint32_t));
    memcpy(embeddingData + sizeof(uint32_t), fileData, fileSize);
    memcpy(embeddingData + sizeof(uint32_t) + fileSize, extension, extensionLength);

    free(fileData);

    // Encrypt the embedding data if encryption is enabled
    if (a != ENC_NONE) {
        // Tamaño cifrado || encripcion(tamaño real || datos archivo || extensión)
        size_t         encrypted_len;
        unsigned char *encrypted_data =
            encrypt_data(embeddingData, embeddingDataSize, pass, a, m, &encrypted_len);
        if (!encrypted_data) {
            printerr("Error encrypting data\n");
            free(embeddingData);
            return NULL;
        }

        free(embeddingData);

        // Prepare the final data to embed: encrypted data size (uint32_t) + encrypted data
        *totalDataSize           = sizeof(uint32_t) + encrypted_len;
        unsigned char *finalData = malloc(*totalDataSize);
        if (!finalData) {
            printerr("Memory allocation failed\n");
            free(encrypted_data);
            return NULL;
        }

        uint32_t encrypted_len32 = htonl((uint32_t) encrypted_len);

        memcpy(finalData, &encrypted_len32, sizeof(uint32_t));
        memcpy(finalData + sizeof(uint32_t), encrypted_data, encrypted_len);

        free(encrypted_data);

        return finalData;
    }
    else {
        // No encryption: prepare the final data to embed: data size (uint32_t) + data
        *totalDataSize = embeddingDataSize;

        return embeddingData;
    }
}
