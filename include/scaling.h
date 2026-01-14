#ifndef SCALING_H
#define SCALING_H

#include "bmp.h"

RGBA **bilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight);

#endif
