#ifndef SCALING_H
#define SCALING_H

#include "bmp.h"
#include <stdbool.h>

RGBA **bilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, bool mark);

#endif
