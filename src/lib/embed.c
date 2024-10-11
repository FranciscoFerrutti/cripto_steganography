#include "include/bmp_utils.h"
#include "include/encryption.h"
#include "include/parse_args.h"
#include "include/steg.h"

void lsb1_embed(FILE *in, FILE *out, bmp_file *bmp);
void lsb4_embed(FILE *in, FILE *out, bmp_file *bmp);
void lsbi_embed(FILE *in, FILE *out, bmp_file *bmp);

void embed(args args, bmp_file *bmp) {
    if (args.pass == NULL) {
        fprintf(stderr,
                "Error: Cannot embed without encryption, and without password cannot encrypt\n");
        exit(1);
    }

    // Open input file
    FILE *in = fopen(args.in, "rb");
    if (in == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", args.in);
        return;
    }

    // Open or create output file
    FILE *out = fopen(args.out, "wb");
    if (out == NULL) {
        fprintf(stderr, "Error: Could not create or open file %s\n", args.out);
        fclose(in);
        return;
    }

    // Open or create carrier BMP file
    FILE *carrier = fopen(args.p, "rb");
    if (carrier == NULL) {
        // If the carrier file does not exist, create it
        carrier = fopen(args.p, "wb");
        if (carrier == NULL) {
            fprintf(stderr, "Error: Could not create or open BMP file %s\n", args.p);
            fclose(in);
            fclose(out);
            return;
        }
        // Initialize the BMP file with default values
        init_bmp(bmp);
        write_bmp(carrier, bmp);
        fclose(carrier);
        // Reopen the carrier file for reading
        carrier = fopen(args.p, "rb");
        if (carrier == NULL) {
            fprintf(stderr, "Error: Could not reopen BMP file %s\n", args.p);
            fclose(in);
            fclose(out);
            return;
        }
    }

    // Read BMP file
    if (!read_bmp(carrier, bmp)) {
        fprintf(stderr, "Error: Could not read BMP file %s\n", args.p);
        fclose(in);
        fclose(out);
        fclose(carrier);
        return;
    }
    fclose(carrier);

    // Apply encryption if required
    if (args.pass != NULL) {
        uint8_t *encrypted_data;
        size_t   file_size = get_file_size(in);
        encrypted_data     = malloc(file_size);

        if (args.a == NULL) {
            args.a = AES128;  // defaulting to aes128
        }
        if (args.m == NULL) {
            args.m = CBC;  // defaulting to cbc
        }

        encrypt(in, encrypted_data, file_size, args.pass, args.a, args.m);

        // Reset the file pointer to start embedding
        in = fmemopen(encrypted_data, file_size, "rb");
    }

    // Select the steganography method
    switch (args.steg) {
        case LSB1:
            lsb1_embed(in, out, bmp);
            break;
        case LSB4:
            lsb4_embed(in, out, bmp);
            break;
        case LSBI:
            lsbi_embed(in, out, bmp);
            break;
        default:
            fprintf(stderr, "Error: Invalid steganography method\n");
            break;
    }

    // Close files
    fclose(in);
    fclose(out);
}

void lsb1_embed(FILE *in, FILE *out, bmp_file *bmp) {
    // Embed the input file into the BMP file
    int c;
    for (int i = 0; i < bmp->info.height; i++) {
        for (int j = 0; j < bmp->info.width; j++) {
            // Read the pixel data
            pixel *px = &bmp->data[i][j];

            // Read the input file data
            c = fgetc(in);

            // Check if the input file has ended
            if (c == EOF) {
                // Write the pixel data to the output file
                fwrite(px, sizeof(pixel), 1, out);
                continue;
            }

            // Embed the input file data into the pixel data
            px->red   = (px->red & 0xFE) | ((c >> 7) & 0x01);
            px->green = (px->green & 0xFE) | ((c >> 6) & 0x01);
            px->blue  = (px->blue & 0xFE) | ((c >> 5) & 0x01);
        }
    }
}

void lsb4_embed(FILE *in, FILE *out, bmp_file *bmp) {
    // Embed the input file into the BMP file
    int c;
    for (int i = 0; i < bmp->info.height; i++) {
        for (int j = 0; j < bmp->info.width; j++) {
            // Read the pixel data
            pixel *px = &bmp->data[i][j];

            // Read the input file data
            c = fgetc(in);

            // Check if the input file has ended
            if (c == EOF) {
                // Write the pixel data to the output file
                fwrite(px, sizeof(pixel), 1, out);
                continue;
            }

            // Embed the input file data into the pixel data
            px->red   = (px->red & 0xF0) | ((c >> 4) & 0x0F);
            px->green = (px->green & 0xF0) | ((c >> 0) & 0x0F);
        }
    }
}

void lsbi_embed(FILE *in, FILE *out, bmp_file *bmp) {
    // Embed the input file into the BMP file
    int c;
    for (int i = 0; i < bmp->info.height; i++) {
        for (int j = 0; j < bmp->info.width; j++) {
            // Read the pixel data
            pixel *px = &bmp->data[i][j];

            // Read the input file data
            c = fgetc(in);

            // Check if the input file has ended
            if (c == EOF) {
                // Write the pixel data to the output file
                fwrite(px, sizeof(pixel), 1, out);
                continue;
            }

            // Embed the input file data into the pixel data
            px->red   = (px->red & 0xFC) | ((c >> 6) & 0x03);
            px->green = (px->green & 0xFC) | ((c >> 4) & 0x03);
            px->blue  = (px->blue & 0xFC) | ((c >> 2) & 0x03);
        }
    }
}
