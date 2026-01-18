#ifndef SHARPEN_H
#define SHARPEN_H

#include "bmp.h"

void unsharpMask(RGBA **pixels, int width, int height, int sigma, float amount);

#endif
