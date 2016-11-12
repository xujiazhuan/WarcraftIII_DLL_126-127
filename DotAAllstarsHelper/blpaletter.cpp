#include "blpaletter.h"
#include <pshpack2.h>

struct BITMAPFILEHEADER
{
	unsigned short bfType;
	unsigned long bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long bfOffBits;
};
struct BITMAPINFOHEADER
{
	unsigned long biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
};

#include <poppack.h>

// masks
Buffer autocastCorners;
Buffer normalCorners;
Buffer passiveCorners;
Buffer normalBorder;
Buffer autocastBorder;
Buffer disabledBorder;
Buffer passiveBorder;
Buffer infocardCorners;
Buffer infocardLevelCorners;
Buffer infocardBorder;
Buffer infocardLevelBorder;

bool IsPowerOfTwo( const long i )
{
	long t = i;
	while ( t % 2 == 0 )
		t >>= 1;
	return ( t == 1 );
}

string ToLower( string s )
{
	for ( unsigned int i = 0; i < s.length( ); i++ )
	{
		s[ i ] = ( char ) tolower( s[ i ] );
	}
	return s;
}

bool GetFirstBytes( const char* filename, char* buffer, unsigned long length )
{
	FILE* file;
	fopen_s( &file, filename, "rb" );
	if ( !file )
		return false;
	if ( fread( buffer, 1, length, file ) < length )
	{
		fclose( file );
		return false;
	}
	fclose( file );
	return true;
}

bool ColorsOk( int colors, bool isJpg )
{
	if ( isJpg && ( colors < 1 || colors > 99 ) )
	{
		fprintf( stderr, "Unsupported JPG quality %d specified\n", colors );
		return false;
	}
	if ( !isJpg && ( colors < 16 || colors > 256 ) )
	{
		fprintf( stderr, "Unsupported palette size %d specified\n", colors );
		return false;
	}
	return true;
}

bool MaskOk( unsigned char *mask, int expectedWidth, int expectedHeight, int expectedBpp, long &offset, const char *maskFile )
{
	TGAHeader* header = ( TGAHeader* ) mask;
	if ( header->colorMapType != 0 || header->imageType != 2 || header->width == 0 || header->height == 0 )
	{
		fprintf( stderr, "Unsupported TGA format of %s\n", maskFile );
		return false;
	}
	if ( header->width != expectedWidth || header->height != expectedHeight )
	{
		fprintf( stderr, "Dimensions of %s are wrong\n", maskFile );
		return false;
	}
	if ( header->bpp / 8 != expectedBpp )
	{
		fprintf( stderr, "Bits per pixel of %s are not %d\n", maskFile, expectedBpp );
		return false;
	}
	offset = ( long ) ( sizeof( TGAHeader ) + header->imageIDLength );
	return true;
}

template<typename T>
inline void AssignWeightedPixel( double *target, T *source, double weight, int bytespp, bool add )
{
	if ( !add )
	{
		target[ 0 ] = ( ( double ) source[ 0 ] ) * weight;
		target[ 1 ] = ( ( double ) source[ 1 ] ) * weight;
		target[ 2 ] = ( ( double ) source[ 2 ] ) * weight;
		if ( bytespp == 4 )
			target[ 3 ] = ( ( double ) source[ 3 ] ) * weight;
	}
	else
	{
		target[ 0 ] += ( ( double ) source[ 0 ] ) * weight;
		target[ 1 ] += ( ( double ) source[ 1 ] ) * weight;
		target[ 2 ] += ( ( double ) source[ 2 ] ) * weight;
		if ( bytespp == 4 )
			target[ 3 ] += ( ( double ) source[ 3 ] ) * weight;
	}
}

inline unsigned char NormalizeComponent( double val )
{
	if ( val < 0.0 )
		return 0;
	if ( val > ( double )0xFF )
		return 0xFF;
	return ( unsigned char ) val;
}

void ScaleImage( unsigned char* rawData, int oldW, int oldH, int newW, int newH, int bytespp, Buffer &target )
{
	if ( oldW == newW && oldH == newH )
	{
		target.length = ( unsigned long ) ( newW * newH * bytespp );
		target.buf = new char[ target.length ];
		memcpy( target.buf, rawData, target.length );
		return;
	}

	// scale horizontally
	double* temp = new double[ ( unsigned int ) ( oldH * newW * bytespp ) ];


	if ( oldW == newW )
	{
		for ( int i = 0; i < oldW * oldH * bytespp; i++ )
			temp[ i ] = ( double ) rawData[ i ];
	}
	else
	{
		double sum = 0;
		double diffW = ( ( double ) oldW / ( double ) newW );
		for ( int i = 0; i < newW; i++ )
		{
			double newSum = sum + diffW;
			int pix = ( int ) floor( sum );
			double weight = min( diffW, 1.0 - fmod( sum, 1.0 ) );
			for ( int j = 0; j < oldH; j++ )
				AssignWeightedPixel( &temp[ ( j*newW + i )*bytespp ], &rawData[ ( j*oldW + pix )*bytespp ], ( weight / diffW ), bytespp, false );
			sum += weight;
			while ( sum < newSum )
			{
				weight = min( newSum - sum, 1.0 );
				pix++;
				for ( int j = 0; j < oldH; j++ )
					AssignWeightedPixel( &temp[ ( j*newW + i )*bytespp ], &rawData[ ( j*oldW + pix )*bytespp ], ( weight / diffW ), bytespp, true );
				sum += weight;
			}
		}
	}

	// scale vertically
	target.length = ( unsigned long ) ( newW * newH * bytespp );
	target.buf = new char[ target.length ];
	double* final = new double[ target.length ];


	if ( newH == oldH )
	{
		memcpy( final, temp, target.length * sizeof( double ) );
	}
	else
	{
		double sum = 0;
		double diffH = ( ( double ) oldH / ( double ) newH );
		for ( int j = 0; j < newH; j++ )
		{
			double newSum = sum + diffH;
			int pix = ( int ) floor( sum );
			double weight = min( diffH, 1.0 - fmod( sum, 1.0 ) );
			for ( int i = 0; i < newW; i++ )
				AssignWeightedPixel( &final[ ( j*newW + i )*bytespp ], &temp[ ( pix*newW + i )*bytespp ], ( weight / diffH ), bytespp, false );
			sum += weight;
			while ( sum < newSum )
			{
				weight = min( newSum - sum, 1.0 );
				pix++;
				for ( int i = 0; i < newW; i++ )
					AssignWeightedPixel( &final[ ( j*newW + i )*bytespp ], &temp[ ( pix*newW + i )*bytespp ], ( weight / diffH ), bytespp, true );
				sum += weight;
			}
		}
	}
	for ( unsigned long i = 0; i < target.length; i++ )
		target.buf[ i ] = ( char ) NormalizeComponent( final[ i ] );
	delete[ ] final;
	delete[ ] temp;
}

