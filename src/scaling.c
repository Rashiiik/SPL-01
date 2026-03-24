#include "../include/scaling.h"
#include "../include/utils.h"
#include "../include/enum.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <xmmintrin.h>

#define NUM_THREADS 8
#define PI 3.14159265358979323846

double sinc(double x) {

    if (x == 0)
    {
        return 1.0;
    }
    

    x = PI * x;

    return sin(x)/x;
}

double lanczos(double x, int a) {

    if (fabs(x) >= a)
    {
        return 0.0;
    }
    else
    {
        return sinc(x) * sinc(x/a);
    }
    
}

void *bilinear(void *arg) {

    ThreadData *data = (ThreadData*)arg;

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

void *lanczosWorkerHorizontal(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    for (int y = data->startRow; y < data->endRow; y++)
    {
        for (int x = 0; x < data->newWidth; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;

            float srcX = x * data->xRatio;
            

            int baseX = floor(srcX);
            

            float weightSum = 0;

            for (int i = -data->radius; i <= data->radius; i++)           
            {                                                 
                int sampleX = baseX + i;            
                              

                if (sampleX < 0) sampleX = 0;
                if (sampleX >= data->oldWidth) sampleX = data->oldWidth - 1;
                    

                float dx = srcX - sampleX;
                                  
                                                            
                float weight = lanczos(dx, data->radius);

                RGBA temp1 = data->pixels[y][sampleX];

                sumR += temp1.r * weight;
                sumG += temp1.g * weight;
                sumB += temp1.b * weight;

                weightSum += weight;

            }
                
            

            if (weightSum != 0.0) {
                data->temp[y][x].r = (unsigned char)fmin(fmax(sumR / weightSum + 0.5, 0.0), 255.0);
                data->temp[y][x].g = (unsigned char)fmin(fmax(sumG / weightSum + 0.5, 0.0), 255.0);
                data->temp[y][x].b = (unsigned char)fmin(fmax(sumB / weightSum + 0.5, 0.0), 255.0);
                data->temp[y][x].a = 255;
            } else {
                data->temp[y][x].r = data->temp[y][x].g = data->temp[y][x].b = 0;
                data->temp[y][x].a = 255;
            }

        }
        
    }

    return NULL;

}

void *lanczosWorkerVertical(void *arg) {

    ThreadData *data = (ThreadData*)arg;

    for (int y = data->startRow; y < data->endRow; y++)
    {
        float srcY = y * data->yRatio;

        for (int x = 0; x < data->newWidth; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;
            
            int baseY = floor(srcY);

            float weightSum = 0;

            for (int j = -data->radius; j <= data->radius; j++)             
            {                                                                         
                int sampleY = baseY + j;            
  
                if (sampleY < 0) sampleY = 0;
                if (sampleY >= data->oldHeight) sampleY = data->oldHeight - 1;

                    
                float dy = srcY - sampleY;              
                                                            
                float weight = lanczos(dy, data->radius);

                RGBA temp1 = data->pixels[sampleY][x];

                sumR += temp1.r * weight;
                sumG += temp1.g * weight;
                sumB += temp1.b * weight;

                weightSum += weight;     
            }

            if (weightSum != 0.0) {
                data->temp[y][x].r = (unsigned char)fmin(fmax(sumR / weightSum + 0.5, 0.0), 255.0);
                data->temp[y][x].g = (unsigned char)fmin(fmax(sumG / weightSum + 0.5, 0.0), 255.0);
                data->temp[y][x].b = (unsigned char)fmin(fmax(sumB / weightSum + 0.5, 0.0), 255.0);
                data->temp[y][x].a = 255;
            } else {
                data->temp[y][x].r = data->temp[y][x].g = data->temp[y][x].b = 0;
                data->temp[y][x].a = 255;
            }

        }
        
    }

    return NULL;

}

RGBA **bilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight) {

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

    }

    for (int i = 0; i < oldHeight; i++) 
    {
        free(pixels[i]);
    }
    free(pixels);

    return temp;
}

