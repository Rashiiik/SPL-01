#include "filters.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void gaussianBlur(RGBA **pixels, int width, int height, int sigma) {
    int kernelSize = ceil(6*sigma + 1);

    if (kernelSize % 2 == 0) {
        kernelSize++;
    }

    float kernel[kernelSize];
    int center = kernelSize/2;
    float sum = 0;

    RGBA **temp = malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        temp[i] = malloc(width * sizeof(RGBA));
    }

    for (int i = 0; i < kernelSize; i++) {
        int x = i - center;
        kernel[i] = exp(-(x*x)/(2*sigma*sigma));
        sum = sum + kernel[i];
    }

    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = kernel[i]/sum;
    }

    printf("Applying Gaussian Blur...\n");
    printf("Horizontal Pass...\n");
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < kernelSize; i++) {
                int dx = i - center;
                int nx = x + dx;

                if (nx < 0) nx = 0;
                if (nx >= width) nx = width-1;
                
                sumR = sumR + kernel[i]*pixels[y][nx].r;
                sumG = sumG + kernel[i]*pixels[y][nx].g;
                sumB = sumB + kernel[i]*pixels[y][nx].b;
            }

            temp[y][x].r = sumR;
            temp[y][x].g = sumG;
            temp[y][x].b = sumB;
        }
        print_progress((float)(y+1) / height);
    }

    printf("\nVertical Pass...\n");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < kernelSize; i++) {
                int dy = i - center;
                int ny = y + dy;

                if (ny < 0) ny = 0;
                if (ny >= height) ny = height-1;
                
                sumR = sumR + kernel[i]*temp[ny][x].r;
                sumG = sumG + kernel[i]*temp[ny][x].g;
                sumB = sumB + kernel[i]*temp[ny][x].b;
            }

            pixels[y][x].r = round(sumR);
            pixels[y][x].g = round(sumG);
            pixels[y][x].b = round(sumB);
        }
        print_progress((float)(y+1)/height);
    }

    printf("\n");

    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
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
}

void transpose(RGBA **pixels, int width, int height) {
    RGBA **temp = malloc(width*(sizeof(RGBA *)));

    for (int i = 0; i < width; i++) {
        temp[i] = malloc(height*(sizeof(RGBA)));
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            temp[x][y] = pixels[y][x];
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y][x] = temp[y][x];
        }
    }

    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

void negative(RGBA **pixels, int width, int height) {
    printf("Converting to negative....\n");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y][x].r = 255 - pixels[y][x].r;
            pixels[y][x].g = 255 - pixels[y][x].g;
            pixels[y][x].b = 255 - pixels[y][x].b;
        }
        print_progress((float)(y+1)/height);
    }

    printf("\n");
}

