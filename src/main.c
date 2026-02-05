#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "../include/bmp.h"
#include "../include/scaling.h"
#include "../include/denoise.h"
#include "../include/sharpen.h"
#include "../include/edge.h"
#include "../include/utils.h"


int main(int argc, char *argv[]) {

    if (argc < 3) 
    {
        printf("Usage: %s <input.bmp> <output.bmp>\n", argv[0]);
        return 1;
    }

    bool devMode = false;

    int width, height, bpp;
    RGBA **pixels = readBmp(argv[1], &width, &height, &bpp);

    RGBA **temp = copyImage(pixels, width, height);
    temp = bilinearInterpolation(temp, width, height, 96, 48, devMode);

    clock_t start, end;
    double cpu_time_used;

    FILE *fp = fopen("Log.csv", "r");

    if (fp == NULL) 
    {
        fp = fopen("Log.csv", "a");
        fprintf(fp, "Operation,Start Time,End Time,CPU Time Used (s)\n");
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
        printf("[7] Boost Mode\n");
        printf("[8] Developer Mode\n");
        printf("[9] Quit\n");
        printf("==================================\n");
        fflush(fp);

        int choice;
        printf("Enter: ");
        scanf("%d", &choice);

        if (choice == 1)
        {
            int newWidth, newHeight;
            printf("==========Upscaling/Downscaling==========\n");
            printf("[1] Bilinear Interpolation\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);
            if (subChoice == 1)
            {
                printf("Current Resolution: %dx%d\n", width, height);
                printf("Enter new width: ");
                scanf("%d", &newWidth);
                printf("Enter new height: ");
                scanf("%d", &newHeight);

                start = clock();
                pixels = bilinearInterpolation(pixels, width, height, newWidth, newHeight, devMode);
                width = newWidth;
                height = newHeight;
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Time taken for bilinear interpolation: %f seconds\n", cpu_time_used);

                fprintf(fp, "Bilinear Interpolation,%lf,%lf,%lf\n", (double)start/CLOCKS_PER_SEC, (double)end/CLOCKS_PER_SEC, cpu_time_used);


                writeBmp(argv[2], pixels, width, height);
            }

        }
        else if (choice == 2)
        {
            printf("==========Denoising==========\n");
            printf("[1] Median Filter\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1) {
                printf("Please enter the kernel size: ");
                int kernelSize;
                scanf("%d", &kernelSize);
                start = clock();
                medianFilter(pixels, width, height, kernelSize);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Time taken for median filter: %f seconds\n", cpu_time_used);
                fprintf(fp, "Median Filter,%lf,%lf,%lf\n", (double)start/CLOCKS_PER_SEC, (double)end/CLOCKS_PER_SEC, cpu_time_used);
                writeBmp(argv[2], pixels, width, height);
            }
        }
        else if (choice == 3)
        {
            printf("==========Sharpening==========\n");
            printf("[1] Unsharp Mask\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1)
            {
                printf("Enter sigma (suggested 1): ");
                int sigma;
                scanf("%d", &sigma);
                printf("Enter amount (suggested 1.5): ");
                float amount;
                scanf("%f", &amount);
                start = clock();
                unsharpMask(pixels, width, height, sigma, amount);
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                printf("Time taken for unsharp mask: %f seconds\n", cpu_time_used);
                fprintf(fp, "Unsharp Mask,%lf,%lf,%lf\n", (double)start/CLOCKS_PER_SEC, (double)end/CLOCKS_PER_SEC, cpu_time_used);
                writeBmp(argv[2], pixels, width, height);
            }
            
        }
        else if (choice == 4) {
            
            printf("Detecting Edges using Sobel Operator...\n");
            start = clock();
            convertToGrayscale(pixels, width, height);
            gaussianBlur(pixels, width, height, 1);
            sobelOperator(pixels, width, height, 100);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Time taken for Sobel Operator: %f seconds\n", cpu_time_used);
            fprintf(fp, "Sobel Operator,%lf,%lf,%lf\n", (double)start/CLOCKS_PER_SEC, (double)end/CLOCKS_PER_SEC, cpu_time_used);
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
                convertToGrayscale(pixels, width, height);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 2) {
                gaussianBlur(pixels, width, height, 1);
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
        else if (choice == 8) {
            devMode = true;
            printf("Developer Mode Activated!\n");
        }
        else if (choice == 9) {
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
