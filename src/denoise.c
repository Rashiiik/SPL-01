#include "../include/denoise.h"
#include "../include/utils.h"
#include "../include/enum.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define NUM_THREADS 8

void *gaussianWorkerHorizontal(void *arg) {

    ThreadData *data = (ThreadData*) arg;

    for (int y = data->startRow; y < data->endRow; y++) {
        for (int x = 0; x < data->width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < data->kernelSize; i++) {
                int dx = i - data->center;
                int nx = x + dx;

                if (nx < 0) nx = 0;
                if (nx >= data->width) nx = data->width-1;
                
                sumR = sumR + data->kernel[i]*data->pixels[y][nx].r;
                sumG = sumG + data->kernel[i]*data->pixels[y][nx].g;
                sumB = sumB + data->kernel[i]*data->pixels[y][nx].b;
            }

            data->temp[y][x].r = sumR;
            data->temp[y][x].g = sumG;
            data->temp[y][x].b = sumB;
        }
    }

    
    return NULL;

}

void *gaussianWorkerVertical(void *arg) {

    ThreadData *data = (ThreadData*) arg;

    for (int y = data->startRow; y < data->endRow; y++) {
        for (int x = 0; x < data->width; x++) {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < data->kernelSize; i++) {
                int dy = i - data->center;
                int ny = y + dy;

                if (ny < 0) ny = 0;
                if (ny >= data->height) ny = data->height-1;
                
                sumR = sumR + data->kernel[i]*data->temp[ny][x].r;
                sumG = sumG + data->kernel[i]*data->temp[ny][x].g;
                sumB = sumB + data->kernel[i]*data->temp[ny][x].b;
            }

            data->pixels[y][x].r = round(sumR);
            data->pixels[y][x].g = round(sumG);
            data->pixels[y][x].b = round(sumB);
        }
    }

    return NULL;
}

void *medianWorker(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    for (int y = data->startRow; y < data->endRow; y++)
    {
        for (int x = 0; x < data->width; x++)
        {
            int rValues[data->kernelSize * data->kernelSize];
            int gValues[data->kernelSize * data->kernelSize];
            int bValues[data->kernelSize * data->kernelSize];
            int count = 0;
            
            for (int ky = -data->center; ky <= data->center; ky++)
            {
                for (int kx = -data->center; kx <= data->center; kx++)
                {
                    int ny = y + ky;
                    int nx = x + kx;

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

                    rValues[count] = data->pixels[ny][nx].r;
                    gValues[count] = data->pixels[ny][nx].g;
                    bValues[count] = data->pixels[ny][nx].b;
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
            data->temp[y][x].r = rValues[medianIndex];
            data->temp[y][x].g = gValues[medianIndex];
            data->temp[y][x].b = bValues[medianIndex];
        }
        
    }

    return NULL;
}

void *nlmWorker(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    int patchSize = (2 * data->patchRadius + 1) * (2 * data->patchRadius + 1);

    float h = 0.175f;

    for (int y = data->startRow; y < data->endRow; y++)
    {
        for (int x = 0; x < data->width; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;

            float weightSum = 0;

            for (int j = -data->searchRadius; j <= data->searchRadius; j++)
            {
                for (int i = -data->searchRadius; i <= data->searchRadius; i++)
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

                    float distance = 0;

                    for (int py = -data->patchRadius; py <= data->patchRadius; py++)
                    {
                        for (int px = -data->patchRadius; px <= data->patchRadius; px++)
                        {
                            int p1x = x + px;
                            int p1y = y + py;
                            int p2x = nx + px;
                            int p2y = ny + py;

                            if (p1x >= 0 && p1x < data->width && p1y >= 0 && p1y < data->height && p2x >= 0 && p2x < data->width && p2y >= 0 && p2y < data->height)
                            {
                                float dr = (data->pixels[p1y][p1x].r - data->pixels[p2y][p2x].r) / 255.0f;
                                float dg = (data->pixels[p1y][p1x].g - data->pixels[p2y][p2x].g) / 255.0f;
                                float db = (data->pixels[p1y][p1x].b - data->pixels[p2y][p2x].b) / 255.0f;
                                distance += dr*dr + dg*dg + db*db;
                            }
                        }
                    }

                    distance /= patchSize;  

                    float weight = expf(-distance / (h*h));
                    sumR += weight * data->pixels[ny][nx].r;
                    sumG += weight * data->pixels[ny][nx].g;
                    sumB += weight * data->pixels[ny][nx].b;
                    weightSum += weight;

                }
                
            }

            data->temp[y][x].r = sumR / weightSum;
            data->temp[y][x].g = sumG / weightSum;
            data->temp[y][x].b = sumB / weightSum;
            
        }
        
    }

    return NULL;
}

void gaussianBlur(RGBA **pixels, int width, int height, int sigma) {

    int kernelSize = ceil(6*sigma + 1);

    if (kernelSize % 2 == 0) {
        kernelSize++;
    }

    float *kernel = malloc(kernelSize * sizeof(float));
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
    }

    for (int i = 0; i < height; i++) {
        free(temp[i]);
    }
    free(temp);
    free(kernel);
}

