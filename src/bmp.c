#include "bmp.h"

#include <stdlib.h>
#include <string.h>

static uint32_t row_padding(uint32_t width) {
    uint32_t row_size = width * sizeof(struct pixel);
    return (4u - (row_size % 4u)) % 4u;
}

static void init_image_headers(struct image *image) {
    uint32_t padding = row_padding(image->width);
    uint32_t row_size = image->width * sizeof(struct pixel) + padding;
    uint32_t pixel_data_size = row_size * image->height;

    image->file_header.bfType = 0x4D42u;
    image->file_header.bfOffBits =
        (uint32_t)(sizeof(struct bmp_file_header) + sizeof(struct bmp_info_header));
    image->file_header.bfSize = image->file_header.bfOffBits + pixel_data_size;

    image->info_header.biSize = sizeof(struct bmp_info_header);
    image->info_header.biWidth = (int32_t)image->width;
    image->info_header.biHeight = (int32_t)image->height;
    image->info_header.biPlanes = 1;
    image->info_header.biBitCount = 24;
    image->info_header.biCompression = 0;
    image->info_header.biSizeImage = pixel_data_size;
    image->info_header.biClrUsed = 0;
    image->info_header.biClrImportant = 0;
}

static int allocate_pixels(struct image *image, uint32_t width, uint32_t height) {
    size_t count = (size_t)width * (size_t)height;

    image->width = width;
    image->height = height;
    image->data = NULL;

    if (count == 0) {
        return -1;
    }

    image->data = malloc(count * sizeof(struct pixel));
    if (image->data == NULL) {
        return -1;
    }

    return 0;
}

int load_bmp(FILE *input, struct image *image) {
    uint32_t padding;
    uint32_t y;

    memset(image, 0, sizeof(*image));

    if (fread(&image->file_header, sizeof(image->file_header), 1, input) != 1) {
        return -1;
    }
    if (fread(&image->info_header, sizeof(image->info_header), 1, input) != 1) {
        return -1;
    }

    if (image->file_header.bfType != 0x4D42u ||
        image->info_header.biSize != sizeof(struct bmp_info_header) ||
        image->info_header.biPlanes != 1 ||
        image->info_header.biBitCount != 24 ||
        image->info_header.biCompression != 0 ||
        image->info_header.biHeight <= 0 ||
        image->info_header.biWidth <= 0) {
        return -1;
    }

    if (allocate_pixels(image,
                        (uint32_t)image->info_header.biWidth,
                        (uint32_t)image->info_header.biHeight) != 0) {
        return -1;
    }

    if (fseek(input, (long)image->file_header.bfOffBits, SEEK_SET) != 0) {
        free_image(image);
        return -1;
    }

    padding = row_padding(image->width);
    for (y = 0; y < image->height; ++y) {
        uint32_t src_y = image->height - 1u - y;
        struct pixel *row = image->data + (size_t)src_y * image->width;
        unsigned char pad[3];

        if (fread(row, sizeof(struct pixel), image->width, input) != image->width) {
            free_image(image);
            return -1;
        }
        if (padding > 0 && fread(pad, 1, padding, input) != padding) {
            free_image(image);
            return -1;
        }
    }

    return 0;
}

int crop(const struct image *source, struct image *result,
         uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    uint32_t row;

    memset(result, 0, sizeof(*result));
    if (allocate_pixels(result, width, height) != 0) {
        return -1;
    }

    result->file_header = source->file_header;
    result->info_header = source->info_header;
    result->file_header.bfReserved1 = source->file_header.bfReserved1;
    result->file_header.bfReserved2 = source->file_header.bfReserved2;
    init_image_headers(result);
    result->info_header.biXPelsPerMeter = source->info_header.biXPelsPerMeter;
    result->info_header.biYPelsPerMeter = source->info_header.biYPelsPerMeter;

    for (row = 0; row < height; ++row) {
        const struct pixel *src_row = source->data + (size_t)(y + row) * source->width + x;
        struct pixel *dst_row = result->data + (size_t)row * width;
        memcpy(dst_row, src_row, (size_t)width * sizeof(struct pixel));
    }

    return 0;
}

int rotate(const struct image *source, struct image *result) {
    uint32_t y;
    uint32_t x;

    memset(result, 0, sizeof(*result));
    if (allocate_pixels(result, source->height, source->width) != 0) {
        return -1;
    }

    result->file_header = source->file_header;
    result->info_header = source->info_header;
    init_image_headers(result);
    result->info_header.biXPelsPerMeter = source->info_header.biXPelsPerMeter;
    result->info_header.biYPelsPerMeter = source->info_header.biYPelsPerMeter;

    for (y = 0; y < source->height; ++y) {
        for (x = 0; x < source->width; ++x) {
            uint32_t new_x = source->height - 1u - y;
            uint32_t new_y = x;
            result->data[(size_t)new_y * result->width + new_x] =
                source->data[(size_t)y * source->width + x];
        }
    }

    return 0;
}

int save_bmp(FILE *output, const struct image *image) {
    uint32_t padding = row_padding(image->width);
    uint32_t y;
    unsigned char pad[3] = {0, 0, 0};

    if (fwrite(&image->file_header, sizeof(image->file_header), 1, output) != 1) {
        return -1;
    }
    if (fwrite(&image->info_header, sizeof(image->info_header), 1, output) != 1) {
        return -1;
    }

    for (y = 0; y < image->height; ++y) {
        uint32_t src_y = image->height - 1u - y;
        const struct pixel *row = image->data + (size_t)src_y * image->width;

        if (fwrite(row, sizeof(struct pixel), image->width, output) != image->width) {
            return -1;
        }
        if (padding > 0 && fwrite(pad, 1, padding, output) != padding) {
            return -1;
        }
    }

    return 0;
}

void free_image(struct image *image) {
    free(image->data);
    image->data = NULL;
    image->width = 0;
    image->height = 0;
}
