#include "embedding.h"

/**
 * @brief Embed a message into a BMP file using the specified steganography method
 *
 * @param carrierFile Path to the BMP file to embed the message into
 * @param messageFile Path to the file containing the message to embed
 * @param outputFile Path to the output BMP file
 * @param method Steganography method to use
 * @param a Encryption algorithm to use
 * @param m Encryption mode to use
 *
 * @note To ensure encryption a password must be provided
 */
void embed(const char *carrierFile,
           const char *messageFile,
           const char *outputFile,
           steg        method,
           encryption  a,
           mode        m,
           const char *pass) {
    BMP_FILE *bmp = read_bmp(carrierFile);
    if (!bmp) {
        printerr("Could not read BMP file %s\n", carrierFile);
        exit(1);
    }

    /* dataSize | (embeddigData[data] | embeddingData[extension]) */
    size_t         dataSize;
    unsigned char *embeddingData = prepare_embedding_data(messageFile, &dataSize, pass, a, m);

    int result = 0;
    /* Select the steganography method and embed the message into bmp*/
    switch (method) {
        case LSB1:
            result = lsb1_encode(bmp, embeddingData, dataSize);
            break;
        case LSB4:
            result = lsb4_encode(bmp, embeddingData, dataSize);
            break;
        case LSBI:
            result = lsbi_encode(bmp, embeddingData, dataSize);
            break;
        default:
            printerr("Invalid steganography method\n");
            free_bmp(bmp);
            free(embeddingData);
            exit(1);
    }

    if (result == -1) {
        printerr("Error embedding data\n");
        free_bmp(bmp);
        free(embeddingData);
        exit(1);
    }
    /* Write the new bmp to outputfile*/
    if (write_bmp(outputFile, bmp) != 0) {
        printerr("Could not write BMP file %s\n", outputFile);
        free_bmp(bmp);
        free(embeddingData);
        exit(1);
    }

    free_bmp(bmp);
    free(embeddingData);

    /*******************************************************************/
    char dataSizeStr[20];
    snprintf(dataSizeStr, sizeof(dataSizeStr), "%zu", dataSize);
    print_table("Successfully embedded data into BMP file",
                0xa6da95,
                "Output file",
                outputFile,
                "Stego Method",
                steg_str[method],
                "Size (bytes)",
                dataSizeStr,
                "Encryption Algorithm",
                encryption_str[a],
                "Enctryption Mode",
                mode_str[m],
                "Password",
                pass,
                NULL);
}
