#include "../include/edge.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 8

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

void *sobelWorker(void *arg) {

    ThreadData *data = (ThreadData*)arg;

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
            
            if (mag > data->threshold) 
            {
                data->temp[y][x] = 255;
            } else {
                data->temp[y][x] = 0;
            }
        }
    }

    for (int y = data->startRow; y < data->endRow; y++) {
        for (int x = 0; x < data->width; x++) {
            unsigned char v = data->temp[y][x];
            data->pixels[y][x].r = v;
            data->pixels[y][x].g = v;
            data->pixels[y][x].b = v;
        }
    }

    return NULL;
}

void convertToGrayscale(RGBA **pixels, int width, int height) {

    printf("Converting to Grayscale\n");

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char r = pixels[y][x].r;
            unsigned char g = pixels[y][x].g;
            unsigned char b = pixels[y][x].b;

            unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);

            pixels[y][x].r = gray;
            pixels[y][x].g = gray;
            pixels[y][x].b = gray;
        }
        print_progress((float)(y + 1) / height);
    }

    print_progress(1.0f);
    printf("\n");
}

void multithreadedGrayscaling(RGBA **pixels, int width, int height) {

    printf("Converting to Grayscale\n");

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    int chunks = height/NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].width = width;
        data[i].height = height;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, grayscale, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    
}

void multithreadedSobel(RGBA **pixels, int width, int height, int threshold) {
    
    unsigned char *tempData = malloc(width * height);
    unsigned char **temp = malloc(height * sizeof(unsigned char*));

    for (int i = 0; i < height; i++) {
        temp[i] = tempData + i * width;
    }

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    int chunks = height/NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].height = height;
        data[i].width = width;
        data[i].threshold = threshold;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, sobelWorker, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(tempData);
    free(temp);
    

}

void sobelOperator(RGBA **pixels, int width, int height, int threshold) {
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

    unsigned char **temp = malloc(height * sizeof(unsigned char *));

    for (int i = 0; i < height; i++) 
    {
        temp[i] = malloc(width * sizeof(unsigned char));
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float gx = 0, gy = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;
                    
                    if (ny < 0) ny = 0;
                    if (ny >= height) ny = height - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= width) nx = width - 1;
                    
                    float intensity = pixels[ny][nx].r;

                    gx += intensity * Gx[ky+1][kx+1];
                    gy += intensity * Gy[ky+1][kx+1];
                }
            }

            float mag = sqrtf(gx*gx + gy*gy);
            
            if (mag > threshold) {
                temp[y][x] = 255;
            } else {
                temp[y][x] = 0;
            }
        }
        print_progress((float)(y+1)/height);
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char v = temp[y][x];
            pixels[y][x].r = v;
            pixels[y][x].g = v;
            pixels[y][x].b = v;
        }
    }

    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);

    printf("\n");
}