void SubtractColor( unsigned char &pixel, unsigned char &mask )
{
	if ( 0xFF - mask > pixel )
		pixel = 0;
	else
		pixel -= ( 0xFF - mask );
}

void DivideColor( unsigned char &pixel, unsigned char &mask )
{
	pixel = ( unsigned char ) ( ( double ) pixel * ( ( double ) mask / ( double ) 0xFF ) );
}

bool ApplyOverlay( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int maskBpp )
{
	if ( !mask )
		return false;
	for ( int i = 0; i < width * height; i++ )
	{
		DivideColor( rawData[ i*bytespp ], mask[ i*maskBpp ] );
		DivideColor( rawData[ i*bytespp + 1 ], mask[ i*maskBpp + 1 ] );
		DivideColor( rawData[ i*bytespp + 2 ], mask[ i*maskBpp + 2 ] );
		if ( bytespp == 4 && maskBpp == 4 )
			DivideColor( rawData[ i*bytespp + 3 ], mask[ i*maskBpp + 3 ] );
	}
	return true;
}

bool ApplyBorder( unsigned char* rawData, unsigned char* mask, int width, int height, int bytespp, int borderBpp )
{
	if ( !mask )
		return false;
	if ( borderBpp == 4 )
	{
		for ( int i = 0; i < width * height; i++ )
		{
			if ( mask[ i*borderBpp + 3 ] == 0xFF )
			{ // no transparence
				rawData[ i*bytespp ] = mask[ i*borderBpp ];
				rawData[ i*bytespp + 1 ] = mask[ i*borderBpp + 1 ];
				rawData[ i*bytespp + 2 ] = mask[ i*borderBpp + 2 ];
				if ( bytespp == 4 )
					rawData[ i*bytespp + 3 ] = mask[ i*borderBpp + 3 ];
			}
			else if ( mask[ i*borderBpp + 3 ] == 0x00 )
			{ // full transparence
				rawData[ i*bytespp ] = rawData[ i*bytespp + 1 ] = rawData[ i*bytespp + 2 ] = 0x00;
				if ( bytespp == 4 )
					rawData[ i*bytespp + 3 ] = 0x00;
			}
		}
	}
	else
	{
		for ( int i = 0; i < width * height; i++ )
		{
			if ( mask[ i*borderBpp ] != 0xFF || mask[ i*borderBpp + 1 ] != 0xFF || mask[ i*borderBpp + 2 ] != 0xFF )
			{ // not white
				rawData[ i*bytespp ] = mask[ i*borderBpp ];
				rawData[ i*bytespp + 1 ] = mask[ i*borderBpp + 1 ];
				rawData[ i*bytespp + 2 ] = mask[ i*borderBpp + 2 ];
			}
		}
	}
	return true;
}

bool FileToBuffer( Buffer &buf, const char* filename )
{
	FILE* file;
	fopen_s( &file, filename, "rb" );
	if ( !file )
	{
		fprintf( stderr, "Could not open %s\n", filename );
		return false;
	}
	fseek( file, 0, SEEK_END );
	buf.length = ( unsigned long ) ftell( file );
	buf.buf = new char[ buf.length ];
	if ( !buf.buf )
	{
		fprintf( stderr, "Out of memory\n" );
		return false;
	}
	rewind( file );
	fread( buf.buf, 1, buf.length, file );
	fclose( file );
	return true;
}

bool BufferToFile( Buffer &buf, const char* filename )
{
	FILE* file;
	fopen_s( &file, filename, "wb" );
	if ( !file )
	{
		fprintf( stderr, "Could not open %s for output\n", filename );
		return false;
	}
	fwrite( buf.buf, 1, buf.length, file );
	fclose( file );
	return true;
}

