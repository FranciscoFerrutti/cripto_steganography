#include "parse_args.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_libs.h"
#include "misc.h"

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

void print_args(const int argc, const char *argv[]) {
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
}

void print_help() {
    printf("%s\n", HELP_MSG);
}

void print_error(const char *msg) {
    printf("Error: %s\n", msg);
    print_help();
}

void parse_args(const int argc, const char *argv[], args *args) {
    if (argc < 2) {
        print_error("Invalid number of arguments");
        exit(1);
    }

    if (strcmp(argv[1], "-help") == 0) {
        print_help();
        exit(0);
    }

    if (strcmp(argv[1], "-embed") == 0) {
        args->action = EMBED;
    }
    else if (strcmp(argv[1], "-extract") == 0) {
        args->action = EXTRACT;
    }
    else {
        print_error("Invalid action");
        exit(1);
    }

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-in") == 0) {
            if (i + 1 < argc) {
                args->in = argv[i + 1];
                i++;
            }
            else {
                print_error("Missing value for -in");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                args->p = argv[i + 1];
                i++;
            }
            else {
                print_error("Missing value for -p");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-out") == 0) {
            if (i + 1 < argc) {
                args->out = argv[i + 1];
                i++;
            }
            else {
                print_error("Missing value for -out");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-steg") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "LSB1") == 0) {
                    args->steg = LSB1;
                }
                else if (strcmp(argv[i + 1], "LSB4") == 0) {
                    args->steg = LSB4;
                }
                else if (strcmp(argv[i + 1], "LSBI") == 0) {
                    args->steg = LSBI;
                }
                else {
                    print_error("Invalid steg");
                    exit(1);
                }
                i++;
            }
            else {
                print_error("Missing value for -steg");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "aes128") == 0) {
                    args->a = AES128;
                }
                else if (strcmp(argv[i + 1], "aes192") == 0) {
                    args->a = AES192;
                }
                else if (strcmp(argv[i + 1], "aes256") == 0) {
                    args->a = AES256;
                }
                else if (strcmp(argv[i + 1], "3des") == 0) {
                    args->a = DES3;
                }
                else {
                    args->a = ENC_NONE;
                }
                i++;
            }
            else {
                print_error("Missing value for -a");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "ecb") == 0) {
                    args->m = ECB;
                }
                else if (strcmp(argv[i + 1], "cfb") == 0) {
                    args->m = CFB;
                }
                else if (strcmp(argv[i + 1], "ofb") == 0) {
                    args->m = OFB;
                }
                else if (strcmp(argv[i + 1], "cbc") == 0) {
                    args->m = CBC;
                }
                else {
                    args->m = MODE_NONE;
                }
                i++;
            }
            else {
                print_error("Missing value for -m");
                exit(1);
            }
        }
        else if (strcmp(argv[i], "-pass") == 0) {
            if (i + 1 < argc) {
                args->pass = argv[i + 1];
                i++;
            }
            else {
                print_error("Missing value for -pass");
                exit(1);
            }
        }
        else {
            print_error("Invalid argument");
            exit(1);
        }
    }
}
