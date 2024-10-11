#include "include/bmp_utils.h"

int init_bmp(bmp_file* bmp, int width, int height) {
    bmp->header.signature = 0x4D42;  // BM
    bmp->header.size      = 0;
    bmp->header.reserved1 = 0;
    bmp->header.reserved2 = 0;
    bmp->header.offset    = 54;

    bmp->info.size            = 40;
    bmp->info.width           = width;
    bmp->info.height          = height;
    bmp->info.planes          = 1;
    bmp->info.bits            = 24;
    bmp->info.compression     = 0;
    bmp->info.imagesize       = width * height * 3;
    bmp->info.xresolution     = 2835;
    bmp->info.yresolution     = 2835;
    bmp->info.ncolors         = 0;
    bmp->info.importantcolors = 0;

    bmp->data = malloc(height * sizeof(pixel*));
    if (bmp->data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }

    for (int i = 0; i < height; i++) {
        bmp->data[i] = malloc(width * sizeof(pixel));
        if (bmp->data[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 0;
        }
    }

    return 1;
}

int write_bmp(FILE* file, bmp_file* bmp) {
    // Write the BMP file header
    fwrite(&bmp->header.signature, 2, 1, file);
    fwrite(&bmp->header.size, 4, 1, file);
    fwrite(&bmp->header.reserved1, 2, 1, file);
    fwrite(&bmp->header.reserved2, 2, 1, file);
    fwrite(&bmp->header.offset, 4, 1, file);

    // Write the DIB header
    fwrite(&bmp->info.size, 4, 1, file);
    fwrite(&bmp->info.width, 4, 1, file);
    fwrite(&bmp->info.height, 4, 1, file);
    fwrite(&bmp->info.planes, 2, 1, file);
    fwrite(&bmp->info.bits, 2, 1, file);
    fwrite(&bmp->info.compression, 4, 1, file);
    fwrite(&bmp->info.imagesize, 4, 1, file);
    fwrite(&bmp->info.xresolution, 4, 1, file);
    fwrite(&bmp->info.yresolution, 4, 1, file);
    fwrite(&bmp->info.ncolors, 4, 1, file);
    fwrite(&bmp->info.importantcolors, 4, 1, file);

    // Write the pixel data
    for (int i = 0; i < bmp->info.height; i++) {
        for (int j = 0; j < bmp->info.width; j++) {
            fwrite(&bmp->data[i][j], sizeof(pixel), 1, file);
        }
    }

    return 1;
}

int read_bmp(FILE* file, bmp_file* bmp) {
    // Read the BMP file header
    fread(&bmp->header.signature, 2, 1, file);
    fread(&bmp->header.size, 4, 1, file);
    fread(&bmp->header.reserved1, 2, 1, file);
    fread(&bmp->header.reserved2, 2, 1, file);
    fread(&bmp->header.offset, 4, 1, file);

    // Read the DIB header
    fread(&bmp->info.size, 4, 1, file);
    fread(&bmp->info.width, 4, 1, file);
    fread(&bmp->info.height, 4, 1, file);
    fread(&bmp->info.planes, 2, 1, file);
    fread(&bmp->info.bits, 2, 1, file);
    fread(&bmp->info.compression, 4, 1, file);
    fread(&bmp->info.imagesize, 4, 1, file);
    fread(&bmp->info.xresolution, 4, 1, file);
    fread(&bmp->info.yresolution, 4, 1, file);
    fread(&bmp->info.ncolors, 4, 1, file);
    fread(&bmp->info.importantcolors, 4, 1, file);

    // Read the pixel data
    bmp->data = malloc(bmp->info.height * sizeof(pixel*));
    if (bmp->data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }

    for (int i = 0; i < bmp->info.height; i++) {
        bmp->data[i] = malloc(bmp->info.width * sizeof(pixel));
        if (bmp->data[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 0;
        }
    }

    for (int i = 0; i < bmp->info.height; i++) {
        for (int j = 0; j < bmp->info.width; j++) {
            fread(&bmp->data[i][j], sizeof(pixel), 1, file);
        }
    }

    return 1;
}

/**
 * Returns the size of a file
 * @param file The file to get the size of
 * @return The size of the file
 */
size_t get_file_size(FILE* file) {
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}
