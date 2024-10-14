#include "common_libs.h"
#include "misc.h"
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
     * @todo por alguna razon el tamaño según el ejemplo tiene ridiculamente muchos bytes
     * @todo  Para ello controlar el parámetro de compresión del encabezado.
     * No se puede encriptar/desencriptar sin password. Si este dato no está, sólo se
     * esteganografia. Son válidas en cambio las siguientes opciones:
     * - indicar algoritmo y password pero no modo: Se asume CBC por default.
     * - Indicar modo y password pero no algoritmo: Se asume aes128 por default.
     * - Indicar sólo password: Se asume algoritmo aes128 en modo CBC por default.
     *
     */

    if (args.action == EMBED) {
        embed(args.p, args.in, args.out, args.steg, args.a, args.m, args.pass);
    }
    else if (args.action == EXTRACT) {
        extract(args.p, args.out, args.steg, args.a, args.m, args.pass);
    }

    return 0;
}