#ifndef BMP_ADT_H
#define BMP_ADT_H

#include "common_libs.h"

// Struct for storing the BMP file header
typedef struct bmp_header
{
    __uint16_t signature;  // File signature
    __uint32_t size;       // File size in bytes
    __uint16_t reserved1;  // Not used
    __uint16_t reserved2;  // Not used
    __uint32_t offset;     // Offset to image data in bytes from beginning of file
} bmp_header;              // total of 14 bytes

// Struct for storing the DIB header
typedef struct info_header
{
    __uint32_t size;             // DIB Header size in bytes
    __int32_t  width;            // Width of the image
    __int32_t  height;           // Height of image
    __uint16_t planes;           // Number of color planes
    __uint16_t bits;             // Bits per pixel
    __uint32_t compression;      // Compression type
    __uint32_t imagesize;        // Image size in bytes
    __int32_t  xresolution;      // Pixels per meter
    __int32_t  yresolution;      // Pixels per meter
    __uint32_t ncolors;          // Number of colors in the color palette
    __uint32_t importantcolors;  // Important colors
} info_header;                   // total of 40 bytes

// We assume that the BMP file is 24 bits per pixel
// Struct for storing the pixel data
typedef struct pixel
{
    __uint8_t blue;   // Blue color component
    __uint8_t green;  // Green color component
    __uint8_t red;    // Red color component
} pixel;              // total of 3 bytes

// Struct for storing the BMP file
typedef struct bmp_file
{
    bmp_header  header;  // BMP file header
    info_header info;    // DIB header
    pixel**     data;    // 2D array of pixel data
} bmp_file;

#endif