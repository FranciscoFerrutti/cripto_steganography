#include "common_libs.h"
#include "parse_args.h"
#include "steganography.h"

int main(const int argc, const char* argv[]) {
    args args;
    memset(&args, 0, sizeof(struct args));

    /*
        args:
            - action
            - in
            - p
            - out
            - steg
            - a
            - m
            - pass
    */
    parse_args(argc, argv, &args);

    /**
     * @todo preguntar y saber todo el tema de que hacer con la extension y el directory de las
     * cosas porque el ejemplo muestra que le mandan "file.txt"
     * @todo little endian/ bg endian?
     * @todo los archivos lado estan encriptados?
     * @todo checkear todas las funcionalidades
     *
     */
    if (args.action == EMBED) {
        embed(args.p, args.in, args.out, args.steg, args.a, args.m, args.pass);
    }
    else if (args.action == EXTRACT) {
        extract(args.p, args.out, args.steg, args.a, args.m, args.pass);
    }
    else {
        fprintf(stderr, "Error: Invalid action\n");
    }

    return 0;
}