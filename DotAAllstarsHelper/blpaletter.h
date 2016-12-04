#pragma once
#ifndef BLPaletter
#define BLPaletter

#include <cmath>
#include <algorithm>

#include "quantizer.h"

#define XMD_H
#include "jpgwrapper.h"





enum IconType
{
	Active,
	Passive,
	Autocast
};

enum InfocardIconType
{
	Normal,
	Level
};

enum PictureType
{
	bmp,
	tga,
	jpg,
	blp
};
#include <pshpack1.h>

struct BLPHeader
{
	char     ident[ 4 ];       // Always 'BLP1'
	uint32_t    compress;           // 0: JPEG, 1: palette
	uint32_t    IsAlpha;          // 8: Alpha
	uint32_t    sizex;          // In pixels, power-of-two
	uint32_t    sizey;
	uint32_t    alphaEncoding;  // 3, 4: Alpha list, 5: Alpha from palette
	uint32_t    flags2;         // Unused
	uint32_t    poffs[ 16 ];
	uint32_t    psize[ 16 ];
};


struct TGAHeader
{
	unsigned char imageIDLength;
	unsigned char colorMapType;
	unsigned char imageType;
	unsigned short colorMapFirstEntryIndex;
	unsigned char colorMapLength;
	unsigned char colorMapEntrySize;
	unsigned short xOrigin;
	unsigned short yOrigin;
	unsigned short width;
	unsigned short height;
	unsigned char bpp;
	unsigned char reserved1 : 2;
	unsigned char imageOrigin : 2;
	unsigned char alphaChannelBits : 4;
};


struct tBGRAPixel
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
};


struct RGBAPix
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};
typedef struct RGBAPix RGBAPix;

struct PAPix
{
	unsigned char i;
	unsigned char A;
};
typedef struct PAPix PAPix;

struct PPix
{
	unsigned char i;
};
typedef struct PPix PPix;
#include <poppack.h>

typedef RGBAPix palette[ 256 ];

unsigned long Blp2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, int &mipmaps, int & alphaflag, int & compresstype, int & pictype, char const *filename );
bool TGA2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool BMP2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool JPG2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool CreatePalettedBLP( Buffer &rawData, Buffer &output, int colors, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );
bool CreateJpgBLP( Buffer &rawData, Buffer &output, int quality, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );

void ScaleImage( unsigned char* rawData, int oldW, int oldH, int newW, int newH, int bytespp, Buffer &target );
bool ApplyOverlay( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int maskBpp );
bool ApplyBorder( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int borderBpp );

#endif
