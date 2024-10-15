#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
#include "encryption.h"
#include "misc.h"
#include "steganography.h"

unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted);
unsigned char *lsb4_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted);
unsigned char *lsbi_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted);

int extract_embedded_data(const unsigned char *dataBuffer,
                          const char          *outputFilePath,
                          const char          *pass,
                          encryption           a,
                          mode                 m);

void extract(const char *carrierFile,
             const char *outputfile,
             steg        method,
             encryption  a,
             mode        m,
             const char *pass) {
    BMP_FILE *bmp;

    /* Read the carrier BMP file */
    bmp = read_bmp(carrierFile);

    if (!bmp) {
        printerr("Could not read BMP file %s\n", carrierFile);
        exit(1);
    }
    int            encrypted     = pass != NULL;
    unsigned char *extractedData = NULL;
    size_t         dataSize      = 0;
    /* Select the steganography extraction method */
    switch (method) {
        case LSB1:
            extractedData = lsb1_decode(bmp, &dataSize, encrypted);
            break;
        case LSB4:
            extractedData = lsb4_decode(bmp, &dataSize, encrypted);
            break;
        case LSBI:
            extractedData = lsbi_decode(bmp, &dataSize, encrypted);
            break;
        default:
            printerr("Invalid steganography method\n");
            free_bmp(bmp);
            exit(1);
    }

    /* Free the BMP as we dont need it anymore*/
    free_bmp(bmp);

    if (!extractedData) {
        printerr("Error extracting data\n");
        exit(1);
    }

    /* At this point, extractedData contains the decrypted extracted data */
    if (extract_embedded_data(extractedData, outputfile, pass, a, m) != 0) {
        printerr("Error processing extracted data\n");
        free(extractedData);
        exit(1);
    }

    /* Free the extracted data buffer */
    free(extractedData);

    /*******************************************************************/

    char dataSizeStr[20];
    snprintf(dataSizeStr, sizeof(dataSizeStr), "%zu", dataSize);
    print_table("Successfully extracted hidden data from BMP file",
                0xa6da95,
                "Output file",
                outputfile,
                "Steganography method",
                steg_str[method],
                "Size (bytes)",
                dataSizeStr,
                "Encryption Algorithm",
                encryption_str[a],
                "Encryption Mode",
                mode_str[m],
                "Password",
                pass,
                NULL);
}

unsigned char *lsb1_decode(BMP_FILE *bmp, size_t *dataSize, int encrypted) {
    size_t width          = bmp->infoHeader.biWidth;
    size_t height         = bmp->infoHeader.biHeight;
    size_t max_data_bytes = (width * height * 3) / 8;  // Each pixel has 3 color components

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
                uint8_t bit = colors[k] & 1;
                currentByte |= bit << (7 - bitIndex);
                bitIndex++;

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

int extract_embedded_data(const unsigned char *dataBuffer,
                          const char          *outputFilePath,
                          const char          *pass,
                          encryption           a,
                          mode                 m) {
    uint32_t             realSize;
    unsigned char       *decryptedData   = NULL;        // Pointer for decrypted data
    const unsigned char *finalDataBuffer = dataBuffer;  // Pointer to handle both cases

    // Read the real size
    memcpy(&realSize, dataBuffer, 4);
    realSize = ntohl(realSize);
    // If a password is provided, decrypt the data
    if (pass != NULL) {
        size_t checkSize = 0;
        decryptedData    = decrypt_data(dataBuffer + 4, realSize, pass, a, m, &checkSize);
        if (!decryptedData || checkSize != realSize) {
            printerr("Error decrypting data\n");
            return -1;
        }

        // Update pointer to use decrypted data
        finalDataBuffer = decryptedData;
        memcpy(&realSize, finalDataBuffer, 4);
        realSize = ntohl(realSize);
    }

    const unsigned char *fileData  = finalDataBuffer + 4;
    const char          *extension = (const char *) (fileData + realSize);

    if (extension[0] != '.') {
        printerr("Invalid file extension\n");
        free(decryptedData);  // Free if allocated
        return -1;
    }

    // Ensure the extension string is null-terminated within the buffer
    size_t max_extension_len = strlen((const char *) finalDataBuffer) - (4 + realSize);
    size_t extension_len     = strnlen(extension, max_extension_len);
    if (extension_len == max_extension_len) {
        printerr("File extension is not null-terminated\n");
        free(decryptedData);  // Free if allocated
        return -1;
    }

    // Construct the full output file path
    size_t fullPathLen        = strlen(outputFilePath) + extension_len + 1;  // +1 for '\0'
    char  *fullOutputFilePath = malloc(fullPathLen);
    if (!fullOutputFilePath) {
        printerr("Memory allocation failed\n");
        free(decryptedData);  // Free if allocated
        return -1;
    }
    strcpy(fullOutputFilePath, outputFilePath);
    strncat(fullOutputFilePath, extension, extension_len);
    fullOutputFilePath[fullPathLen - 1] = '\0';  // Ensure null-termination

    // Write the file data to the output file
    FILE *outFile = fopen(fullOutputFilePath, "wb");
    if (!outFile) {
        printerr("Failed to open output file %s\n", fullOutputFilePath);
        free(fullOutputFilePath);
        free(decryptedData);  // Free if allocated
        return -1;
    }

    size_t bytesWritten = fwrite(fileData, 1, realSize, outFile);
    if (bytesWritten != realSize) {
        printerr("Failed to write all data to output file\n");
        fclose(outFile);
        free(fullOutputFilePath);
        free(decryptedData);  // Free if allocated
        return -1;
    }

    fclose(outFile);
    free(fullOutputFilePath);
    free(decryptedData);  // Free if allocated

    return 0;
}