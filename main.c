#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#pragma pack(push, 1)

typedef struct {

    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;

} BITMAPFILEHEADER;

typedef struct {

    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;

} BITMAPINFOHEADER;

#pragma pack(pop)

typedef struct {

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a; 

} RGBA;

RGBA **readBmp(const char *filename, int *width, int *height, int *bpp) {

    FILE *fp = fopen(filename, "rb");

    if (!fp) {
        perror("Cannot open file");
        return NULL;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    if (fileHeader.bfType != 0x4D42) 
    {
        printf("Not a BMP file.\n");
        fclose(fp);
        return NULL;
    }

    *width = infoHeader.biWidth;
    *height = infoHeader.biHeight;
    *bpp = infoHeader.biBitCount;

    if (*bpp != 24 && *bpp != 32) 
    {
        printf("Only 24-bit and 32-bit BMP supported.\n");
        fclose(fp);
        return NULL;
    }

    RGBA **pixels = (RGBA **)malloc(*height * sizeof(RGBA *));

    for (int i = 0; i < *height; i++)
    {
        pixels[i] = (RGBA *)malloc(*width * sizeof(RGBA));
    }

    int row_padded = (*bpp == 24) ? ((*width * 3 + 3) & (~3)) : (*width * 4);

    unsigned char *row = (unsigned char *)malloc(row_padded);

    fseek(fp, fileHeader.bfOffBits, SEEK_SET);

    for (int i = 0; i < *height; i++) 
    {
        fread(row, sizeof(unsigned char), row_padded, fp);

        for (int j = 0; j < *width; j++) 
        {
            if (*bpp == 24) 
            {
                pixels[*height - 1 - i][j].b = row[j * 3];
                pixels[*height - 1 - i][j].g = row[j * 3 + 1];
                pixels[*height - 1 - i][j].r = row[j * 3 + 2];
                pixels[*height - 1 - i][j].a = 0;
            } 
            else 
            {
                pixels[*height - 1 - i][j].b = row[j * 4];
                pixels[*height - 1 - i][j].g = row[j * 4 + 1];
                pixels[*height - 1 - i][j].r = row[j * 4 + 2];
                pixels[*height - 1 - i][j].a = row[j * 4 + 3];
            }
        }
    }

    free(row);
    fclose(fp);
    return pixels;
}

int writeBmp(const char *filename, RGBA **pixels, int width, int height) {

    FILE *fp = fopen(filename, "wb");

    if (!fp) 
    {
        printf("Cannot create output file");
        return 0;
    }

    int row_padded = (width * 3 + 3) & (~3);
    int pixel_data_size = row_padded * height;

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    fileHeader.bfType = 0x4D42; 
    fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pixel_data_size;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;        
    infoHeader.biCompression = 0;      
    infoHeader.biSizeImage = pixel_data_size;
    infoHeader.biXPelsPerMeter = 2835; 
    infoHeader.biYPelsPerMeter = 2835; 
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

    unsigned char *row = (unsigned char *)malloc(row_padded);

    for (int y = 0; y < height; y++)
    {
        int src_y = height - 1 - y; 

        for (int x = 0; x < width; x++)
        {
            row[x * 3 + 0] = pixels[src_y][x].b;
            row[x * 3 + 1] = pixels[src_y][x].g;
            row[x * 3 + 2] = pixels[src_y][x].r;
        }

        for (int p = width * 3; p < row_padded; p++)
        {
            row[p] = 0;
        }

        fwrite(row, 1, row_padded, fp);
    }

    free(row);

    fclose(fp);

    printf("Image written to output: %s\n", filename);

    return 1;
}

void print_progress(float progress) {

    int barWidth = 32; 

    int pos = (int)(progress * barWidth);

    printf("\r["); 

    for (int i = 0; i < barWidth; i++)
    {
        if (i < pos) 
        {
            printf("#");
        }
        else 
        {
            printf(".");
        }

    }

    printf("] %6.2f%%", progress * 100.0f);

    fflush(stdout);
}

void convertToGrayscale(RGBA **pixels, int width, int height) {

    printf("Converting to Grayscale\n");

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char r = pixels[i][j].r;
            unsigned char g = pixels[i][j].g;
            unsigned char b = pixels[i][j].b;

            unsigned char gray = (unsigned char)(0.299*r + 0.587*g + 0.114*b);

            pixels[i][j].r = gray;
            pixels[i][j].g = gray;
            pixels[i][j].b = gray;
        }

        print_progress((float)(i + 1) / height);
        
    }

    print_progress(1.0f);
    
    printf("\n");
    
}

