#include "common_libs.h"
#include "parse_args.h"
#include "steganography.h"

int main(const int argc, const char* argv[]) {
    args args;
    memset(&args, 0, sizeof(struct args));

    parse_args(argc, argv, &args);

    /**
     * @todo hace el enctryption o decryption antes de embed o extract
     */
    if (args.action == EMBED) {
        embed(args.p, args.in, args.out, args.steg);
    }
    else if (args.action == EXTRACT) {
        extract(args.p, args.out, args.steg);
    }
    else {
        fprintf(stderr, "Error: Invalid action\n");
    }

    return 0;
}