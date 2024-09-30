#include "lib/include/common_libs.h"
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

    free(args);  // Free the allocated memory
    return 0;
}