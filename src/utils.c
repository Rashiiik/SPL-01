#include "../include/utils.h"
#include "../include/bmp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

RGBA **copyImage(RGBA **pixels, int width, int height) {
    RGBA **newImage = (RGBA **)malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        newImage[i] = (RGBA *)malloc(width * sizeof(RGBA));
        memcpy(newImage[i], pixels[i], width * sizeof(RGBA));
    }
    return newImage;
}

void convertToAscii(RGBA **pixels, int width, int height) {
    const char asciiChar[] = "@%%#*+=-:. ";
    int len = strlen(asciiChar);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = pixels[y][x].r * (len-1)/255;
            printf("%c", asciiChar[index]);
        }
        printf("\n");
    }
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

    for (int i = 0; i < height; i++) 
    {
        free(temp[i]);
    }
    free(temp);
}

void negative(RGBA **pixels, int width, int height) {
    printf("Converting to negative....\n");

    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            pixels[y][x].r = 255 - pixels[y][x].r;
            pixels[y][x].g = 255 - pixels[y][x].g;
            pixels[y][x].b = 255 - pixels[y][x].b;
        }
    }

    printf("\n");
}
