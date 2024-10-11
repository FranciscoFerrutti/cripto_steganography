#include "lib/include/common_libs.h"
#include "lib/include/embed.h"
#include "lib/include/parse_args.h"

int main(const int argc, const char* argv[]) {
    args* args = malloc(sizeof(struct args));
    if (args == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Initialize the args struct to avoid undefined behavior
    memset(args, 0, sizeof(struct args));

    parse_args(argc, argv, args);

    // Check if the action is embed or extract
    if (args->action == EMBED) {
        bmp_file bmp;
        embed(*args, &bmp);
    }
    else if (args->action == EXTRACT) {
        // extract(args);
    }
    else {
        fprintf(stderr, "Error: Invalid action\n");
    }

    free(args);  // Free the allocated memory
    return 0;
}