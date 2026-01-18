#include "../include/bmp.h"
#include <stdio.h>
#include <stdlib.h>

RGBA **readBmp(const char *filename, int *width, int *height, int *bpp) {
    FILE *fp = fopen(filename, "rb");

    if (!fp) {
        perror("Cannot open file");
        return NULL;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    if (fileHeader.bfType != 0x4D42) {
        printf("Not a BMP file.\n");
        fclose(fp);
        return NULL;
    }

    *width = infoHeader.biWidth;
    *height = infoHeader.biHeight;
    *bpp = infoHeader.biBitCount;

    if (*bpp != 24 && *bpp != 32) {
        printf("Only 24-bit and 32-bit BMP supported.\n");
        fclose(fp);
        return NULL;
    }

    RGBA **pixels = (RGBA **)malloc(*height * sizeof(RGBA *));
    for (int i = 0; i < *height; i++) {
        pixels[i] = (RGBA *)malloc(*width * sizeof(RGBA));
    }

    int row_padded = (*bpp == 24) ? ((*width * 3 + 3) & (~3)) : (*width * 4);
    unsigned char *row = (unsigned char *)malloc(row_padded);

    fseek(fp, fileHeader.bfOffBits, SEEK_SET);

    for (int i = 0; i < *height; i++) {
        fread(row, sizeof(unsigned char), row_padded, fp);

        for (int j = 0; j < *width; j++) {
            if (*bpp == 24) {
                pixels[*height - 1 - i][j].b = row[j * 3];
                pixels[*height - 1 - i][j].g = row[j * 3 + 1];
                pixels[*height - 1 - i][j].r = row[j * 3 + 2];
                pixels[*height - 1 - i][j].a = 0;
            } else {
                pixels[*height - 1 - i][j].b = row[j * 4];
                pixels[*height - 1 - i][j].g = row[j * 4 + 1];
                pixels[*height - 1 - i][j].r = row[j * 4 + 2];
                pixels[*height - 1 - i][j].a = row[j * 4 + 3];
            }
        }
    }

    free(row);
    fclose(fp);
    return pixels;
}

int writeBmp(const char *filename, RGBA **pixels, int width, int height) {
    FILE *fp = fopen(filename, "wb");

    if (!fp) {
        printf("Cannot create output file");
        return 0;
    }

    int row_padded = (width * 3 + 3) & (~3);
    int pixel_data_size = row_padded * height;

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    fileHeader.bfType = 0x4D42;
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pixel_data_size;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = pixel_data_size;
    infoHeader.biXPelsPerMeter = 2835;
    infoHeader.biYPelsPerMeter = 2835;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    unsigned char *row = (unsigned char *)malloc(row_padded);

    for (int y = 0; y < height; y++) {
        int src_y = height - 1 - y;

        for (int x = 0; x < width; x++) {
            row[x * 3 + 0] = pixels[src_y][x].b;
            row[x * 3 + 1] = pixels[src_y][x].g;
            row[x * 3 + 2] = pixels[src_y][x].r;
        }

        for (int p = width * 3; p < row_padded; p++) {
            row[p] = 0;
        }

        fwrite(row, 1, row_padded, fp);
    }

    free(row);
    fclose(fp);

    printf("Image written to output: %s\n", filename);
    return 1;
}
