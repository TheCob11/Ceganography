#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <FreeImage\FreeImage.h>

void encodeByte(BYTE **bits, BYTE data)
{
    BYTE *curr = *bits;
    curr[0] = (curr[0] & ~0b11) | ((data >> 0) & 0b11);
    curr[1] = (curr[1] & ~0b11) | ((data >> 2) & 0b11);
    curr[2] = (curr[2] & ~0b11) | ((data >> 4) & 0b11);
    curr[3] = (curr[3] & ~0b11) | ((data >> 6) & 0b11);
    *bits += 4;
}

void encode(FILE *data, FIBITMAP *img, FREE_IMAGE_FORMAT type, char wImgName[])
{
    long dataSize;
    BYTE *bits;
    fseek(data, 0, SEEK_END);
    dataSize = ftell(data);
    rewind(data);
    if (fopen(wImgName, "wb") == NULL)
    {
        printf("Couldn't get new image file");
        exit(1);
    }
    // FreeImage_Save(FIF_BMP, img, "Stegosaurus.bmp", 0);
    bits = FreeImage_GetBits(img);
    for (int i = 0; i < 4; i++)
        encodeByte(&bits, ((BYTE *)&dataSize)[i]);
    for (int i = 0; i <= dataSize; i++)
        encodeByte(&bits, fgetc(data));
    FreeImage_Save(type, img, wImgName, 0);
    // FreeImage_Save(FIF_BMP, img, "Steg_osaurus.bmp", 0);
}

BYTE decodeByte(BYTE **bits)
{
    BYTE *curr = *bits;
    *bits += 4;
    // printf("\nchomp %x %x %x %x\n", curr[0], curr[1], curr[2], curr[3]);
    return (BYTE)(((curr[0] & 0b11) << 0) | ((curr[1] & 0b11) << 2) | ((curr[2] & 0b11) << 4) | ((curr[3] & 0b11) << 6));
}

void decode(FIBITMAP *img, FILE *data)
{
    long dataSize;
    BYTE *bits;
    bits = FreeImage_GetBits(img);
    dataSize = *(long *)(BYTE[4]){decodeByte(&bits), decodeByte(&bits), decodeByte(&bits), decodeByte(&bits)};
    // printf("\nsize: %ld\n", dataSize);
    rewind(data);
    for (int i = 0; i < dataSize; i++)
        fputc(decodeByte(&bits), data);
}

int main(int argc, char *argv[])
{
    FILE *data;
    FIBITMAP *img;
    FREE_IMAGE_FORMAT type;
    if (argc < 4 || (argv[1][1] != 'e' && argv[1][1] != 'd'))
    {
        printf("Usage: stegosauruc.exe -[e for encode, d for decode] {image file to read} {data file to store(or write to, if decoding)} [{image file to write to if encoding}]\n");
        exit(1);
    }

    data = fopen(argv[3], "wb");
    if (data == NULL)
    {
        printf("Couldn't get data file");
        exit(1);
    }

    FreeImage_Initialise(TRUE);
    type = FreeImage_GetFileType(argv[2], 0);
    img = FreeImage_ConvertTo24Bits(FreeImage_Load(type, argv[2], 0));
    if (img == NULL)
    {
        printf("Couldn't get image file");
        exit(1);
    }

    if (argv[1][1] == 'e')
    {
        if (argc < 5)
        {
            printf("Not enough params");
            exit(1);
        }
        printf("Encoding... ");
        encode(data, img, type, argv[4]);
        printf("Done!");
    }
    else if (argv[1][1] == 'd')
        decode(img, data);
    FreeImage_Unload(img);
    fclose(data);
    return 0;
}