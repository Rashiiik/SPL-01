#ifndef EDGE_H
#define EDGE_H

#include "bmp.h"

void convertToGrayscale(RGBA **pixels, int width, int height);
void sobelOperator(RGBA **pixels, int width, int height);
void multithreadedGrayscaling(RGBA **pixels, int width, int height);
void multithreadedSobel(RGBA **pixels, int width, int height);

#endif
