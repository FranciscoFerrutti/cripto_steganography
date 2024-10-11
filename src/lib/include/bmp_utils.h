#ifndef BMP_ADT_H
#define BMP_ADT_H

#include "common_libs.h"

// Struct for storing the BMP file header
typedef struct bmp_header
{
    uint16_t signature;  // File signature
    uint32_t size;       // File size in bytes
    uint16_t reserved1;  // Not used
    uint16_t reserved2;  // Not used
    uint32_t offset;     // Offset to image data in bytes from beginning of file
} bmp_header;            // total of 14 bytes

// Struct for storing the DIB header
typedef struct info_header
{
    uint32_t size;             // DIB Header size in bytes
    int32_t  width;            // Width of the image
    int32_t  height;           // Height of image
    uint16_t planes;           // Number of color planes
    uint16_t bits;             // Bits per pixel
    uint32_t compression;      // Compression type
    uint32_t imagesize;        // Image size in bytes
    int32_t  xresolution;      // Pixels per meter
    int32_t  yresolution;      // Pixels per meter
    uint32_t ncolors;          // Number of colors in the color palette
    uint32_t importantcolors;  // Important colors
} info_header;                 // total of 40 bytes

// We assume that the BMP file is 24 bits per pixel
// Struct for storing the pixel data
typedef struct pixel
{
    uint8_t blue;   // Blue color component
    uint8_t green;  // Green color component
    uint8_t red;    // Red color component
} pixel;            // total of 3 bytes

// Struct for storing the BMP file
typedef struct bmp_file
{
    bmp_header  header;  // BMP file header
    info_header info;    // DIB header
    pixel**     data;    // 2D array of pixel data
} bmp_file;

// i need functions init_bmp, write_bmp and read_bmp
int init_bmp(bmp_file* bmp, int width, int height);
int write_bmp(FILE* file, bmp_file* bmp);
int read_bmp(FILE* file, bmp_file* bmp);

size_t get_file_size(FILE* file);

#endif