unsigned char* GetMask( Buffer &mask, const char* maskFile )
{
	if ( !mask.buf )
	{
		Buffer input;
		int wh = 64;
		if ( &mask == &infocardBorder || &mask == &infocardLevelBorder )
			wh = 48;
		if ( !FileToBuffer( input, maskFile ) )
			return 0;
		long offset = 0;
		if ( !MaskOk( ( unsigned char* ) input.buf, wh, wh, 4, offset, maskFile ) )
		{
			delete[ ] input.buf;
			return 0;
		}
		mask.length = ( unsigned long ) ( wh*wh * 4 );
		mask.buf = new char[ mask.length ];
		memcpy( mask.buf, input.buf + offset, mask.length );
		delete[ ] input.buf;
	}
	return ( unsigned char* ) mask.buf;
}

bool UpdateIcon( Buffer &icon, IconType kind, bool enabled, int bpp )
{
	if ( enabled )
	{
		if ( kind == Active )
		{
			return ApplyOverlay( ( unsigned char* ) icon.buf, GetMask( normalBorder, "NormalBorder" ), 64, 64, bpp, 4 ) &&
				ApplyBorder( ( unsigned char* ) icon.buf, GetMask( normalCorners, "NormalCorners" ), 64, 64, bpp, 4 );
		}
		else if ( kind == Passive )
		{
			return ApplyOverlay( ( unsigned char* ) icon.buf, GetMask( passiveBorder, "PassiveBorder" ), 64, 64, bpp, 4 ) &&
				ApplyBorder( ( unsigned char* ) icon.buf, GetMask( passiveCorners, "PassiveCorners" ), 64, 64, bpp, 4 );
		}
		else
		{
			return ApplyOverlay( ( unsigned char* ) icon.buf, GetMask( autocastBorder, "AutocastBorder" ), 64, 64, bpp, 4 ) &&
				ApplyBorder( ( unsigned char* ) icon.buf, GetMask( autocastCorners, "AutocastCorners" ), 64, 64, bpp, 4 );
		}
	}
	else
	{
		if ( kind == Active )
		{
			return ApplyBorder( ( unsigned char* ) icon.buf, GetMask( normalCorners, "NormalCorners" ), 64, 64, bpp, 4 ) &&
				ApplyOverlay( ( unsigned char* ) icon.buf, GetMask( disabledBorder, "DisabledBorder" ), 64, 64, bpp, 4 );
		}
		else if ( kind == Passive )
		{
			return ApplyBorder( ( unsigned char* ) icon.buf, GetMask( passiveCorners, "PassiveCorners" ), 64, 64, bpp, 4 ) &&
				ApplyOverlay( ( unsigned char* ) icon.buf, GetMask( disabledBorder, "DisabledBorder" ), 64, 64, bpp, 4 );
		}
		else
		{
			return ApplyBorder( ( unsigned char* ) icon.buf, GetMask( autocastCorners, "AutocastCorners" ), 64, 64, bpp, 4 ) &&
				ApplyOverlay( ( unsigned char* ) icon.buf, GetMask( disabledBorder, "DisabledBorder" ), 64, 64, bpp, 4 );
		}
	}
}

bool CreateIcon( Buffer &input, int width, int height, int bpp, Buffer &icon, IconType kind, bool enabled )
{
	ScaleImage( ( unsigned char* ) input.buf, width, height, 64, 64, bpp, icon );
	return UpdateIcon( icon, kind, enabled, bpp );
}

bool CreateInfocardIcon( Buffer &input, int width, int height, int bpp, Buffer &icon, InfocardIconType kind )
{
	Buffer temp;
	ScaleImage( ( unsigned char* ) input.buf, width, height, 48, 48, bpp, temp );
	if ( kind == Normal )
	{
		if ( !ApplyOverlay( ( unsigned char* ) temp.buf, GetMask( infocardBorder, "InfocardBorder" ), 48, 48, bpp, 4 ) )
		{
			delete[ ] temp.buf;
			return false;
		}
	}
	else
	{
		if ( !ApplyOverlay( ( unsigned char* ) temp.buf, GetMask( infocardLevelBorder, "InfocardLevelBorder" ), 48, 48, bpp, 4 ) )
		{
			delete[ ] temp.buf;
			return false;
		}
	}

	icon.length = ( unsigned long ) ( 64 * 64 * bpp );
	icon.buf = new char[ icon.length ];
	for ( int i = 0; i < 64; i++ )
	{
		if ( i >= 12 && i < 60 )
		{
			memset( icon.buf + i * 64 * bpp, 0, ( size_t ) ( 4 * bpp ) );
			memcpy( icon.buf + ( i * 64 + 4 )*bpp, temp.buf + ( i - 12 ) * 48 * bpp, ( size_t ) ( 48 * bpp ) );
			memset( icon.buf + ( i * 64 + 52 )*bpp, 0, ( size_t ) ( 12 * bpp ) );
		}
		else
		{
			memset( icon.buf + i * 64 * bpp, 0, ( size_t ) ( 64 * bpp ) );
		}
	}
	delete[ ] temp.buf;
	if ( kind == Normal )
		return ApplyBorder( ( unsigned char* ) icon.buf, GetMask( infocardCorners, "InfocardCorners" ), 64, 64, bpp, 4 );
	else
		return ApplyBorder( ( unsigned char* ) icon.buf, GetMask( infocardLevelCorners, "InfocardLevelCorners" ), 64, 64, bpp, 4 );
}

bool CanConvertTGA( const char* filename )
{
	TGAHeader header;
	if ( !GetFirstBytes( filename, ( char* ) &header, sizeof( header ) ) )
		return false;
	if ( header.colorMapType != 0 || header.imageType != 2 || header.width == 0 || header.height == 0 )
		return false;
	if ( !IsPowerOfTwo( header.width ) || !IsPowerOfTwo( header.height ) )
		return false;
	if ( header.bpp != 32 && header.bpp != 24 )
		return false;
	return true;
}

