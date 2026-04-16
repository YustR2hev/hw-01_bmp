#ifndef BMP_H
#define BMP_H

#include <stdint.h>
#include <stdio.h>

#pragma pack(push, 1)
struct bmp_file_header {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct bmp_info_header {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

struct pixel {
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

struct image {
    uint32_t width;
    uint32_t height;
    struct pixel *data;
    struct bmp_file_header file_header;
    struct bmp_info_header info_header;
};

int load_bmp(FILE *input, struct image *image);
int crop(const struct image *source, struct image *result,
         uint32_t x, uint32_t y, uint32_t width, uint32_t height);
int rotate(const struct image *source, struct image *result);
int save_bmp(FILE *output, const struct image *image);
void free_image(struct image *image);

#endif
