#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/bmp.h"
#include "../include/scaling.h"
#include "../include/denoise.h"
#include "../include/sharpen.h"
#include "../include/edge.h"
#include "../include/utils.h"

volatile bool inputActive = false;

void *usage(void *arg) {
    while (1) {
        if (!inputActive) {
            printf("\r                                                               Work in progress");
            fflush(stdout);
        }
        usleep(500000);
    }
}

int main(int argc, char *argv[]) {

    pthread_t thread;

    if (argc < 3) 
    {
        printf("Usage: %s <input.bmp> <output.bmp>\n", argv[0]);
        return 1;
    }

    pthread_create(&thread, NULL, usage, NULL);


    bool multithreading = false;

    int width, height, bpp;
    RGBA **pixels = readBmp(argv[1], &width, &height, &bpp);

    RGBA **temp = copyImage(pixels, width, height);
    temp = bilinearInterpolation(temp, width, height, 96, 48);

    struct timespec start, end;

    FILE *fp = fopen("Log.csv", "r");

    if (fp == NULL) 
    {
        fp = fopen("Log.csv", "a");
        fprintf(fp, "Operation,Time Taken (s)\n");
    } 
    else 
    {
        fclose(fp);
        fp = fopen("Log.csv", "a");
    }    

    printf(" ____________________________________________________________________________________________________\n");
    printf("|                                                                                                    |\n");
    printf("|            ==========                 \\ \\    / /                                                   |\n");
    printf("|           / /       /   ==========     \\ \\  / /             / /========       / /                  |\n");
    printf("|          / /       /       / /          \\ \\/ /             / /               / /                   |\n");
    printf("|         / /=======/       / /            \\ \\/             / /               / /                    |\n");
    printf("|        / /               / /              \\ \\            / /========       / /                     |\n");
    printf("|       / /               / /              / \\ \\          / /               / /                      |\n");
    printf("|      / /               / /              / / \\ \\        / /               / /                       |\n");
    printf("|     / /             =========          / /   \\ \\      / /========       / /===============         |\n");
    printf("|                                       / /     \\ \\                                                  |\n");
    printf("|                                                                                                    |\n");
    printf("|____________________________________________________________________________________________________|\n\n");

    while (1) {
        printf("==============MENU================\n");
        printf("[1] Scaling\n");
        printf("[2] Denoise\n");
        printf("[3] Sharpening\n");
        printf("[4] Edge Detection\n");
        printf("[5] Utilities\n");
        printf("[6] Status\n");
        printf("[7] Performance Enhancements\n");
        printf("[8] Quit\n");
        printf("==================================\n");
        fflush(fp);

        int choice;
        printf("Enter: ");
        inputActive = true;
        scanf("%d", &choice);
        inputActive = false;

        if (choice == 1)
        {
            int radius;
            int newWidth, newHeight;
            printf("==========Upscaling/Downscaling==========\n");
            printf("[1] Bilinear Interpolation\n");
            printf("[2] Lanczos\n");
            printf("[3] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            inputActive = true;
            scanf("%d", &subChoice);
            inputActive = false;
            if (subChoice == 1)
            {
                inputActive = true;
                printf("Current Resolution: %dx%d\n", width, height);
                printf("Enter new width: ");
                scanf("%d", &newWidth);
                printf("Enter new height: ");
                scanf("%d", &newHeight);
                inputActive = false;

                if (multithreading == false)
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    pixels = bilinearInterpolation(pixels, width, height, newWidth, newHeight);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                else
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    pixels = multithreadedBilinearInterpolation(pixels, width, height, newWidth, newHeight);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                
                width = newWidth;
                height = newHeight;
                double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
                printf("Time taken for bilinear interpolation: %lf seconds\n", elapsed);

                fprintf(fp, "Bilinear Interpolation,%lf\n", elapsed);

                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 2)
            {
                printf("Current Resolution: %dx%d\n", width, height);
                printf("Enter new width: ");
                scanf("%d", &newWidth);
                printf("Enter new height: ");
                scanf("%d", &newHeight);

                printf("Enter the kernel radius (3 recommended or higher for better quality): ");
                scanf("%d", &radius);

                if (multithreading == false)
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    pixels = lanczosInterpolation(pixels, width, height, newWidth, newHeight, radius);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                else
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    pixels = multithreadedLanczos(pixels, width, height, newWidth, newHeight, radius);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                

                width = newWidth;
                height = newHeight;
                double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
                printf("Time taken for lanczos interpolation: %lf seconds\n", elapsed);

                fprintf(fp, "Lanczos Interpolation,%lf\n", elapsed);

                writeBmp(argv[2], pixels, width, height);
                
            }

        }
        else if (choice == 2)
        {
            printf("==========Denoising==========\n");
            printf("[1] Median Filter\n");
            printf("[2] Non Local Means\n");
            printf("[3] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1) {
                printf("Please enter the kernel size: ");
                int kernelSize;
                scanf("%d", &kernelSize);

                if (multithreading == false)
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    medianFilter(pixels, width, height, kernelSize);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                else
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    multithreadedMedian(pixels, width, height, kernelSize);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                
                double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
                printf("Time taken for median filter: %lf seconds\n", elapsed);
                fprintf(fp, "Median Filter,%lf\n", elapsed);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 2)
            {
                int searchRadius, patchRadius;

                printf("Enter the search radius (Recommended 10): ");
                scanf("%d", &searchRadius);
                printf("Enter the patch radius (Recommended 2): ");
                scanf("%d", &patchRadius);

                if (multithreading == false)
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    nonLocalMeans(pixels, width, height, searchRadius, patchRadius);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                else
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    multithreadedNLM(pixels, width, height, searchRadius, patchRadius);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }

                double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
                printf("Time taken for non-local means: %lf seconds\n", elapsed);
                fprintf(fp, "Non-Local Means,%lf\n", elapsed);
                writeBmp(argv[2], pixels, width, height);
            }
            
        }
        else if (choice == 3)
        {
            printf("==========Sharpening==========\n");
            printf("[1] Laplacian Filter\n");
            printf("[2] Unsharp Mask\n");
            printf("[3] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1)
            {
                printf("Enter amount (suggested 1.5): ");
                float amount;
                scanf("%f", &amount);

                if (multithreading == false)
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    laplacianFilter(pixels, width, height, amount);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                else
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    multithreadedLaplacian(pixels, width, height, amount);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                
                double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
                printf("Time taken for Laplacian Filter: %lf seconds\n", elapsed);
                fprintf(fp, "Laplacian Filter,%lf\n", elapsed);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 2)
            {
                printf("Enter sigma (suggested 1): ");
                int sigma;
                scanf("%d", &sigma);
                printf("Enter amount (suggested 1.5): ");
                float amount;
                scanf("%f", &amount);

                if (multithreading == false)
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    unsharpMask(pixels, width, height, sigma, amount);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                else
                {
                    clock_gettime(CLOCK_MONOTONIC, &start);
                    multithreadedUnsharpMask(pixels, width, height, sigma, amount);
                    clock_gettime(CLOCK_MONOTONIC, &end);
                }
                
                double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
                printf("Time taken for unsharp mask: %lf seconds\n", elapsed);
                fprintf(fp, "Unsharp Mask,%lf\n", elapsed);
                writeBmp(argv[2], pixels, width, height);
            }
            
        }
        else if (choice == 4) {
            
            printf("Detecting Edges using Sobel Operator...\n");

            if (multithreading == false)
            {
                clock_gettime(CLOCK_MONOTONIC, &start);
                convertToGrayscale(pixels, width, height);
                gaussianBlur(pixels, width, height, 1);
                sobelOperator(pixels, width, height);
                clock_gettime(CLOCK_MONOTONIC, &end);
            }
            else
            {
                clock_gettime(CLOCK_MONOTONIC, &start);
                multithreadedGrayscaling(pixels, width, height);
                multithreadedGaussian(pixels, width, height, 1);
                multithreadedSobel(pixels, width, height);
                clock_gettime(CLOCK_MONOTONIC, &end);               
            }
            
            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            printf("Time taken for Sobel Operator: %lf seconds\n", elapsed);
            fprintf(fp, "Sobel Operator,%lf\n", elapsed);
            writeBmp(argv[2], pixels, width, height);
            
        }
        else if (choice == 5) {
            printf("============Utilites==============\n");
            printf("[1] Grayscale Conversion\n");
            printf("[2] Apply Blur\n");
            printf("[3] Transpose/Rotate[Crashes]\n");
            printf("[4] Negative[Works]\n");
            printf("[5] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice > 5) {
                printf("Enter a Valid choice\n");
            }

            if (subChoice == 1) {
                if (multithreading == false)
                {
                    convertToGrayscale(pixels, width, height);
                }
                else
                {
                    multithreadedGrayscaling(pixels, width, height);
                }            
                
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 2) {

                if (multithreading == false)
                {
                    gaussianBlur(pixels, width, height, 1);
                }
                else
                {
                    multithreadedGaussian(pixels, width, height, 1);
                }
                
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 3) {
                transpose(pixels, width, height);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 4) {
                negative(pixels, width, height);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 5) {
                continue;
            }
        }
        else if (choice == 6) {
            printf("============BMP Info===============\n");
            convertToAscii(temp, 96, 48);      
            printf("Input      : %s\n", argv[1]);
            printf("Width      : %d\n", width);
            printf("Height     : %d\n", height);
            printf("Bit Count  : %d\n", bpp);
            printf("Output     : %s\n", argv[2]);
            printf("Boost Mode : [Lorem ipsum]\n");
        }
        else if (choice == 7)
        {
            printf("============BOOST MODE===============\n");
            printf("WARNING! WARNING! WARNING!\n");
            printf("Multithreading has been enabled\n");
            printf("==================================\n");

            if (multithreading == false)
            {
                multithreading = true;
            }
            else
            {
                multithreading = false;
            }
            
            
        }
        else if (choice == 8) {
            printf("==================================\n");
            break;
        }
    }

    for (int i = 0; i < height; i++) {
       free(pixels[i]);
    }
    free(pixels);

    return 0;
}