bool CanConvertBMP( const char* filename )
{
	char buf[ sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) ];
	if ( !GetFirstBytes( filename, buf, sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) ) )
		return false;
	BITMAPFILEHEADER *FileHeader = ( BITMAPFILEHEADER* ) buf;
	BITMAPINFOHEADER *InfoHeader = ( BITMAPINFOHEADER* ) ( FileHeader + 1 );
	if ( FileHeader->bfType != 0x4D42 )
		return false;
	if ( !IsPowerOfTwo( InfoHeader->biWidth ) || !IsPowerOfTwo( InfoHeader->biHeight ) )
		return false;
	if ( InfoHeader->biBitCount != 32 && InfoHeader->biBitCount != 24 )
		return false;
	return true;
}

bool CanConvertJPG( const char* )
{
	return true;
}

int GetRequiredMipMaps( int width, int height )
{
	int mips = 0;
	while ( width > 0 && height > 0 )
	{
		mips++;
		width = width / 2;
		height = height / 2;
	}
	return mips;
}

bool CreateJpgBLP( Buffer &rawData, Buffer &output, int quality, char const *, int width, int height, int bytespp, int alphaflag, int &maxmipmaps )
{
	Buffer target[ 16 ];
	Buffer scaled[ 16 ];
	Buffer source;
	source.buf = rawData.buf;
	source.length = ( unsigned long ) ( width * height * 4 );
	if ( bytespp < 4 )
	{
		source.buf = new char[ ( unsigned int ) ( width * height * 4 ) ];
		for ( int j = 0; j < width * height; j++ )
		{
			memcpy( source.buf + j * 4, rawData.buf + j*bytespp, ( size_t ) bytespp );
			source.buf[ j * 4 + 3 ] = '\xFF';
		}
	}

	int truemipmaps = GetRequiredMipMaps( width, height );
	BLPHeader blpHeader;
	memcpy( blpHeader.ident, "BLP1", 4 );
	blpHeader.compress = 0; // jpg compression
	blpHeader.IsAlpha = ( uint32_t ) alphaflag;
	blpHeader.sizey = ( unsigned long ) height;
	blpHeader.sizex = ( unsigned long ) width;
	blpHeader.alphaEncoding = ( uint32_t ) ( !alphaflag ? 5 : 4 ); // BGR or BGRA
	blpHeader.flags2 = 1;
	memset( &blpHeader.poffs, 0, 16 * sizeof( long ) );
	memset( &blpHeader.psize, 0, 16 * sizeof( long ) );

	int xdimension = width;
	int ydimension = height;
	output.length = sizeof( BLPHeader ) + 4; // header + one int for jpg header size
	for ( int i = 0; i < 16; i++ )
	{
		if ( i < maxmipmaps && xdimension > 0 && ydimension > 0 )
		{
			if ( i == 0 )
				scaled[ 0 ] = source;
			else // generate mipmaps
				ScaleImage( ( unsigned char* ) scaled[ i - 1 ].buf, xdimension * 2, ydimension * 2, xdimension, ydimension, 4, scaled[ i ] );
			if ( !ConvertToJpg( scaled[ i ], target[ i ], xdimension, ydimension, 4, quality, true ) )
			{
				for ( int j = 0; j <= i; j++ )
				{ // cleanup
					if ( bytespp < 4 || j > 0 )
						delete[ ] scaled[ j ].buf;
					if ( target[ j ].buf )
						delete[ ] target[ j ].buf;
				}
				return false;
			}
			blpHeader.poffs[ i ] = output.length;
			blpHeader.psize[ i ] = target[ i ].length;
			output.length += target[ i ].length;
		}
		else
		{
			if ( i < truemipmaps )
			{
				blpHeader.poffs[ i ] = blpHeader.poffs[ i - 1 ];
				blpHeader.psize[ i ] = blpHeader.psize[ i - 1 ];
			}
			else
			{
				blpHeader.poffs[ i ] = 0;
				blpHeader.psize[ i ] = 0;
			}
		}
		xdimension = xdimension / 2;
		ydimension = ydimension / 2;
	}
	maxmipmaps = min( truemipmaps, maxmipmaps );

	output.buf = new char[ output.length ];
	memcpy( output.buf, &blpHeader, sizeof( BLPHeader ) );
	memset( output.buf + sizeof( BLPHeader ), 0, 4 );
	char* blp = output.buf + sizeof( BLPHeader ) + 4;
	for ( int i = 0; i < 16; i++ )
	{
		if ( i < maxmipmaps && width > 0 && height > 0 )
		{
			memcpy( blp, target[ i ].buf, target[ i ].length );
			if ( bytespp < 4 || i > 0 ) // cleanup
				delete[ ] scaled[ i ].buf;
			if ( target[ i ].buf )
				delete[ ] target[ i ].buf;
			blp += target[ i ].length;
		}
		width = width / 2;
		height = height / 2;
	}
	return true;
}

