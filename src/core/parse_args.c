#include "parse_args.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_libs.h"

#define HELP_MSG \
    "\nUsage for concealment: \n\t\
stegobmp -embed -in <file> -p <bitmapfile> -out <bitmapfile> -steg <LSB1 | LSB4 | LSBI>\n\n\
\nConcealment command parameters:\n\
-embed: option for concealment\n\
-in <file>: indicates the file to conceal\n\
-p <bitmapfile>: carrier bmp file\n\
-out <bitmapfile>: bmp output file with embedded information\n\
-steg <LSB1 | LSB4 | LSBI>: steganography algorithm. \n\tOptions are: LSB (1bit), LSB (4 bits), LSB (Enhanced)\n\
\nConcealment optional parameters:\n\
-a <aes128 | aes192 | aes256 | 3des>\n\
-m <ecb | cfb | ofb | cbc>\n\
-pass password: encryption password\n\
\n\n\nUsage for extraction:\n\
stegobmp -extract -p <bitmapfile> -out <file> -steg <LSB1 | LSB4 | LSBI> -a <aes128 | aes192 | aes256 | 3des> -m <ecb | cfb | ofb | cbc> -pass <password>\n\
\nExtraction command parameters:\n\
-extract: option for extraction from bmp file\n\
-p <bitmapfile>: bmp carrier file\n\
-out <file>: file to be overwritten with output\n"

void print_help() {
    printf("%s\n", HELP_MSG);
}

void parse_args(const int argc, const char *argv[], args *args) {
    int option_index = 0;
    int opt;

    args->action = NONE;
    args->in     = NULL;
    args->p      = NULL;
    args->out    = NULL;
    args->steg   = STEG_NONE;
    args->a      = ENC_NONE;
    args->m      = MODE_NONE;
    args->pass   = NULL;

    if (argc < 2) {
        print_help();
        exit(1);
    }

    // larger than 1 character commands should be -- and single character commands should be -
    static struct option long_options[] = {{"embed", no_argument, 0, 'e'},
                                           {"extract", no_argument, 0, 'x'},
                                           {"in", required_argument, 0, 'i'},
                                           {"p", required_argument, 0, 'p'},
                                           {"out", required_argument, 0, 'o'},
                                           {"steg", required_argument, 0, 's'},
                                           {"a", required_argument, 0, 'a'},
                                           {"m", required_argument, 0, 'm'},
                                           {"pass", required_argument, 0, 'k'},
                                           {"help", no_argument, 0, 'h'},
                                           {0, 0, 0, 0}};

    while ((opt = getopt_long(
                argc, (char *const *) argv, "exi:p:o:s:a:m:k:h", long_options, &option_index)) !=
           -1) {
        switch (opt) {
            case 'e':  // Embed option
                args->action = EMBED;
                break;
            case 'x':  // Extract option
                args->action = EXTRACT;
                break;
            case 'i':  // Input file
                args->in = optarg;
                break;
            case 'p':  // Carrier file
                args->p = optarg;
                break;
            case 'o':  // Output file
                args->out = optarg;
                break;
            case 's':  // Steganography method
                if (strcmp(optarg, "LSB1") == 0) {
                    args->steg = LSB1;
                }
                else if (strcmp(optarg, "LSB4") == 0) {
                    args->steg = LSB4;
                }
                else if (strcmp(optarg, "LSBI") == 0) {
                    args->steg = LSBI;
                }
                else {
                    fprintf(stderr, "Invalid steg value: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'a':  // Encryption algorithm
                if (strcmp(optarg, "aes128") == 0) {
                    args->a = AES128;
                }
                else if (strcmp(optarg, "aes192") == 0) {
                    args->a = AES192;
                }
                else if (strcmp(optarg, "aes256") == 0) {
                    args->a = AES256;
                }
                else if (strcmp(optarg, "3des") == 0) {
                    args->a = DES3;
                }
                else {
                    fprintf(stderr, "Invalid encryption algorithm: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'm':  // Encryption mode
                if (strcmp(optarg, "ecb") == 0) {
                    args->m = ECB;
                }
                else if (strcmp(optarg, "cfb") == 0) {
                    args->m = CFB;
                }
                else if (strcmp(optarg, "ofb") == 0) {
                    args->m = OFB;
                }
                else if (strcmp(optarg, "cbc") == 0) {
                    args->m = CBC;
                }
                else {
                    fprintf(stderr, "Invalid mode value: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'k':
                args->pass = optarg;
                break;
            case 'h':
            case '?':
                print_help();
                exit(0);
            default:
                print_help();
                exit(1);
        }
    }

    if (args->action == EMBED) {
        if (!args->in || !args->p || !args->out || !args->steg) {
            fprintf(stderr, "Error: Missing required arguments for embedding.\n");
            print_help();
            exit(1);
        }
    }
    else if (args->action == EXTRACT) {
        if (!args->p || !args->out || !args->steg) {
            fprintf(stderr, "Error: Missing required arguments for extraction.\n");
            print_help();
            exit(1);
        }
    }
    else {
        fprintf(stderr, "Error: No action specified. Use --embed or --extract.\n");
        print_help();
        exit(1);
    }
}