void multithreadedGaussian(RGBA **pixels, int width, int height, int sigma) {

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    RGBA *tempData = malloc(width * height * sizeof(RGBA));
    
    RGBA **temp = malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        temp[i] = tempData + i * width;
    }

    int kernelSize = ceil(6*sigma + 1);

    if (kernelSize % 2 == 0) {
        kernelSize++;
    }

    float kernel[kernelSize];

    int center = kernelSize/2;
    float sum = 0;

    for (int i = 0; i < kernelSize; i++) {
        int x = i - center;
        kernel[i] = exp(-(x*x)/(2*sigma*sigma));
        sum = sum + kernel[i];
    }

    for (int i = 0; i < kernelSize; i++) {
        kernel[i] = kernel[i]/sum;
    }

    int chunks = height/NUM_THREADS;

    printf("Applying Gaussian Blur...\n");

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].width = width;
        data[i].height = height;
        data[i].sigma = sigma;
        data[i].center = center;
        data[i].kernelSize = kernelSize;
        data[i].kernel = kernel;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, gaussianWorkerHorizontal, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].width = width;
        data[i].height = height;
        data[i].sigma = sigma;
        data[i].center = center;
        data[i].kernelSize = kernelSize;
        data[i].kernel = kernel;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, gaussianWorkerVertical, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    free(tempData);
    free(temp);
    
}

void medianFilter(RGBA **pixels, int width, int height, int kernelSize) {

    if (kernelSize%2== 0)
    {
        kernelSize++;
    }
    
    RGBA **temp = malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        temp[i] = malloc(width * sizeof(RGBA));
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
            temp[y][x].r = rValues[medianIndex];
            temp[y][x].g = gValues[medianIndex];
            temp[y][x].b = bValues[medianIndex];
        }
        print_progress((float)(y+1)/height);
        printf("\n");
        
    }

    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            pixels[y][x] = temp[y][x];
        }
    }

    for (int i = 0; i < height; i++) 
    {
        free(temp[i]);
    }
    free(temp);
    
}

void multithreadedMedian(RGBA **pixels, int width, int height, int kernelSize) {

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    RGBA *tempData = malloc(width * height * sizeof(RGBA));
    
    RGBA **temp = malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        temp[i] = tempData + i * width;
    }

    if (kernelSize%2== 0)
    {
        kernelSize++;
    }

    int center = kernelSize / 2;

    int chunks = height/NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].width = width;
        data[i].center = center;
        data[i].height = height;
        data[i].kernelSize = kernelSize;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, medianWorker, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            pixels[y][x] = temp[y][x];
        }
    }

    free(tempData);
    free(temp);
}

void nonLocalMeans(RGBA **pixels, int width, int height, int searchRadius, int patchRadius) {

    RGBA **temp = malloc(height * sizeof(RGBA *));    

    for (int i = 0; i < height; i++)
    {
        temp[i] = malloc(width * sizeof(RGBA));
    }

    int patchSize = (2 * patchRadius + 1) * (2 * patchRadius + 1);

    float h = 0.175f;  
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;

            float weightSum = 0;

            for (int j = -searchRadius; j <= searchRadius; j++)
            {
                for (int i = -searchRadius; i <= searchRadius; i++)
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

                    float distance = 0;

                    for (int py = -patchRadius; py <= patchRadius; py++)
                    {
                        for (int px = -patchRadius; px <= patchRadius; px++)
                        {
                            int p1x = x + px;
                            int p1y = y + py;
                            int p2x = nx + px;
                            int p2y = ny + py;

                            if (p1x >= 0 && p1x < width && p1y >= 0 && p1y < height && p2x >= 0 && p2x < width && p2y >= 0 && p2y < height)
                            {
                                float dr = (pixels[p1y][p1x].r - pixels[p2y][p2x].r) / 255.0f;
                                float dg = (pixels[p1y][p1x].g - pixels[p2y][p2x].g) / 255.0f;
                                float db = (pixels[p1y][p1x].b - pixels[p2y][p2x].b) / 255.0f;
                                distance += dr*dr + dg*dg + db*db;
                            }
                        }
                    }

                    distance /= patchSize;  

                    float weight = expf(-distance / (h*h));
                    sumR += weight * pixels[ny][nx].r;
                    sumG += weight * pixels[ny][nx].g;
                    sumB += weight * pixels[ny][nx].b;
                    weightSum += weight;

                }
                
            }

            temp[y][x].r = sumR / weightSum;
            temp[y][x].g = sumG / weightSum;
            temp[y][x].b = sumB / weightSum;
            
        }
        
    }

    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            pixels[y][x] = temp[y][x];
        }
    }

    for (int i = 0; i < height; i++) 
    {
        free(temp[i]);
    }
    free(temp);
    
}

void multithreadedNLM(RGBA **pixels, int width, int height, int searchRadius, int patchRadius) {

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    RGBA *tempData = malloc(width * height * sizeof(RGBA));
    
    RGBA **temp = malloc(height * sizeof(RGBA *));
    for (int i = 0; i < height; i++) {
        temp[i] = tempData + i * width;
    }
    
    int chunks = height/NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].width = width;
        data[i].height = height;
        data[i].searchRadius = searchRadius;
        data[i].patchRadius = patchRadius;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, nlmWorker, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            pixels[y][x] = temp[y][x];
        }
    }

    free(tempData);
    free(temp);
}
