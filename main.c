#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include "filters.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input.bmp> <output.bmp>\n", argv[0]);
        return 1;
    }

    int width, height, bpp;
    RGBA **pixels = readBmp(argv[1], &width, &height, &bpp);

    if (!pixels) {
        return 1;
    }

    printf(" ___________________________________________________________________\n");
    printf("|...................................................................|\n");
    printf("|.......#######...#.......#........#.......#####...#######..........|\n");
    printf("|..........#......#.#...#.#......#...#....#........#................|\n");
    printf("|..........#......#..#.#..#.....#######...#..###...#######..........|\n");
    printf("|..........#......#...#...#.....#.....#...#....#...#................|\n");
    printf("|.......#######...#.......#.....#.....#....#####...#######..........|\n");
    printf("|...................................................................|\n");
    printf("|___________________________________________________________________|\n\n");

    while (1) {
        printf("==============MENU================\n");
        printf("[1] Scaling\n");
        printf("[2] Denoise\n");
        printf("[3] Sharpening\n");
        printf("[4] Edge Detection\n");
        printf("[5] Utilities\n");
        printf("[6] Status\n");
        printf("[7] Boost Mode\n");
        printf("[8] Quit\n");
        printf("==================================\n");

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
                printf("Enter new width: ");
                scanf("%d", &newWidth);
                printf("Enter new height: ");
                scanf("%d", &newHeight);
                
                pixels = bilinearInterpolation(pixels, width, height, newWidth, newHeight);
                width = newWidth;
                height = newHeight;

                writeBmp(argv[2], pixels, width, height);
            }

        }
        else if (choice == 2)
        {
            printf("==========Denoising==========\n");
            printf("[1] Gaussian Blur\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1) {
                gaussianBlur(pixels, width, height, 1);
                writeBmp(argv[2], pixels, width, height);
            }
        }
        
        if (choice == 4) {
            printf("==========Edge Detection==========\n");
            printf("[1] Sobel Operator[Needs work]\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1) {
                convertToGrayscale(pixels, width, height);
                gaussianBlur(pixels, width, height, 1);
                sobelOperator(pixels, width, height, 100);
                writeBmp(argv[2], pixels, width, height);
            }
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
            printf("Input      : %s\n", argv[1]);
            printf("Width      : %d\n", width);
            printf("Height     : %d\n", height);
            printf("Bit Count  : %d\n", bpp);
            printf("Output     : %s\n", argv[2]);
            printf("Boost Mode : [Lorem ipsum]\n");
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