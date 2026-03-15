#ifndef SCALING_H
#define SCALING_H

#include "bmp.h"
#include <stdbool.h>

RGBA **bilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight);
RGBA **multithreadedBilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight);
RGBA **lanczosInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, int radius);
RGBA **multithreadedLanczos(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, int radius);

#endif
