#ifndef UTILS_H
#define UTILS_H

#include "bmp.h"

ImageSoA convertToSoA(RGBA **pixels, int width, int height);
RGBA **convertToRGBA(ImageSoA img, int width, int height);
RGBA **copyImage(RGBA **pixels, int width, int height);
void convertToAscii(RGBA **pixels, int width, int height);
void negative(RGBA **pixels, int width, int height);
void transpose(RGBA **pixels, int width, int height);

#endif
