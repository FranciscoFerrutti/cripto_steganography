#ifndef BMP_ADT_H
#define BMP_ADT_H

#include "common_libs.h"

#pragma pack(push, 1)  // avoids padding bytes in structs in memory

/**
 * @cite https://paulbourke.net/dataformats/bmp/
 */

#define BF_TYPE 0x4D42

typedef struct /**** BMP file header structure ****/
{
    uint16_t bfType;      /* Magic identifier: "BM" (fast way to check if is .bmp)*/
    uint32_t bfSize;      /* File size in bytes */
    uint16_t bfReserved1; /* Reserved */
    uint16_t bfReserved2; /* ... */
    uint32_t bfOffBits;   /* Offset to image data in bytes */
} BITMAPFILEHEADER;

typedef struct /**** BMP file info structure ****/
{
    uint32_t biSize;          /* Size of info header */
    uint32_t biWidth;         /* Width of the image */
    uint32_t biHeight;        /* Height of the image */
    uint16_t biPlanes;        /* Number of color planes */
    uint16_t biBitCount;      /* Bits per pixel */
    uint32_t biCompression;   /* Compression type */
    uint32_t biSizeImage;     /* Image size in bytes */
    uint32_t biXPelsPerMeter; /* Pixels per meter in X */
    uint32_t biYPelsPerMeter; /* Pixels per meter in Y */
    uint32_t biClrUsed;       /* Number of colors */
    uint32_t biClrImportant;  /* Important colors */
} BITMAPINFOHEADER;

typedef struct /**** Pixel structure ****/
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} PIXEL;

typedef struct /**** BMP file ****/
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    PIXEL          **pixels;
} BMP_FILE;

/* Function prototypes */
BMP_FILE *read_bmp(const char *filename);
int       write_bmp(const char *filename, BMP_FILE *bmp);
void      free_bmp(BMP_FILE *bmp);

#pragma pack(pop)

#endif
