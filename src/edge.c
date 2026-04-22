#include "../include/edge.h"
#include "../include/utils.h"
#include "../include/enum.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

int Gx[3][3] = { 
    {-1, 0, 1},
    {-2, 0, 2}, 
    {-1, 0, 1}
};

int Gy[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}
};

float maxMag = 0;
float autoThreshold;

pthread_mutex_t maxMagMutex;

void *grayscale(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    for (int y = data->startRow; y < data->endRow; y++)
    {
        for (int x = 0; x < data->width; x++)
        {
            unsigned char r = data->pixels[y][x].r;
            unsigned char g = data->pixels[y][x].g;
            unsigned char b = data->pixels[y][x].b;

            unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);

            data->pixels[y][x].r = gray;
            data->pixels[y][x].g = gray;
            data->pixels[y][x].b = gray;
        }
        
    }

    return NULL;
    
}

void *sobelWorkerP1(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    float localMax = 0;

    for (int y = data->startRow; y < data->endRow; y++) {
        for (int x = 0; x < data->width; x++) {
            float gx = 0, gy = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;
                    
                    if (ny < 0) ny = 0;
                    if (ny >= data->height) ny = data->height - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= data->width) nx = data->width - 1;
                    
                    float intensity = data->pixels[ny][nx].r;

                    gx += intensity * Gx[ky+1][kx+1];
                    gy += intensity * Gy[ky+1][kx+1];
                }
            }

            
            float mag = sqrtf(gx*gx + gy*gy);
            
            if (mag > localMax) localMax = mag;
            
        }

    }

    pthread_mutex_lock(&maxMagMutex);
    if (localMax > maxMag)
    {
        maxMag = localMax;
    }
    pthread_mutex_unlock(&maxMagMutex);
    
    return NULL;
}

void *sobelWorkerP2(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    for (int y = data->startRow; y < data->endRow; y++) {
        for (int x = 0; x < data->width; x++) {
            float gx = 0, gy = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;
                    
                    if (ny < 0) ny = 0;
                    if (ny >= data->height) ny = data->height - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= data->width) nx = data->width - 1;
                    
                    float intensity = data->pixels[ny][nx].r;

                    gx += intensity * Gx[ky+1][kx+1];
                    gy += intensity * Gy[ky+1][kx+1];
                }
            }

            
            float mag = sqrtf(gx*gx + gy*gy);

            if (mag > autoThreshold) {
                data->temp[y][x].r = 255;
                data->temp[y][x].g = 255;
                data->temp[y][x].b = 255;
            } else {
                data->temp[y][x].r = 0;
                data->temp[y][x].g = 0;
                data->temp[y][x].b = 0;
            }
        }
    }

    return NULL;
}

void convertToGrayscale(RGBA **pixels, int width, int height, int threadCount) {

    printf("Converting to Grayscale\n");

    pthread_t threads[threadCount];
    ThreadData data[threadCount];

    int chunks = height/threadCount;

    for (int i = 0; i < threadCount; i++)
    {
        data[i].pixels = pixels;
        data[i].width = width;
        data[i].height = height;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, grayscale, &data[i]);
    }

    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    
}

void sobelOperator(RGBA **pixels, int width, int height, int threadCount) {

    maxMag = 0;
    
    RGBA *tempData = malloc(width * height * sizeof(RGBA));
    
    RGBA **temp = malloc(height * sizeof(RGBA *));
    
    for (int i = 0; i < height; i++) {
        temp[i] = tempData + i * width;
    }

    pthread_t threads[threadCount];
    ThreadData data[threadCount];

    int chunks = height/threadCount;

    pthread_mutex_init(&maxMagMutex, NULL);

    for (int i = 0; i < threadCount; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].height = height;
        data[i].width = width;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, sobelWorkerP1, &data[i]);
    }

    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&maxMagMutex);

    autoThreshold = 0.2f * maxMag;

    for (int i = 0; i < threadCount; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].height = height;
        data[i].width = width;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, sobelWorkerP2, &data[i]);
    }

    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(threads[i], NULL);
    }
    

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y][x] = temp[y][x];
        }
    }

    free(tempData);
    free(temp);
    
}