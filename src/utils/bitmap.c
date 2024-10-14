#include "bitmap.h"

#include <stdio.h>
#include <stdlib.h>

#include "misc.h"
/**
 * @brief Read a BMP file and store it in a BMP_FILE structure
 *
 * @param filename Path to the BMP file
 *
 * @return BMP_FILE structure containing the BMP file data
 */
BMP_FILE *read_bmp(const char *filename) {
    FILE     *filePtr;  // File pointer
    BMP_FILE *bmp;      // BMP file structure where the data will be stored
    uint32_t  i;        // Loop counter

    // Allocate memory for BMP_FILE structure
    bmp = (BMP_FILE *) malloc(sizeof(BMP_FILE));
    if (!bmp) {
        printerr("Memory allocation for BMP_FILE failed\n");
        return NULL;
    }

    // Open the file in binary mode
    filePtr = fopen(filename, "rb");
    if (filePtr == NULL) {
        printerr("Opening BMP file\n");
        free(bmp);
        return NULL;
    }

    // Read the bitmap file header
    if (fread(&bmp->fileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1) {
        printerr("Reading BMP file header.\n");
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    // Verify that this is a BMP file by checking the magic number
    if (bmp->fileHeader.bfType != BF_TYPE) {
        printerr("Not a valid BMP file, magic number mismatch.\n");
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    // Read the bitmap info header (DIB header)
    if (fread(&bmp->infoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1) {
        printerr("Reading BMP info header.\n");
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    // Check if the BMP file is 24-bit (RGB format)
    if (bmp->infoHeader.biBitCount != 24) {
        printerr("Unsupported BMP format: only 24-bit BMP files are supported.\n");
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    if (bmp->infoHeader.biCompression != 0) {
        printerr("BMP file is compressed, only uncompressed BMP files are supported.\n");
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    // Allocate memory for a column of pixels
    bmp->pixels = (PIXEL **) malloc(bmp->infoHeader.biHeight * sizeof(PIXEL *));
    if (!bmp->pixels) {
        printerr("Memory allocation for pixel rows failed\n");
        fclose(filePtr);
        free(bmp);
        return NULL;
    }

    // Allocate memory for each pixel row
    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        bmp->pixels[i] = (PIXEL *) malloc(bmp->infoHeader.biWidth * sizeof(PIXEL));
        if (!bmp->pixels[i]) {
            printerr("Memory allocation for pixel row %d failed\n", i);
            for (uint32_t k = 0; k < i; k++)
                free(bmp->pixels[k]);
            free(bmp->pixels);
            fclose(filePtr);
            free(bmp);
            return NULL;
        }
    }

    // Move the file pointer to the start of the bitmap data
    fseek(filePtr, bmp->fileHeader.bfOffBits, SEEK_SET);

    // Read the pixel data from top to bottom
    for (i = 0; i < bmp->infoHeader.biHeight; i++) {
        if (fread(bmp->pixels[i], sizeof(PIXEL), bmp->infoHeader.biWidth, filePtr) !=
            bmp->infoHeader.biWidth) {
            printerr("Reading pixel data.\n");
            for (uint32_t k = 0; k < i; k++)
                free(bmp->pixels[k]);
            free(bmp->pixels);
            fclose(filePtr);
            free(bmp);
            return NULL;
        }
        // Skip padding bytes
        // The number of bytes in a row must be a multiple of 4
        // width *3 = number of bytes in a row
        // number of bytes in row % 4 = remaining bytes
        // 4 - remaining bytes = padding bytes
        // padding bytes % 4 ensures that the padding bytes are not greater than 4
        // example: width = 10 pixels = 30 bytes = 30 % 4 = 2 remaining bytes = 4 - 2 = 2 padding
        // bytes padding bytes = 2 % 4 = 2 and fseek will skip 2 bytes
        fseek(filePtr, (4 - (bmp->infoHeader.biWidth * 3) % 4) % 4, SEEK_CUR);
    }

    fclose(filePtr);
    return bmp;
}

/**
 * @brief Write a BMP file from a BMP_FILE structure
 *
 * @param filename Path to the output BMP file
 * @param bmp BMP file structure to write to the file
 *
 * @return 0 on success, -1 on failure
 */
int write_bmp(const char *filename, BMP_FILE *bmp) {
    FILE *filePtr = fopen(filename, "wb");
    if (filePtr == NULL) {
        printerr("Opening BMP file\n");
        return -1;
    }

    // Write the file header
    if (fwrite(&bmp->fileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr) != 1) {
        printerr("Writing BMP file header\n");
        fclose(filePtr);
        return -1;
    }

    // Write the info header
    if (fwrite(&bmp->infoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr) != 1) {
        printerr("Writing BMP info header\n");
        fclose(filePtr);
        return -1;
    }

    /* Calculate padding per row (BMP rows must be a multiple of 4 bytes) */
    uint32_t paddingSize = (4 - (bmp->infoHeader.biWidth * 3) % 4) % 4;
    uint8_t  padding[3]  = {0, 0, 0};  // Padding bytes, up to 3 bytes of padding

    /* Write the pixel data from top to bottom */
    for (uint32_t i = 0; i < bmp->infoHeader.biHeight; i++) {
        /* Write pixel data for the current row, but reverse the index */
        if (fwrite(bmp->pixels[i], sizeof(PIXEL), bmp->infoHeader.biWidth, filePtr) !=
            bmp->infoHeader.biWidth) {
            printerr("Writing pixel data for row %d\n", i);
            fclose(filePtr);
            return -1;
        }

        /* Write padding bytes, if any */
        if (paddingSize > 0) {
            if (fwrite(padding, 1, paddingSize, filePtr) != paddingSize) {
                printerr("Writing padding for row %d\n", i);
                fclose(filePtr);
                return -1;
            }
        }
    }

    fclose(filePtr);
    return 0;
}
/* Free the BMP */
void free_bmp(BMP_FILE *bmp) {
    for (uint32_t i = 0; i < bmp->infoHeader.biHeight; i++) {
        free(bmp->pixels[i]);
    }
    free(bmp->pixels);
    free(bmp);
}
