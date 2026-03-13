#include "../include/scaling.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <xmmintrin.h>

#define NUM_THREADS 8

void *bilinear(void *arg) {

    ThreadData1 *data = (ThreadData1*)arg;

    for (int y = data->startRow; y < data->endRow; y++)
    {
        for (int x = 0; x < data->newWidth; x++)
        {
            float gx = x * data->xRatio;
            float gy = y * data->yRatio;

            int x1 = (int)gx;
            int y1 = (int)gy;

            int x2 = x1 + 1;
            int y2 = y1 + 1;

            if (x2 >= data->oldWidth) {
                x2 = data->oldWidth - 1;
            }  
            if (y2 >= data->oldHeight) {
                y2 = data->oldHeight - 1;
            } 

            float dx = gx - x1;
            float dy = gy - y1;

            RGBA p11 = data->pixels[y1][x1];
            RGBA p21 = data->pixels[y1][x2];
            RGBA p12 = data->pixels[y2][x1];
            RGBA p22 = data->pixels[y2][x2];

            data->temp[y][x].r = (1-dx)*(1-dy)*p11.r + dx*(1-dy)*p21.r + (1-dx)*dy*p12.r + dx*dy*p22.r;

            data->temp[y][x].g = (1-dx)*(1-dy)*p11.g + dx*(1-dy)*p21.g + (1-dx)*dy*p12.g + dx*dy*p22.g;

            data->temp[y][x].b = (1-dx)*(1-dy)*p11.b + dx*(1-dy)*p21.b + (1-dx)*dy*p12.b + dx*dy*p22.b;
        }
        
    }
    
    
    return NULL;
}

RGBA **bilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, bool mark) {
    //printf("Resizing Image using Bilinear Interpolation...\n");

    RGBA **temp = malloc(newHeight * sizeof(RGBA *));
    for (int i = 0; i < newHeight; i++) {
        temp[i] = malloc(newWidth * sizeof(RGBA));
    }

    float xRatio = (float)(oldWidth - 1) / (newWidth - 1);
    float yRatio = (float)(oldHeight - 1) / (newHeight - 1);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {

            float gx = x * xRatio;
            float gy = y * yRatio;

            int x1 = (int)gx;
            int y1 = (int)gy;

            int x2 = x1 + 1;
            int y2 = y1 + 1;

            if (x2 >= oldWidth) {
                x2 = oldWidth - 1;
            }  
            if (y2 >= oldHeight) {
                y2 = oldHeight - 1;
            } 

            float dx = gx - x1;
            float dy = gy - y1;

            RGBA p11 = pixels[y1][x1];
            RGBA p21 = pixels[y1][x2];
            RGBA p12 = pixels[y2][x1];
            RGBA p22 = pixels[y2][x2];

            temp[y][x].r = (1-dx)*(1-dy)*p11.r + dx*(1-dy)*p21.r + (1-dx)*dy*p12.r + dx*dy*p22.r;

            temp[y][x].g = (1-dx)*(1-dy)*p11.g + dx*(1-dy)*p21.g + (1-dx)*dy*p12.g + dx*dy*p22.g;

            temp[y][x].b = (1-dx)*(1-dy)*p11.b + dx*(1-dy)*p21.b + (1-dx)*dy*p12.b + dx*dy*p22.b;
        }

        if (mark == true)
        {
            print_progress((float)(y+1)/newHeight);
        }

    }

    printf("\n");

    for (int i = 0; i < oldHeight; i++) 
    {
        free(pixels[i]);
    }
    free(pixels);

    return temp;
}

RGBA **multithreadedBilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight) {

    RGBA **temp = malloc(newHeight * sizeof(RGBA *));
    for (int i = 0; i < newHeight; i++) {
        temp[i] = malloc(newWidth * sizeof(RGBA));
    }

    pthread_t threads[NUM_THREADS];
    ThreadData1 data[NUM_THREADS];

    int chunks = newHeight/NUM_THREADS;

    float xRatio = (float)(oldWidth - 1) / (newWidth - 1);
    float yRatio = (float)(oldHeight - 1) / (newHeight - 1);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].oldHeight = oldHeight;
        data[i].oldWidth = oldWidth;
        data[i].xRatio = xRatio;
        data[i].yRatio = yRatio;
        data[i].newHeight = newHeight;
        data[i].newWidth = newWidth;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, bilinear, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    

    for (int i = 0; i < oldHeight; i++) 
    {
        free(pixels[i]);
    }
    free(pixels);

    return temp;
}
