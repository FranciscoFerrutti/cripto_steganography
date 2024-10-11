#include "include/bmp_utils.h"

// Create a new BMP file
bmp_file* bmp_create(const int width, const int height) {
    bmp_file* bmp = malloc(sizeof(struct bmp_file));
    if (bmp == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Initialize the BMP file to avoid undefined behavior
    memset(bmp, 0, sizeof(struct bmp_file));

    // Initialize the BMP file header
    bmp->header.signature = 0x4D42;  // "BM" in ASCII
    bmp->header.size = sizeof(bmp_header) + sizeof(info_header) + height * width * sizeof(pixel);
    bmp->header.reserved1 = 0;
    bmp->header.reserved2 = 0;
    bmp->header.offset    = sizeof(bmp_header) + sizeof(info_header);

    // Initialize the DIB header
    bmp->info.size            = sizeof(info_header);
    bmp->info.width           = width;
    bmp->info.height          = height;
    bmp->info.planes          = 1;
    bmp->info.bits            = 24;
    bmp->info.compression     = 0;
    bmp->info.imagesize       = height * width * sizeof(pixel);
    bmp->info.xresolution     = 0;
    bmp->info.yresolution     = 0;
    bmp->info.ncolors         = 0;
    bmp->info.importantcolors = 0;

    // Allocate memory for the pixel data
    bmp->data = malloc(height * sizeof(pixel*));
    if (bmp->data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(bmp);
        return NULL;
    }

    for (int i = 0; i < height; i++) {
        bmp->data[i] = malloc(width * sizeof(pixel));
        if (bmp->data[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            for (int j = 0; j < i; j++) {
                free(bmp->data[j]);
            }
            free(bmp->data);
            free(bmp);
            return NULL;
        }
    }

    return bmp;
}

// Load a BMP file
bmp_file* bmp_load(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file\n");
        return NULL;
    }

    bmp_file* bmp = malloc(sizeof(struct bmp_file));
    if (bmp == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    // Read the BMP file header
    fread(&bmp->header, sizeof(bmp_header), 1, file);

    // Read the DIB header
    fread(&bmp->info, sizeof(info_header), 1, file);

    // Allocate memory for the pixel data
    bmp->data = malloc(bmp->info.height * sizeof(pixel*));
    if (bmp->data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(bmp);
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < bmp->info.height; i++) {
        bmp->data[i] = malloc(bmp->info.width * sizeof(pixel));
        if (bmp->data[i] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            for (int j = 0; j < i; j++) {
                free(bmp->data[j]);
            }
            free(bmp->data);
            free(bmp);
            fclose(file);
            return NULL;
        }
    }

    // Read the pixel data
    for (int i = bmp->info.height - 1; i >= 0; i--) {
        fread(bmp->data[i], sizeof(pixel), bmp->info.width, file);
    }

    fclose(file);
    return bmp;
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
