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

    RGBA temp[height][width];

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
        
    }

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
        printf("[5] Convert to Grayscale\n");
        printf("[6] Apply Blur\n");
        printf("[7] Status\n");
        printf("[8] Boost Mode\n");
        printf("[9] Quit\n");
        printf("==================================\n");

        int choice;

        printf("Enter: ");

        scanf("%d", &choice);

        if (choice == 4)
        {
            printf("==========Edge Detection==========\n");
            printf("[1] Sobel Operator\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1)
            {
                convertToGrayscale(pixels, width, height);
                gaussianBlur(pixels, width, height, 1.0);
            }
            
            
        }
        else if (choice == 5)
        {
            convertToGrayscale(pixels, width, height);
            writeBmp(argv[2], pixels, width, height);
            printf("\nGrayscale Conversion Complete\n");
        }
        else if (choice == 6)
        {
            printf("==============Blur================\n");
            printf("[1] Gaussian\n");
            printf("[?] Back\n");
            printf("==================================\n");

            int subChoice;
            printf("Enter: ");
            scanf("%d", &subChoice);

            if (subChoice == 1)
            {
                gaussianBlur(pixels, width, height, 1.0);
                writeBmp(argv[2], pixels, width, height);
            }
        }
        
        if (choice == 7)
        {
            printf("============BMP Info===============\n");
            printf("Input      : %s\n", argv[1]);
            printf("Width      : %d\n", width);
            printf("Height     : %d\n", height);
            printf("Bit Count  : %d\n", bpp);
            printf("Output     : %s\n", argv[2]);
            printf("Boost Mode : [Lorem ipsum]\n");
            
        }
        else if (choice == 9)
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
