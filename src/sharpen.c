#include "../include/sharpen.h"
#include "../include/denoise.h"
#include "../include/utils.h"
#include "../include/enum.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 8

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

void unsharpMask(RGBA **pixels, int width, int height, int sigma, float amount) {
    
    RGBA **temp = copyImage(pixels, width, height);
    
    gaussianBlur(pixels, width, height, sigma);
    
    printf("Sharpening...\n");
    
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            float diffR = amount * (temp[y][x].r - pixels[y][x].r);
            float diffG = amount * (temp[y][x].g - pixels[y][x].g);
            float diffB = amount * (temp[y][x].b - pixels[y][x].b);
            
            int newR = temp[y][x].r + round(diffR);
            int newG = temp[y][x].g + round(diffG);
            int newB = temp[y][x].b + round(diffB);
            
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
            
            pixels[y][x].r = (unsigned char)newR;
            pixels[y][x].g = (unsigned char)newG;
            pixels[y][x].b = (unsigned char)newB;
        }
        print_progress((float)(y + 1) / height);
    }
    
    printf("\n");
    
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

void multithreadedUnsharpMask(RGBA **pixels, int width, int height, int sigma, float amount) {

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];
    
    RGBA **temp = copyImage(pixels, width, height);
    
    multithreadedGaussian(pixels, width, height, sigma);

    int chunks = height/NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
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

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}

void laplacianFilter(RGBA **pixels, int width, int height, float amount) {

    RGBA **temp = copyImage(pixels, width, height);

    int kernelSize = 3;

    float kernel[3][3] = { 
        {0, -1, 0},
        {-1, 4, -1},
        {0, -1, 0}
    };

    int center = kernelSize/2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int j = -center; j <= center; j++)
            {
                for (int i = -center; i <= center; i++)
                {
                    int ny = j + y;
                    int nx = i + x;

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

                    sumR += kernel[j+center][i+center] * temp[ny][nx].r;
                    sumG += kernel[j+center][i+center] * temp[ny][nx].g;
                    sumB += kernel[j+center][i+center] * temp[ny][nx].b;
                }
                
            }
            
            float lapR = sumR;
            float lapG = sumG;
            float lapB = sumB;

            int newR = temp[y][x].r + round(amount * lapR);
            int newG = temp[y][x].g + round(amount * lapG);
            int newB = temp[y][x].b + round(amount * lapB);       

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
            
            pixels[y][x].r = (unsigned char)newR;
            pixels[y][x].g = (unsigned char)newG;
            pixels[y][x].b = (unsigned char)newB;
        }
        
    }

    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);

}

void multithreadedLaplacian(RGBA **pixels, int width, int height, float amount) {

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    RGBA **temp = copyImage(pixels, width, height);

    int kernelSize = 3;

    float kernel[3][3] = {
    {0, -1, 0},
    {-1, 4, -1},
    {0, -1, 0}
};
    int center = kernelSize/2;

    int chunks = height/NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
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

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
}