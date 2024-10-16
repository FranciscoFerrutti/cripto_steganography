#include "extraction.h"

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
    if (process_extracted_data(extractedData, outputfile, pass, a, m) != 0) {
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
