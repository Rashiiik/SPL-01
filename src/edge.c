#include "../include/edge.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void convertToGrayscale(RGBA **pixels, int width, int height) {

    printf("Converting to Grayscale\n");

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned char r = pixels[i][j].r;
            unsigned char g = pixels[i][j].g;
            unsigned char b = pixels[i][j].b;

            unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);

            pixels[i][j].r = gray;
            pixels[i][j].g = gray;
            pixels[i][j].b = gray;
        }
        print_progress((float)(i + 1) / height);
    }

    print_progress(1.0f);
    printf("\n");
}

void sobelOperator(RGBA **pixels, int width, int height, int threshold) {
    int Gx[3][3] = { 
        {-1, 0, 1},
        {-2, 0, 2}, 
        {-1, 0, 1}
    };

    int Gy[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    unsigned char **temp = malloc(height * sizeof(unsigned char *));
    for (int i = 0; i < height; i++) {
        temp[i] = calloc(width, sizeof(unsigned char));
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float gx = 0, gy = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;
                    
                    if (ny < 0) ny = 0;
                    if (ny >= height) ny = height - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= width) nx = width - 1;
                    
                    float intensity = pixels[ny][nx].r;

                    gx += intensity * Gx[ky+1][kx+1];
                    gy += intensity * Gy[ky+1][kx+1];
                }
            }

            float mag = sqrtf(gx*gx + gy*gy);
            
            if (mag > threshold) {
                temp[y][x] = 255;
            } else {
                temp[y][x] = 0;
            }
        }
        print_progress((float)(y+1)/height);
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char v = temp[y][x];
            pixels[y][x].r = v;
            pixels[y][x].g = v;
            pixels[y][x].b = v;
        }
    }

    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);

    printf("\n");
}
