#ifndef SCALING_H
#define SCALING_H

#include "bmp.h"
#include <stdbool.h>

typedef struct 
{
    RGBA **pixels;
    RGBA **temp;

    float xRatio, yRatio;

    int oldWidth, oldHeight, newWidth, newHeight;

    int startRow, endRow;

} ThreadData1;


RGBA **bilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, bool mark);
RGBA **multithreadedBilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight);

#endif
