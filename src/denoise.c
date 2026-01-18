#include "../include/denoise.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void gaussianBlur(RGBA **pixels, int width, int height, int sigma) {
    int kernelSize = ceil(6*sigma + 1);

    if (kernelSize % 2 == 0) {
        kernelSize++;
    }

    float kernel[kernelSize];
    int center = kernelSize/2;
    float sum = 0;

    RGBA **temp = malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        temp[i] = malloc(width * sizeof(RGBA));
    }

    for (int i = 0; i < kernelSize; i++) {
        int x = i - center;
        kernel[i] = exp(-(x*x)/(2*sigma*sigma));
        sum = sum + kernel[i];
    }

    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = kernel[i]/sum;
    }

    printf("Applying Gaussian Blur...\n");
    printf("Horizontal Pass...\n");
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < kernelSize; i++) {
                int dx = i - center;
                int nx = x + dx;

                if (nx < 0) nx = 0;
                if (nx >= width) nx = width-1;
                
                sumR = sumR + kernel[i]*pixels[y][nx].r;
                sumG = sumG + kernel[i]*pixels[y][nx].g;
                sumB = sumB + kernel[i]*pixels[y][nx].b;
            }

            temp[y][x].r = sumR;
            temp[y][x].g = sumG;
            temp[y][x].b = sumB;
        }
        print_progress((float)(y+1) / height);
    }

    printf("\nVertical Pass...\n");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < kernelSize; i++) {
                int dy = i - center;
                int ny = y + dy;

                if (ny < 0) ny = 0;
                if (ny >= height) ny = height-1;
                
                sumR = sumR + kernel[i]*temp[ny][x].r;
                sumG = sumG + kernel[i]*temp[ny][x].g;
                sumB = sumB + kernel[i]*temp[ny][x].b;
            }

            pixels[y][x].r = round(sumR);
            pixels[y][x].g = round(sumG);
            pixels[y][x].b = round(sumB);
        }
        print_progress((float)(y+1)/height);
    }

    printf("\n");

    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

void medianFilter(RGBA **pixels, int width, int height, int kernelSize) {

    if (kernelSize%2== 0)
    {
        kernelSize++;
    }
    
    RGBA **output = malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        output[i] = malloc(width * sizeof(RGBA));
    }

    int center = kernelSize / 2;
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int rValues[kernelSize * kernelSize];
            int gValues[kernelSize * kernelSize];
            int bValues[kernelSize * kernelSize];
            int count = 0;
            
            for (int ky = -center; ky <= center; ky++)
            {
                for (int kx = -center; kx <= center; kx++)
                {
                    int ny = y + ky;
                    int nx = x + kx;

                    if (ny < 0) 
                    {
                        ny = 0;
                    }
                    if (ny >= height) 
                    {
                        ny = height - 1;
                    }
                    if (nx < 0) 
                    {
                        nx = 0;
                    }
                    if (nx >= width) 
                    {
                        nx = width - 1;
                    }

                    rValues[count] = pixels[ny][nx].r;
                    gValues[count] = pixels[ny][nx].g;
                    bValues[count] = pixels[ny][nx].b;
                    count++;
                }
            }

            // Doing bubble sort to sort the pixel values
                for (int i = 0; i < count - 1; i++) 
                {
                    for (int j = 0; j < count - i - 1; j++) 
                    {
                        if (rValues[j] > rValues[j + 1]) 
                        {
                            int temp = rValues[j];
                            rValues[j] = rValues[j + 1];
                            rValues[j + 1] = temp;
                        }
                        if (gValues[j] > gValues[j + 1]) 
                        {
                            int temp = gValues[j];
                            gValues[j] = gValues[j + 1];
                            gValues[j + 1] = temp;
                        }
                        if (bValues[j] > bValues[j + 1]) 
                        {
                            int temp = bValues[j];
                            bValues[j] = bValues[j + 1];
                            bValues[j + 1] = temp;
                        }
                    }
                }
                int medianIndex = count / 2;
                output[y][x].r = rValues[medianIndex];
                output[y][x].g = gValues[medianIndex];
                output[y][x].b = bValues[medianIndex];
        }
        print_progress((float)(y+1)/height);
        printf("\n");
        
    }

    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            pixels[y][x] = output[y][x];
        }
    }

    for (int i = 0; i < height; i++) 
    {
        free(output[i]);
    }
    free(output);
    
}
