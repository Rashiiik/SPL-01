#include "../include/scaling.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>

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

    for (int i = 0; i < height; i++) 
    {
        free(temp[i]);
    }
    free(temp);
}

RGBA **bilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight) {
    printf("Resizing Image using Bilinear Interpolation...\n");

    RGBA **temp = malloc(newHeight * sizeof(RGBA *));
    for (int i = 0; i < newHeight; i++) {
        temp[i] = malloc(newWidth * sizeof(RGBA));
    }

    float xRatio = (float)(oldWidth - 1) / (newWidth - 1);
    float yRatio = (float)(oldHeight - 1) / (newHeight - 1);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {

            float gx = x * xRatio;
            float gy = y * yRatio;

            int x1 = (int)gx;
            int y1 = (int)gy;

            int x2 = x1 + 1;
            int y2 = y1 + 1;

            if (x2 >= oldWidth) {
                x2 = oldWidth - 1;
            }  
            if (y2 >= oldHeight) {
                y2 = oldHeight - 1;
            } 

            float dx = gx - x1;
            float dy = gy - y1;

            RGBA p11 = pixels[y1][x1];
            RGBA p21 = pixels[y1][x2];
            RGBA p12 = pixels[y2][x1];
            RGBA p22 = pixels[y2][x2];

            temp[y][x].r = (1-dx)*(1-dy)*p11.r + dx*(1-dy)*p21.r + (1-dx)*dy*p12.r + dx*dy*p22.r;

            temp[y][x].g = (1-dx)*(1-dy)*p11.g + dx*(1-dy)*p21.g + (1-dx)*dy*p12.g + dx*dy*p22.g;

            temp[y][x].b = (1-dx)*(1-dy)*p11.b + dx*(1-dy)*p21.b + (1-dx)*dy*p12.b + dx*dy*p22.b;
        }

        print_progress((float)(y+1)/newHeight);
    }

    printf("\n");

    for (int i = 0; i < oldHeight; i++) 
    {
        free(pixels[i]);
    }
    free(pixels);

    return temp;
}
