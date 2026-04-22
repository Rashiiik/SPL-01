#include "../include/sharpen.h"
#include "../include/denoise.h"
#include "../include/utils.h"
#include "../include/enum.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 16

void *unsharpWorker(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    for (int y = data->startRow; y < data->endRow; y++) 
    {
        for (int x = 0; x < data->width; x++) 
        {
            float diffR = data->amount * (data->temp[y][x].r - data->pixels[y][x].r);
            float diffG = data->amount * (data->temp[y][x].g - data->pixels[y][x].g);
            float diffB = data->amount * (data->temp[y][x].b - data->pixels[y][x].b);
            
            int newR = data->temp[y][x].r + round(diffR);
            int newG = data->temp[y][x].g + round(diffG);
            int newB = data->temp[y][x].b + round(diffB);
            
            if (newR < 0) 
            {
                newR = 0;
            }
            if (newR > 255) 
            {
                newR = 255;
            }
            if (newG < 0) 
            { 
                newG = 0; 
            }
            if (newG > 255) 
            {
                newG = 255;
            }
            if (newB < 0) 
            {
                newB = 0;
            }
            if (newB > 255) 
            {
                newB = 255;
            }
            
            data->pixels[y][x].r = (unsigned char)newR;
            data->pixels[y][x].g = (unsigned char)newG;
            data->pixels[y][x].b = (unsigned char)newB;
        }
    }

    return NULL;
}

void *laplacianWorker(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    for (int y = data->startRow; y < data->endRow; y++) {
        for (int x = 0; x < data->width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int j = -data->center; j <= data->center; j++)
            {
                for (int i = -data->center; i <= data->center; i++)
                {
                    int ny = j + y;
                    int nx = i + x;

                    if (ny < 0) 
                    {
                        ny = 0;
                    }
                    if (ny >= data->height) 
                    {
                        ny = data->height - 1;
                    }
                    if (nx < 0) 
                    {
                        nx = 0;
                    }
                    if (nx >= data->width) 
                    {
                        nx = data->width - 1;
                    }

                    sumR += data->Kernel[j+data->center][i+data->center] * data->temp[ny][nx].r;
                    sumG += data->Kernel[j+data->center][i+data->center] * data->temp[ny][nx].g;
                    sumB += data->Kernel[j+data->center][i+data->center] * data->temp[ny][nx].b;
                }
                
            }
            
            float lapR = sumR;
            float lapG = sumG;
            float lapB = sumB;

            int newR = data->temp[y][x].r + round(data->amount * lapR);
            int newG = data->temp[y][x].g + round(data->amount * lapG);
            int newB = data->temp[y][x].b + round(data->amount * lapB);       

            if (newR < 0) 
            {
                newR = 0;
            }
            if (newR > 255) 
            {
                newR = 255;
            }
            if (newG < 0) 
            { 
                newG = 0; 
            }
            if (newG > 255) 
            {
                newG = 255;
            }
            if (newB < 0) 
            {
                newB = 0;
            }
            if (newB > 255) 
            {
                newB = 255;
            }
            
            data->pixels[y][x].r = (unsigned char)newR;
            data->pixels[y][x].g = (unsigned char)newG;
            data->pixels[y][x].b = (unsigned char)newB;
        }
        
    }

    return NULL;
}

void unsharpMask(RGBA **pixels, int width, int height, int sigma, float amount, int threadCount) {

    pthread_t threads[threadCount];
    ThreadData data[threadCount];
    
    RGBA **temp = copyImage(pixels, width, height);
    
    gaussianBlur(pixels, width, height, sigma, threadCount);

    int chunks = height/threadCount;

    for (int i = 0; i < threadCount; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].width = width;
        data[i].height = height;
        data[i].sigma = sigma;
        data[i].amount = amount;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, unsharpWorker, &data[i]);
    }

    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

void laplacianFilter(RGBA **pixels, int width, int height, float amount, int threadCount) {

    pthread_t threads[threadCount];
    ThreadData data[threadCount];

    RGBA **temp = copyImage(pixels, width, height);

    int kernelSize = 3;

    float kernel[3][3] = {
    {0, -1, 0},
    {-1, 4, -1},
    {0, -1, 0}
    };
    
    int center = kernelSize/2;

    int chunks = height/threadCount;

    for (int i = 0; i < threadCount; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].width = width;
        data[i].height = height;
        data[i].amount = amount;
        data[i].center = center;
        
        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                data[i].Kernel[r][c] = kernel[r][c];
            }
        }
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, laplacianWorker, &data[i]);
    }

    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}