#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
    unsigned char a; // For storing the Alpha channel(opacity) of 32 bit 

} RGBA;

RGBA **read_bmp(const char *filename, int *width, int *height, int *bpp) {

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

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <input.bmp> <output.bmp>\n", argv[0]);
        return 1;
    }

    int width, height, bpp;

    RGBA **pixels = read_bmp(argv[1], &width, &height, &bpp);

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
        printf("----------------------------------\n");
        printf("[1] Upscale\n");
        printf("[2] Denoise\n");
        printf("[3] Sharpening\n");
        printf("[4] Edge Detection\n");
        printf("[5] Image Status\n");
        printf("[6] Boost Mode : \n");
        printf("[7] Quit\n");
        printf("----------------------------------\n");

        int choice;

        printf("Enter: ");

        scanf("%d", &choice);

        printf("----------------------------------\n");

        if (choice == 5)
        {
            printf("BMP Info: %s\n", argv[1]);
            printf("Width: %d\n", width);
            printf("Height: %d\n", height);
            printf("Bit Count: %d\n", bpp);
            printf("Image will be written to: %s\n", argv[2]);
        }
        else if (choice == 7)
        {
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
