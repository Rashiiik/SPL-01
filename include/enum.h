#include "bmp.h"

typedef struct 
{
    RGBA **pixels;
    RGBA **temp;
    float xRatio, yRatio;
    int newHeight, newWidth;
    int oldHeight, oldWidth;
    int height, width;
    int startRow, endRow;
    int sigma;
    int kernelSize;
    float *kernel;
    int center;
    float amount;

} ThreadData;