void gaussianBlur(RGBA **pixels, int width, int height, int sigma) {

    int kernelSize = ceil(6*sigma + 1);

    if (kernelSize % 2 == 0)
    {
        kernelSize++;
    }

    float kernel[kernelSize];

    int center = kernelSize/2;

    float sum = 0;

    RGBA **temp = malloc(height * sizeof(RGBA *));

    for (int i = 0; i < height; i++) 
    {
        temp[i] = malloc(width * sizeof(RGBA));
    }

    for (int i = 0; i < kernelSize; i++)
    {
        int x = i - center;

        kernel[i] = exp(-(x*x)/(2*sigma*sigma));

        sum = sum + kernel[i];
    }

    for (int i = 0; i < kernelSize; i++)
    {
        kernel[i] = kernel[i]/sum;
    }

    printf("Applying Gaussian Blur...\n");

    printf("Horizontal Pass...\n");
    
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < kernelSize; i++)
            {
                int dx = i - center;
                int nx = x + dx;

                if (nx < 0)
                {
                    nx = 0;
                }

                if (nx >= width)
                {
                    nx = width-1;
                }
                
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

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float sumR = 0, sumG = 0, sumB = 0;

            for (int i = 0; i < kernelSize; i++)
            {
                int dy = i - center;
                int ny = y + dy;

                if (ny < 0)
                {
                    ny = 0;
                }

                if (ny >= height)
                {
                    ny = height-1;
                }
                
                sumR = sumR + kernel[i]*temp[ny][x].r;
                sumG = sumG + kernel[i]*temp[ny][x].g;
                sumB = sumB + kernel[i]*temp[ny][x].b;

            }

            pixels[y][x].r = round(sumR);
            pixels[y][x].g = round(sumG);
            pixels[y][x].b = round(sumB);
            
        }
        
        print_progress((float)(y+1)/height);
        
    }

    printf("\n");

    for (int i = 0; i < height; i++) 
    {
        free(temp[i]);
    }
    
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
        temp[i] = calloc(width, sizeof(unsigned char));
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float gx = 0, gy = 0;

            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
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
                    
                    float intensity = pixels[ny][nx].r;

                    gx += intensity * Gx[ky+1][kx+1];
                    gy += intensity * Gy[ky+1][kx+1];
                    
                }
                
            }

            float mag = sqrtf(gx*gx + gy*gy);
            
            if (mag > threshold)
            {
                temp[y][x] = 255;
            }
            else
            {
                temp[y][x] = 0;
            }
        
        }

        print_progress((float)(y+1)/height);
        
    }

    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            unsigned char v = temp[y][x];
            pixels[y][x].r = v;
            pixels[y][x].g = v;
            pixels[y][x].b = v;
        }
    }

    for (int i = 0; i < height; i++) 
    {
        free(temp[i]);
    }
    
    free(temp);
    
}

void transpose(RGBA **pixels, int width, int height) {

    RGBA **temp = malloc(width*(sizeof(RGBA *)));

    for (int i = 0; i < width; i++)
    {
        temp[i] = malloc(height*(sizeof(RGBA)));
    }

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            temp[x][y] = pixels[y][x];                 // Under construction
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

void negative(RGBA **pixels, int width, int height) {

    printf("Converting to negative....\n");

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            pixels[y][x].r = 255 - pixels[y][x].r;
            pixels[y][x].g = 255 - pixels[y][x].g;
            pixels[y][x].b = 255 - pixels[y][x].b;
        }
        
        print_progress((float)(y+1)/height);
    }

    printf("\n");
    
}

void convertToAscii(RGBA **pixels, int width, int height) {

    const char asciiChar[] = "@%%#*+=-:. ";

    int len = strlen(asciiChar);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)                  // Need to downscale first else it doesnt fit
        {
            int index = pixels[y][x].r * (len-1)/255;
            printf("%c", asciiChar[index]);
        }
        printf("\n");
    }
    
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <input.bmp> <output.bmp>\n", argv[0]);
        return 1;
    }

    int width, height, bpp;

    RGBA **pixels = readBmp(argv[1], &width, &height, &bpp);

    if (!pixels)
    {
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

    while (1)
    {
        printf("==============MENU================\n");
        printf("[1] Upscale\n");
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

        if (choice == 4)
        {
            printf("==========Edge Detection==========\n");
            printf("[1] Sobel Operator[Needs work]\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1)
            {
                convertToGrayscale(pixels, width, height);
                gaussianBlur(pixels, width, height, 1.0);
                sobelOperator(pixels, width, height, 100);
                writeBmp(argv[2], pixels, width, height);
            }
            
            
        }
        else if (choice == 5)
        {
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

            if (subChoice > 5)
            {
                printf("Enter a Valid choice\n");
            }
            

            if (subChoice == 1)
            {
                convertToGrayscale(pixels, width, height);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 2)
            {
                gaussianBlur(pixels, width, height, 1);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 3)
            {
                transpose(pixels, width, height);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 4)
            {
                negative(pixels, width, height);
                writeBmp(argv[2], pixels, width, height);
            }
            else if (subChoice == 5)
            {
                continue;
            }
            
        }
        else if (choice == 6)
        {
            printf("============BMP Info===============\n");
            printf("Input      : %s\n", argv[1]);
            printf("Width      : %d\n", width);
            printf("Height     : %d\n", height);
            printf("Bit Count  : %d\n", bpp);
            printf("Output     : %s\n", argv[2]);
            printf("Boost Mode : [Lorem ipsum]\n");
            
        }
        else if (choice == 8)
        {
            printf("==================================\n");
            break;
        }
        
    }

    for (int i = 0; i < height; i++)
    {
       free(pixels[i]);
    }

    free(pixels);

    return 0;
}
