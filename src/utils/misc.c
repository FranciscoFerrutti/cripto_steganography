#include "misc.h"

void print_line(size_t table_width) {
    printf("+");
    for (size_t i = 0; i < table_width - 2; i++)
        printf("-");
    printf("+\n");
}
void set_rgb_color_from_int(int color) {
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    printf("\033[38;2;%d;%d;%dm", r, g, b);
}
void reset_color() {
    printf("\033[0m");
}
/**
 * @brief Print a table with the given attributes and values
 *
 * @param header         Header of the table
 * @param firstAttribute First attribute to print
 * @param ...            Pairs of attributes and values to print
 *
 * Example usage:
 *
 * print_table( "Output file",
 *              "output.bmp",
 *              "Steganography method",
 *              "LSB1",
 *              "Additional Info",
 *              "Extra data",
 *              NULL  // Must terminate with NULL
 * );
 *
 */
void print_table(const char *header, int color, const char *firstAttribute, ...) {
    va_list     args;
    const char *attribute;
    const char *value;
    char       *formattedRow;

    size_t headerLength = strlen(header);
    size_t tableWidth   = (headerLength > 49) ? headerLength + 4 : 53;

    set_rgb_color_from_int(color);

    print_line(tableWidth);
    printf("| %-*s |\n", (int) (tableWidth - 4), header);
    print_line(tableWidth);

    reset_color();

    printf("| %-20s | %-26s |\n", "Attribute", "Value");

    print_line(tableWidth);

    va_start(args, firstAttribute);

    attribute = firstAttribute;
    while (attribute != NULL) {
        value = va_arg(args, const char *);
        if (value == NULL) {
            break;
        }

        asprintf(&formattedRow, "| %-20s | %-26s |\n", attribute, value);
        printf("%s", formattedRow);
        free(formattedRow);

        attribute = va_arg(args, const char *);
    }

    print_line(tableWidth);

    va_end(args);  // Clean up the va_list
}

void printerr(const char *format, ...) {
    va_list args;

    // Start processing the variable arguments
    va_start(args, format);

    // Print the "Error" message in red
    fprintf(stderr, "\033[0;31mError\033[0m: ");

    // Print the formatted message (like printf)
    vfprintf(stderr, format, args);

    // Clean up the variable arguments list
    va_end(args);
}