bool CreatePalettedBLP( Buffer &rawData, Buffer &output, int colors, char const *, int width, int height, int bytespp, int alphaflag, int &maxmipmaps )
{
	CQuantizer* q = new CQuantizer( ( unsigned int ) colors, 8 );
	q->ProcessImage( ( unsigned char* ) rawData.buf, ( unsigned long ) ( width * height ), ( unsigned char ) bytespp, 0x00 );
	int truemipmaps = GetRequiredMipMaps( width, height );
	BLPHeader blpHeader;
	memcpy( blpHeader.ident, "BLP1", 4 );
	blpHeader.compress = 1; // paletted
	blpHeader.IsAlpha = 8;
	blpHeader.sizey = ( unsigned long ) height;
	blpHeader.sizex = ( unsigned long ) width;
	blpHeader.alphaEncoding = 4; // BGR or BGRA
	blpHeader.flags2 = 1;
	memset( &blpHeader.poffs, 0, 16 * sizeof( long ) );
	memset( &blpHeader.psize, 0, 16 * sizeof( long ) );


	if ( !maxmipmaps )
		maxmipmaps = truemipmaps;

	output.length = sizeof( BLPHeader ) + sizeof( BGRAPix ) * 256; // header + palette
	Buffer bufs[ 16 ];
	int xdimension = width;
	int ydimension = height;
	for ( int i = 0; i < 16; i++ )
	{
		if ( i < maxmipmaps && xdimension > 0 && ydimension > 0 )
		{
			if ( i == 0 )
				bufs[ 0 ] = rawData;
			else // generate mipmaps
				ScaleImage( ( unsigned char* ) bufs[ i - 1 ].buf, xdimension * 2, ydimension * 2, xdimension, ydimension, bytespp, bufs[ i ] );
			blpHeader.poffs[ i ] = output.length;
			blpHeader.psize[ i ] = ( unsigned long ) ( xdimension * ydimension * 2 ); //(q->NeedsAlphaChannel() ? 2 : 1);
			output.length += blpHeader.psize[ i ];
		}
		else
		{
			if ( i < truemipmaps )
			{ // war3 requires at least 8 mipmaps for the alpha channel to work
				blpHeader.poffs[ i ] = blpHeader.poffs[ i - 1 ];
				blpHeader.psize[ i ] = blpHeader.psize[ i - 1 ];
			}
			else
			{
				blpHeader.poffs[ i ] = 0;
				blpHeader.psize[ i ] = 0;
			}
		}
		xdimension = xdimension / 2;
		ydimension = ydimension / 2;
	}
	//maxmipmaps = min( truemipmaps, maxmipmaps );

	output.buf = new char[ output.length ];
	unsigned char* blpData = ( unsigned char* ) output.buf;
	memcpy( blpData, &blpHeader, sizeof( BLPHeader ) );
	memset( blpData + sizeof( BLPHeader ), 0, sizeof( BGRAPix ) * 256 );
	unsigned char* blp = blpData + sizeof( BLPHeader ) + sizeof( BGRAPix ) * 256;
	BGRAPix *palette = ( BGRAPix* ) ( blpData + sizeof( BLPHeader ) );
	q->SetColorTable( palette );
	for ( int i = 0; i <= 16; i++ )
	{
		if ( i < maxmipmaps && width > 0 && height > 0 )
		{
			BGRAPix* raw = ( BGRAPix* ) bufs[ i ].buf;
			memset( blp, 0xFF, ( size_t ) ( width * height * 2 ) ); //(q->NeedsAlphaChannel() ? 2 : 1);
			q->FloydSteinbergDither( ( unsigned char* ) bufs[ i ].buf, width, height, ( unsigned char ) bytespp, blp, palette );
			if ( q->NeedsAlphaChannel( ) )
			{
				for ( int y = 0; y < height; y++ )
				{
					for ( int x = 0; x < width; x++ )
					{
						unsigned char *z = blp + width * ( height - y - 1 ) + x + width * height;
						BGRAPix *j = raw + width * y + x;
						*( z ) = j->A;
					}
				}
			}

			/*
			if (q->NeedsAlphaChannel()) {
				memset(blp, 0xFF, width * height * 2);
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						unsigned char *i = blp + width * (height - y - 1) + x;
						BGRAPix *j = raw + width * y + x;
						*(i) = q->GetNearestIndexFast(j, palette);
						*(i + width * height) = j->A;
					}
				}
			} else {
				//memset(blp, 0xFF, width * height);
				memset(blp, 0xFF, width * height * 2);
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						*(blp + width * (height - y - 1) + x) = q->GetNearestIndexFast((BGRAPix*)((unsigned char*)raw + (width * y + x) * bytespp), palette);
					}
				}
			}
			*/
			if ( i > 0 )
				delete[ ] bufs[ i ].buf; // cleanup
			blp += width * height * 2; //(q->NeedsAlphaChannel() ? 2 : 1);
		}
		width = width / 2;
		height = height / 2;
	}

	delete q;
	return true;
}

bool TGA2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, const char* filename )
{
	TGAHeader* header = ( TGAHeader* ) input.buf;
	if ( header->colorMapType != 0 || header->imageType != 2 || header->width == 0 || header->height == 0 )
	{
		fprintf( stderr, "Unsupported TGA format of %s\n", filename );
		return false;
	}

	if ( !IsPowerOfTwo( header->width ) || !IsPowerOfTwo( header->height ) )
	{
		fprintf( stderr, "Width or height of %s are not power of two\n", filename );
		return false;
	}
	if ( header->bpp != 32 && header->bpp != 24 )
	{
		fprintf( stderr, "Bits per pixel of %s are not 24 or 32\n", filename );
		return false;
	}
	bpp = 4;
	if ( header->bpp < 32 )
		bpp = 3;
	width = header->width;
	height = header->height;
	output.length = ( unsigned long ) ( width*height*bpp );
	output.buf = input.buf + sizeof( TGAHeader ) + header->imageIDLength;
	return true;
}

