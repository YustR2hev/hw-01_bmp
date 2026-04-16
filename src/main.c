#include "bmp.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_u32(const char *text, uint32_t *value) {
    char *end = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return -1;
    }
    if (parsed > UINT32_MAX) {
        return -1;
    }

    *value = (uint32_t)parsed;
    return 0;
}

int main(int argc, char **argv) {
    FILE *input = NULL;
    FILE *output = NULL;
    struct image source;
    struct image cropped;
    struct image rotated;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    int status = 1;

    memset(&source, 0, sizeof(source));
    memset(&cropped, 0, sizeof(cropped));
    memset(&rotated, 0, sizeof(rotated));

    if (argc != 8) {
        return 1;
    }
    if (strcmp(argv[1], "crop-rotate") != 0) {
        return 1;
    }
    if (parse_u32(argv[4], &x) != 0 ||
        parse_u32(argv[5], &y) != 0 ||
        parse_u32(argv[6], &width) != 0 ||
        parse_u32(argv[7], &height) != 0) {
        return 1;
    }
    if (width == 0 || height == 0) {
        return 1;
    }

    input = fopen(argv[2], "rb");
    if (input == NULL) {
        goto cleanup;
    }
    if (load_bmp(input, &source) != 0) {
        goto cleanup;
    }
    fclose(input);
    input = NULL;

    if (x >= source.width || y >= source.height) {
        goto cleanup;
    }
    if (width > source.width - x || height > source.height - y) {
        goto cleanup;
    }

    if (crop(&source, &cropped, x, y, width, height) != 0) {
        goto cleanup;
    }
    if (rotate(&cropped, &rotated) != 0) {
        goto cleanup;
    }

    output = fopen(argv[3], "wb");
    if (output == NULL) {
        goto cleanup;
    }
    if (save_bmp(output, &rotated) != 0) {
        goto cleanup;
    }

    status = 0;

cleanup:
    if (input != NULL) {
        fclose(input);
    }
    if (output != NULL) {
        fclose(output);
    }
    free_image(&source);
    free_image(&cropped);
    free_image(&rotated);
    return status;
}