RGBA **multithreadedBilinearInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight) {

    RGBA *tempData = malloc(newHeight * newWidth * sizeof(RGBA));
    
    RGBA **temp = malloc(newHeight * sizeof(RGBA *));
    for (int i = 0; i < newHeight; i++) {
        temp[i] = tempData + i * newWidth;
    }

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

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

RGBA **lanczosHorizontal(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, int radius) {

    RGBA **temp = malloc(oldHeight * sizeof(RGBA *));
    for (int i = 0; i < oldHeight; i++) {
        temp[i] = malloc(newWidth * sizeof(RGBA));
    }

    float xRatio = (float)(oldWidth - 1) / (newWidth - 1);

    for (int y = 0; y < oldHeight; y++)
    {
        for (int x = 0; x < newWidth; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;

            float srcX = x * xRatio;

            int baseX = floor(srcX);
            float weightSum = 0;

            
            for (int i = -radius; i <= radius; i++)          
            {                                                 
                int sampleX = baseX + i;           
                if (sampleX < 0) sampleX = 0;
                if (sampleX >= oldWidth) sampleX = oldWidth - 1;

                float dx = srcX - sampleX;

                float weight = lanczos(dx, radius);

                RGBA temp1 = pixels[y][sampleX];

                sumR += temp1.r * weight;
                sumG += temp1.g * weight;
                sumB += temp1.b * weight;

                weightSum += weight;
            }            

            if (weightSum != 0.0) {
                temp[y][x].r = (unsigned char)fmin(fmax(sumR / weightSum + 0.5, 0.0), 255.0);
                temp[y][x].g = (unsigned char)fmin(fmax(sumG / weightSum + 0.5, 0.0), 255.0);
                temp[y][x].b = (unsigned char)fmin(fmax(sumB / weightSum + 0.5, 0.0), 255.0);
                temp[y][x].a = 255;
            } else {
                temp[y][x].r = temp[y][x].g = temp[y][x].b = 0;
                temp[y][x].a = 255;
            }

        }
        
    }

    return temp;

}

RGBA **lanczosVertical(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, int radius) {

    RGBA **temp = malloc(newHeight * sizeof(RGBA *));
    for (int i = 0; i < newHeight; i++) {
        temp[i] = malloc(newWidth * sizeof(RGBA));
    }

    float yRatio = (float)(oldHeight - 1) / (newHeight - 1);

    for (int y = 0; y < newHeight; y++)
    {
        float srcY = y * yRatio;
        
        for (int x = 0; x < newWidth; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;

            int baseY = floor(srcY);
            float weightSum = 0;

            
            for (int j = -radius; j <= radius; j++)          
            {                                                 
                int sampleY = baseY + j;           
                if (sampleY < 0) sampleY = 0;
                if (sampleY >= oldHeight) sampleY = oldHeight - 1;

                float dy = srcY - sampleY;

                float weight = lanczos(dy, radius);

                RGBA temp1 = pixels[sampleY][x];

                sumR += temp1.r * weight;
                sumG += temp1.g * weight;
                sumB += temp1.b * weight;

                weightSum += weight;
            }            

            if (weightSum != 0.0) {
                temp[y][x].r = (unsigned char)fmin(fmax(sumR / weightSum + 0.5, 0.0), 255.0);
                temp[y][x].g = (unsigned char)fmin(fmax(sumG / weightSum + 0.5, 0.0), 255.0);
                temp[y][x].b = (unsigned char)fmin(fmax(sumB / weightSum + 0.5, 0.0), 255.0);
                temp[y][x].a = 255;
            } else {
                temp[y][x].r = temp[y][x].g = temp[y][x].b = 0;
                temp[y][x].a = 255;
            }

        }
        
    }

    for (int i = 0; i < oldHeight; i++) 
    {
        free(pixels[i]);
    }
    free(pixels);

    return temp;

}

RGBA **lanczosInterpolation(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, int radius) {

    RGBA **temp = lanczosHorizontal(pixels, oldWidth, oldHeight, newWidth, newHeight, radius);
    RGBA **output = lanczosVertical(temp, oldWidth, oldHeight, newWidth, newHeight, radius);

    return output;

}

RGBA **multithreadedLanczos(RGBA **pixels, int oldWidth, int oldHeight, int newWidth, int newHeight, int radius) {

    RGBA *tempData = malloc(oldHeight * newWidth * sizeof(RGBA));
    
    RGBA **temp = malloc(oldHeight * sizeof(RGBA *));
    for (int i = 0; i < oldHeight; i++) {
        temp[i] = tempData + i * newWidth;
    }

    RGBA *outData = malloc(newHeight * newWidth * sizeof(RGBA));

    RGBA **output = malloc(newHeight * sizeof(RGBA*));
    for(int i=0;i<newHeight;i++) {
        output[i] = outData + i*newWidth;
    }

    pthread_t threads[NUM_THREADS];
    ThreadData data[NUM_THREADS];

    int chunks = oldHeight/NUM_THREADS;

    float xRatio = (float)(oldWidth - 1) / (newWidth - 1);
    float yRatio = (float)(oldHeight - 1) / (newHeight - 1);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = pixels;
        data[i].temp = temp;
        data[i].oldHeight = oldHeight;
        data[i].oldWidth = oldWidth;
        data[i].radius = radius;
        data[i].xRatio = xRatio;
        data[i].yRatio = yRatio;
        data[i].newHeight = newHeight;
        data[i].newWidth = newWidth;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, lanczosWorkerHorizontal, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    chunks = newHeight/NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        data[i].pixels = temp;
        data[i].temp = output;
        data[i].oldHeight = oldHeight;
        data[i].oldWidth = oldWidth;
        data[i].radius = radius;
        data[i].xRatio = xRatio;
        data[i].yRatio = yRatio;
        data[i].newHeight = newHeight;
        data[i].newWidth = newWidth;
        data[i].startRow = i*chunks;
        data[i].endRow = (i+1)*chunks;

        pthread_create(&threads[i], NULL, lanczosWorkerVertical, &data[i]);
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

    free(tempData);
    free(temp);

    return output;

}
