#include "../include/sharpen.h"
#include "../include/denoise.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void unsharpMask(RGBA **pixels, int width, int height, int sigma, float amount) {
    printf("Applying Unsharp Mask (sigma=%d, amount=%.2f)...\n", sigma, amount);
    
    // Create a copy of the original image
    RGBA **original = copyImage(pixels, width, height);
    
    // Blur the pixels (this modifies pixels in-place)
    gaussianBlur(pixels, width, height, sigma);
    
    printf("Sharpening...\n");
    
    // Apply unsharp mask formula: sharpened = original + amount * (original - blurred)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate the difference and add it back
            float diffR = amount * (original[y][x].r - pixels[y][x].r);
            float diffG = amount * (original[y][x].g - pixels[y][x].g);
            float diffB = amount * (original[y][x].b - pixels[y][x].b);
            
            // Add difference to original and clamp to [0, 255]
            int newR = original[y][x].r + round(diffR);
            int newG = original[y][x].g + round(diffG);
            int newB = original[y][x].b + round(diffB);
            
            // Clamp values
            if (newR < 0) newR = 0;
            if (newR > 255) newR = 255;
            if (newG < 0) newG = 0;
            if (newG > 255) newG = 255;
            if (newB < 0) newB = 0;
            if (newB > 255) newB = 255;
            
            pixels[y][x].r = (unsigned char)newR;
            pixels[y][x].g = (unsigned char)newG;
            pixels[y][x].b = (unsigned char)newB;
        }
        print_progress((float)(y + 1) / height);
    }
    
    printf("\n");
    
    // Free the original copy
    for (int i = 0; i < height; i++) {
        free(original[i]);
    }
    free(original);
}
