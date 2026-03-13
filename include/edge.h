#ifndef EDGE_H
#define EDGE_H

#include "bmp.h"

typedef struct 
{
    RGBA **pixels;
    unsigned char **temp;
    int threshold;
    int height;
    int width;
    int startRow;
    int endRow;
} ThreadData;


void convertToGrayscale(RGBA **pixels, int width, int height);
void sobelOperator(RGBA **pixels, int width, int height, int threshold);
void multithreadedGrayscaling(RGBA **pixels, int width, int height);
void multithreadedSobel(RGBA **pixels, int width, int height, int threshold);

#endif
