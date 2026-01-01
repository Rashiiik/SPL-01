#include "utils.h"
#include <stdio.h>
#include <string.h>

void print_progress(float progress) {
    int barWidth = 32;
    int pos = (int)(progress * barWidth);

    printf("\r[");

    for (int i = 0; i < barWidth; i++) {
        if (i < pos) {
            printf("#");
        } else {
            printf(".");
        }
    }

    printf("] %6.2f%%", progress * 100.0f);
    fflush(stdout);
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