bool BMP2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, char const *filename )
{
	BITMAPFILEHEADER *FileHeader = ( BITMAPFILEHEADER* ) input.buf;
	BITMAPINFOHEADER *InfoHeader = ( BITMAPINFOHEADER* ) ( FileHeader + 1 );
	if ( FileHeader->bfType != 0x4D42 )
	{
		fprintf( stderr, "Unsupported BMP format of %s\n", filename );
		return false;
	}
	if ( !IsPowerOfTwo( InfoHeader->biWidth ) || !IsPowerOfTwo( InfoHeader->biHeight ) )
	{
		fprintf( stderr, "Width or height of %s are not power of two\n", filename );
		return false;
	}
	if ( InfoHeader->biBitCount != 32 && InfoHeader->biBitCount != 24 )
	{
		fprintf( stderr, "Bits per pixel of %s are not 24 or 32\n", filename );
		return false;
	}
	bpp = 4;
	if ( InfoHeader->biBitCount < 32 )
		bpp = 3;
	width = InfoHeader->biWidth;
	height = InfoHeader->biHeight;
	output.length = ( unsigned long ) ( width*height*bpp );
	output.buf = input.buf + FileHeader->bfOffBits;
	if ( bpp == 4 ) // invert alpha
		for ( int i = 0; i < width*height; i++ )
			output.buf[ i*bpp + 3 ] = 0xFF - output.buf[ i*bpp + 3 ];
	return true;
}

void SwapBLPHeader( BLPHeader *header )
{
	header->compress = ( unsigned long ) _blp_swap_int32( header->compress );
	header->IsAlpha = ( unsigned long ) _blp_swap_int32( header->IsAlpha );
	header->sizex = ( unsigned long ) _blp_swap_int32( header->sizex );
	header->sizey = ( unsigned long ) _blp_swap_int32( header->sizey );
	header->alphaEncoding = ( unsigned long ) _blp_swap_int32( header->alphaEncoding );
	header->flags2 = ( unsigned long ) _blp_swap_int32( header->flags2 );

	int i = 0;
	for ( ; i < 16; i++ )
	{
		( header->poffs )[ i ] = ( unsigned long ) _blp_swap_int32( ( header->poffs )[ i ] );
		( header->psize )[ i ] = ( unsigned long ) _blp_swap_int32( ( header->psize )[ i ] );
	}
}


static void textureInvertRBInPlace( RGBAPix *bufsrc, unsigned long srcsize )
{
	for ( unsigned long i = 0; i < ( srcsize / 4 ); i++ )
	{
		unsigned char red = bufsrc[ i ].B;
		bufsrc[ i ].B = bufsrc[ i ].R;
		bufsrc[ i ].R = red;
	}
}

struct tBGRAPixel
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
};

void flip_vertically( unsigned char *pixels, const size_t width, const size_t height, const size_t bytes_per_pixel )
{
	const size_t stride = width * bytes_per_pixel;
	unsigned char *row = ( unsigned char * ) malloc( stride );
	if ( !row )
		return;
	unsigned char *low = pixels;
	unsigned char *high = &pixels[ ( height - 1 ) * stride ];

	for ( ; low < high; low += stride, high -= stride )
	{
		memcpy( row, low, stride );
		memcpy( low, high, stride );
		memcpy( high, row, stride );
	}
	free( row );
}


tBGRAPixel * blp1_convert_paletted_separated_alpha_BGRA( uint8_t* pSrc, tBGRAPixel* pInfos, unsigned int width, unsigned int height, bool invertAlpha )
{
	tBGRAPixel* pBuffer = new tBGRAPixel[ width * height ];
	tBGRAPixel* pDst = pBuffer;

	uint8_t* pIndices = pSrc;
	uint8_t* pAlpha = pSrc + width * height;

	for ( unsigned int y = 0; y < height; y++ )
	{
		for ( unsigned int x = 0; x < width; x++ )
		{
			*pDst = pInfos[ *pIndices ];

			if ( invertAlpha )
				pDst->a = ( uint8_t ) ( 0xFF - *pAlpha );
			else
				pDst->a = *pAlpha;

			++pIndices;
			++pAlpha;
			++pDst;
		}
	}
	flip_vertically( ( unsigned char* ) pBuffer, width, height, 4 );

	return pBuffer;
}

RGBAPix * blp1_convert_paletted_separated_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height, bool invertAlpha )
{
	RGBAPix * outrgba = ( RGBAPix * ) blp1_convert_paletted_separated_alpha_BGRA( pSrc, ( tBGRAPixel * ) pInfos, width, height, invertAlpha );

	/*for ( int i = 0; i < width * height; i++ )
	{
		int red = outrgba[ i ].B;
		outrgba[ i ].B = outrgba[ i ].R;
		outrgba[ i ].R = red;
	}*/

	return outrgba;
}


tBGRAPixel* blp1_convert_paletted_alpha_BGRA( uint8_t* pSrc, tBGRAPixel* pInfos, unsigned int width, unsigned int height )
{
	tBGRAPixel* pBuffer = new tBGRAPixel[ width * height ];
	tBGRAPixel* pDst = pBuffer;

	uint8_t* pIndices = pSrc;

	for ( unsigned int y = 0; y < height; ++y )
	{
		for ( unsigned int x = 0; x < width; ++x )
		{
			*pDst = pInfos[ *pIndices ];
			pDst->a = ( uint8_t ) ( 0xFF - pDst->a );

			++pIndices;
			++pDst;
		}
	}

	return pBuffer;
}

