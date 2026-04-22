#ifndef DENOISE_H
#define DENOISE_H

#include "bmp.h"

void gaussianBlur(RGBA **pixels, int width, int height, int sigma, int threadCount);
void medianFilter(RGBA **pixels, int width, int height, int kernelSize, int threadCount);
void nonLocalMeans(RGBA **pixels, int width, int height, int searchRadius, int patchRadius, int threadCount);

#endif
