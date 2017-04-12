#pragma once
#ifndef BLPaletter
#define BLPaletter

#include <cmath>
#include <algorithm>

#include "quantizer.h"

#define XMD_H
//#include "jpgwrapper.h"
#include "Jpeg.h"




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


struct TGAHeader {
	char  imageIDLength;
	char  colorMapType;
	char  imageType;
	short int colourmaporigin;
	short int colourmaplength;
	char  colourmapdepth;
	short int x_origin;
	short int y_origin;
	short width;
	short height;
	char  bpp;
	char  imagedescriptor;
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

	RGBAPix( )
	{
		this->R = 0;
		this->G = 0;
		this->B = 0;
		this->A = 0;
	}

	RGBAPix( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = R;
		this->G = G;
		this->B = B;
		this->A = A;
	}

	RGBAPix RGBAPixWar3( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = B;
		this->G = G;
		this->B = R;
		this->A = A;
		return *this;
	}


	unsigned int RGBAPixWar3_u( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		return ( unsigned int )( this->A << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	unsigned int ToUINT( )
	{
		return ( unsigned int )( this->A << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	unsigned int FromUINT(unsigned int color )
	{
		return ( unsigned int )( this->A << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}
	RGBAPix FromString( const char * str )
	{
		if ( str && strlen( str ) == 8 )
		{

		}
		else
		{
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
		}
		return *this;
	}


};
typedef struct RGBAPix RGBAPix;

struct RGBPix
{
	unsigned char R;
	unsigned char G;
	unsigned char B;

	RGBPix( )
	{
		this->R = 0;
		this->G = 0;
		this->B = 0;
	}

	RGBPix( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = R;
		this->G = G;
		this->B = B;
	}

	RGBPix RGBPixWar3( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		this->R = B;
		this->G = G;
		this->B = R;
		return *this;
	}



	unsigned int RGBPixWar3_u( unsigned char R, unsigned char G, unsigned char B, unsigned char A )
	{
		return ( unsigned int )( 0 << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	unsigned int ToUINT( )
	{
		return ( unsigned int )( 0 << 24 | this->R << 16 | this->G << 8 | this->B << 0 );
	}

	RGBAPix ToRGBAPix( )
	{
		RGBAPix tmp = RGBAPix( );
		tmp.R = this->R;
		tmp.G = this->G;
		tmp.B = this->B;
		tmp.A = 255;
		return tmp;
	}


};
typedef struct RGBPix RGBPix;



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

unsigned long Blp2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, int &mipmaps, int & alphaflag, int & compresstype, int & pictype, char const *filename );
bool TGA2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool BMP2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool JPG2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, const char* filename );
bool CreatePalettedBLP( Buffer rawData, Buffer &output, int colors, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );
bool RAW2Tga( Buffer input, Buffer &output, int width, int height, int bpp, const char* filename );
bool CreateJpgBLP( Buffer rawData, Buffer &output, int quality, char const *filename, int width, int height, int bytespp, int  alphaflag, int &maxmipmaps );
void textureInvertRBInPlace( RGBAPix *bufsrc, unsigned long srcsize );
void ScaleImage( unsigned char* rawData, int oldW, int oldH, int newW, int newH, int bytespp, Buffer &target );
bool ApplyOverlay( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int maskBpp );
bool ApplyBorder( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int borderBpp );
void flip_vertically( unsigned char *pixels, const size_t width, const size_t height, const size_t bytes_per_pixel );
int ArrayXYtoId( int width, int x, int y );
#endif
