#ifndef DENOISE_H
#define DENOISE_H

#include "bmp.h"

void gaussianBlur(RGBA **pixels, int width, int height, int sigma);
void medianFilter(RGBA **pixels, int width, int height, int kernelSize);

#endif
