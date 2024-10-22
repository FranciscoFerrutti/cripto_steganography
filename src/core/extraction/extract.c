#include "extraction.h"

/**
 * @brief Extract hidden data from a BMP file using the specified steganography method
 *
 * @param carrierFile Path to the BMP file to extract data from
 * @param outputFile Path to the output file to store the extracted data
 * @param method Steganography method to use
 * @param a Encryption algorithm to use
 * @param m Encryption mode to use
 * @param pass Password to decrypt the data
 *
 * @note To ensure decryption a password must be provided
 *
 */
void extract(const char *carrierFile,
             const char *outputFile,
             steg        method,
             encryption  a,
             mode        m,
             const char *pass) {
    BMP_FILE *bmp = read_bmp(carrierFile);

    // Ensure BMP file was read correctly
    if (!bmp) {
        printerr("Could not read BMP file: %s\n", carrierFile);
        exit(EXIT_FAILURE);
    }

    // Determine if encryption is being used based on password presence
    int            encrypted     = pass != NULL;
    unsigned char *extractedData = NULL;
    size_t         dataSize      = 0;

    // Steganography extraction based on the selected method
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
            exit(EXIT_FAILURE);
    }

    // Free BMP resources after extraction
    free_bmp(bmp);

    if (!extractedData) {
        printerr("Error extracting data\n");
        exit(EXIT_FAILURE);
    }

    // Process the extracted data with decryption if needed
    if (process_extracted_data(extractedData, outputFile, pass, a, m) != 0) {
        printerr("Error processing extracted data\n");
        free(extractedData);
        exit(EXIT_FAILURE);
    }

    // Free the memory allocated for the extracted data
    free(extractedData);

    // Output extraction details
    char dataSizeStr[20];
    snprintf(dataSizeStr, sizeof(dataSizeStr), "%zu", dataSize);

    print_table("Successfully extracted hidden data from BMP file",
                0xa6da95,
                "Output file",
                outputFile,
                "Steganography method",
                steg_str[method],
                "Size (bytes)",
                dataSizeStr,
                "Encryption Algorithm",
                encryption_str[a],
                "Encryption Mode",
                mode_str[m],
                "Password",
                pass ? pass : "None",
                NULL);
}
