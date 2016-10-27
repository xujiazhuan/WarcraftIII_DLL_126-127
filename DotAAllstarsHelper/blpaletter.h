#pragma once
#ifndef BLPaletter
#define BLPaletter
#pragma warning(disable:4668)
#pragma warning(disable:4820)
#pragma warning(disable:4710)
#pragma warning(disable:4100)
#pragma warning(disable:4514)

#include <cmath>
#include <algorithm>

#include "quantizer.h"
#include "buffer.h"

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


#include <pshpack2.h>

/*struct BITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned long bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long bfOffBits;
};*/


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
string ToLower( string s );
bool FileToBuffer( Buffer &buf, const char* filename );
bool BufferToFile( Buffer &buf, const char* filename );

bool CanConvertTGA( const char* filename );
bool CanConvertBMP( const char* filename );
bool CanConvertJPG( const char* filename );
bool ColorsOk( int colors, bool isJpg );

unsigned long Blp2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, int &mipmaps, int & alphaflag, int & compresstype, int & pictype, char const *filename );
bool TGA2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp,  const char* filename );
bool BMP2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool JPG2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool CreatePalettedBLP( Buffer &rawData, Buffer &output, int colors, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );
bool CreateJpgBLP( Buffer &rawData, Buffer &output, int quality, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );

void ScaleImage( unsigned char* rawData, int oldW, int oldH, int newW, int newH, int bytespp, Buffer &target );
bool ApplyOverlay( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int maskBpp );
bool ApplyBorder( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int borderBpp );
bool UpdateIcon( Buffer &icon, IconType kind, bool enabled, int bpp );
bool CreateIcon( Buffer &input, int width, int height, int bpp, Buffer &icon, IconType kind, bool enabled );
bool CreateInfocardIcon( Buffer &input, int width, int height, int bpp, Buffer &icon, InfocardIconType kind );

#endif
