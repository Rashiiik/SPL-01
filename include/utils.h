#ifndef UTILS_H
#define UTILS_H

#include "bmp.h"

void print_progress(float progress);
RGBA **copyImage(RGBA **pixels, int width, int height);
void convertToAscii(RGBA **pixels, int width, int height);
void negative(RGBA **pixels, int width, int height);

#endif
