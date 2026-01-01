#ifndef FILTERS_H
#define FILTERS_H

#include "bmp.h"

void convertToGrayscale(RGBA **pixels, int width, int height);
void gaussianBlur(RGBA **pixels, int width, int height, int sigma);
void sobelOperator(RGBA **pixels, int width, int height, int threshold);
void negative(RGBA **pixels, int width, int height);
void transpose(RGBA **pixels, int width, int height);

#endif