RGBAPix* blp1_convert_paletted_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height )
{
	RGBAPix * outrgba = ( RGBAPix * ) blp1_convert_paletted_alpha_BGRA( pSrc, ( tBGRAPixel* ) pInfos, width, height );

	/*for ( int i = 0; i < width * height; i++ )
	{
		int red = outrgba[ i ].B;
		outrgba[ i ].B = outrgba[ i ].R;
		outrgba[ i ].R = red;
	}*/

	return outrgba;
}

tBGRAPixel* blp1_convert_paletted_no_alpha_BGRA( uint8_t* pSrc, tBGRAPixel* pInfos, unsigned int width, unsigned int height )
{
	tBGRAPixel* pBuffer = new tBGRAPixel[ width * height ];
	tBGRAPixel* pDst = pBuffer;

	uint8_t* pIndices = pSrc;

	for ( unsigned int y = 0; y < height; ++y )
	{
		for ( unsigned int x = 0; x < width; ++x )
		{
			*pDst = pInfos[ *pIndices ];
			pDst->a = 0xFF;

			++pIndices;
			++pDst;
		}
	}
	flip_vertically( ( unsigned char* ) pBuffer, width, height, 4 );

	return pBuffer;
}

RGBAPix* blp1_convert_paletted_no_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height )
{
	RGBAPix * outrgba = ( RGBAPix * ) blp1_convert_paletted_no_alpha_BGRA( pSrc, ( tBGRAPixel* ) pInfos, width, height );

	/*for ( int i = 0; i < width * height; i++ )
	{
		int red = outrgba[ i ].B;
		outrgba[ i ].B = outrgba[ i ].R;
		outrgba[ i ].R = red;

	}*/

	return outrgba;
}

unsigned long Blp2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, int &mipmaps, int & alphaflag, int & compresstype, int & pictype, char const *filename )
{
	BLPHeader blph;
	bpp = 4;
	width = 0;
	height = 0;
	unsigned long curpos = 0;
	unsigned long textureSize = 0;
	if ( input.buf == NULL || input.length == NULL || input.length < sizeof( BLPHeader ) )
	{
		fprintf( stderr, "Input buffer NULL or Small ( %s )\n", filename );
		return 0;
	}

	memcpy( &blph, input.buf, sizeof( BLPHeader ) );


	//SwapBLPHeader( &blph );




	if ( memcmp( blph.ident, "BLP1", 4 ) != 0 )
	{
		return 0;
	}
	mipmaps = 0;
	for ( int i = 0; i < 15; i++ )
	{
		if ( blph.poffs[ i ] > 0 )
		{
			mipmaps++;
		}
	}


	alphaflag = ( int ) blph.IsAlpha;


	curpos += sizeof( BLPHeader );
	textureSize = blph.sizex * blph.sizey * 4;
	compresstype = ( int ) blph.compress;

	pictype = ( int ) blph.alphaEncoding;
	if ( blph.compress == 1 )
	{
		//fprintf( stderr, "c1\n" );

		if ( input.length < curpos + 256 * 4 )
		{
			fprintf( stderr, "Input buffer NULL or Small ( %s ) 2\n", filename );
			return 0;
		}


		/*if ( output.buf == NULL )
		{
			width = blph.sizex;
			height = blph.sizey;
			return textureSize;
		}*/

		RGBAPix Pal[ 256 ];
		memcpy( Pal, input.buf + curpos, 256 * 4 );
		curpos += 256 * 4;

		int offset = ( int ) blph.poffs[ 0 ];
		int size = ( int ) blph.psize[ 0 ];


		//fprintf( stderr, "c2\n" );
		// alpha channel
		if ( alphaflag > 0 && ( blph.alphaEncoding == 4 || blph.alphaEncoding == 3 ) )
		{
			bpp = 4;

			if ( input.length < curpos + blph.sizex * blph.sizey * 2 )
			{
				fprintf( stderr, "Input buffer NULL or Small ( %s ) 3\n", filename );
				return 0;
			}
			//	fprintf( stderr, "c3\n" );
			uint8_t* tdata = new uint8_t[ ( unsigned int ) size ];
			memcpy( tdata, input.buf + offset, ( size_t ) size );
			//	fprintf( stderr, "c4\n" );


			RGBAPix *pic = blp1_convert_paletted_separated_alpha( ( uint8_t* ) tdata, Pal, blph.sizex, blph.sizey, 0 );


			/*	unsigned long k, j, i;
				//fprintf( stderr, "c5\n" );
				j = 0;
				i = ( blph.sizex * blph.sizey ) / 2;
				for ( k = 0; k < i; ++k )
				{
					pic[ j ].R = Pal[ tdata[ k ].i ].B;
					pic[ j ].G = Pal[ tdata[ k ].i ].G;
					pic[ j ].B = Pal[ tdata[ k ].i ].R;
					pic[ j ].A = tdata[ k + i ].i;
					++j;

					pic[ j ].R = Pal[ tdata[ k ].A ].B;
					pic[ j ].G = Pal[ tdata[ k ].A ].G;
					pic[ j ].B = Pal[ tdata[ k ].A ].R;
					pic[ j ].A = tdata[ k + i ].A;
					++j;
				}*/
				//	fprintf( stderr, "c6\n" );
			delete[ ] tdata;


			output.length = textureSize;
			output.buf = new char[ textureSize ];
			memcpy( output.buf, pic, textureSize );
			delete[ ] pic;

			width = ( int ) blph.sizex;
			height = ( int ) blph.sizey;
			//	fprintf( stderr, "c7\n" );
			return textureSize;
		}
		else if ( alphaflag > 0 && blph.alphaEncoding == 5 )
		{

			bpp = 4;


			//	fprintf( stderr, "c8\n" );
			if ( input.length < curpos + blph.sizex*blph.sizey )
			{
				fprintf( stderr, "Input buffer NULL or Small ( %s ) 4\n", filename );
				return 0;
			}

			uint8_t* tdata = new uint8_t[ ( unsigned int ) size ];
			memcpy( tdata, input.buf + offset, ( size_t ) size );
			//fprintf( stderr, "c9\n" );
			RGBAPix *pic = blp1_convert_paletted_alpha( ( uint8_t* ) tdata, Pal, blph.sizex, blph.sizey );

			/*unsigned long k, i;

			i = blph.sizex*blph.sizey;
			for ( k = 0; k < i; ++k )
			{
				pic[ k ].R = Pal[ tdata[ k ].i ].B;
				pic[ k ].G = Pal[ tdata[ k ].i ].G;
				pic[ k ].B = Pal[ tdata[ k ].i ].R;
				pic[ k ].A = ( unsigned char ) ( 255 - Pal[ tdata[ k ].i ].A );
			}*/
			//	fprintf( stderr, "c10\n" );
			delete[ ] tdata;
			output.length = textureSize;
			output.buf = new char[ textureSize ];
			memcpy( output.buf, pic, textureSize );
			delete[ ] pic;
			//fprintf( stderr, "c11\n" );
			width = ( int ) blph.sizex;
			height = ( int ) blph.sizey;

			return textureSize;
		}
		else
		{
			bpp = 4;

			//	fprintf( stderr, "c8\n" );
			if ( input.length < curpos + blph.sizex*blph.sizey )
			{
				fprintf( stderr, "Input buffer NULL or Small ( %s ) 4\n", filename );
				return 0;
			}

			uint8_t* tdata = new uint8_t[ ( unsigned int ) size ];
			memcpy( tdata, input.buf + offset, ( size_t ) size );
			//fprintf( stderr, "c9\n" );
			RGBAPix *pic = blp1_convert_paletted_no_alpha( ( uint8_t* ) tdata, Pal, blph.sizex, blph.sizey );

			/*unsigned long k, i;

			i = blph.sizex*blph.sizey;
			for ( k = 0; k < i; ++k )
			{
			pic[ k ].R = Pal[ tdata[ k ].i ].B;
			pic[ k ].G = Pal[ tdata[ k ].i ].G;
			pic[ k ].B = Pal[ tdata[ k ].i ].R;
			pic[ k ].A = ( unsigned char ) ( 255 - Pal[ tdata[ k ].i ].A );
			}*/
			//	fprintf( stderr, "c10\n" );
			delete[ ] tdata;
			output.length = textureSize;
			output.buf = new char[ textureSize ];
			memcpy( output.buf, pic, textureSize );
			delete[ ] pic;
			//fprintf( stderr, "c11\n" );
			width = ( int ) blph.sizex;
			height = ( int ) blph.sizey;

			return textureSize;
		}

	}
	// JPEG compressed
	else if ( blph.compress == 0 )
	{
		bpp = 4;
		/*if ( output.buf == NULL )
		{
			width = blph.sizex;
			height = blph.sizey;
			return textureSize;
		}
		*/

		long JPEGHeaderSize;
		memcpy( &JPEGHeaderSize, input.buf + curpos, 4 );
		JPEGHeaderSize = _blp_swap_int32( JPEGHeaderSize );
		curpos += 4;

		Buffer tdata;
		tdata.length = blph.psize[ 0 ] + JPEGHeaderSize;
		tdata.buf = new char[ blph.psize[ 0 ] + JPEGHeaderSize ];
		memcpy( tdata.buf, input.buf + curpos, ( size_t ) JPEGHeaderSize );

		curpos = blph.poffs[ 0 ];
		memcpy( ( tdata.buf + JPEGHeaderSize ), input.buf + curpos, blph.psize[ 0 ] );


		if ( !JPG2Raw( tdata, output, width, height, bpp, filename ) )
		{
			delete[ ] tdata.buf;
			width = 0;
			height = 0;
			return ( 0 );
		}

		delete[ ] tdata.buf;

		// Output should be RGBA, BLPs use BGRA
		textureInvertRBInPlace( ( RGBAPix* ) output.buf, output.length );

		width = ( int ) blph.sizex;
		height = ( int ) blph.sizey;

		return textureSize;
	}

	return 0;
}

bool JPG2Raw( Buffer &input, Buffer &output, int &width, int &height, int &bpp, char const *filename )
{
	width = 0;
	height = 0;
	bpp = 32;
	if ( !DecompressJpg( input, output, width, height, bpp ) )
	{
		fprintf( stderr, "Unsupported JPG format of %s\n", filename );
		return false;
	}
	if ( !IsPowerOfTwo( width ) || !IsPowerOfTwo( height ) )
	{
		fprintf( stderr, "Width or height of %s are not power of two\n", filename );
		delete[ ] output.buf;
		return false;
	}
	if ( bpp != 4 && bpp != 3 )
	{
		fprintf( stderr, "Bits per pixel of %s are not 24 or 32\n", filename );
		delete[ ] output.buf;
		return false;
	}
	return true;
}
