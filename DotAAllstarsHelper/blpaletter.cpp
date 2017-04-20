#include "blpaletter.h"

bool IsPowerOfTwo( const long i )
{
	long t = i;
	while ( t % 2 == 0 )
		t >>= 1;
	return ( t == 1 );
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


bool MaskOk( unsigned char *mask, int expectedWidth, int expectedHeight, int expectedBpp, long &offset, const char *maskFile )
{
	TGAHeader* header = ( TGAHeader* )mask;
	if ( header->colorMapType != 0 || header->imageType != 2 || header->width == 0 || header->height == 0 )
		return false;

	if ( header->width != expectedWidth || header->height != expectedHeight )
		return false;

	if ( header->bpp / 8 != expectedBpp )
		return false;

	offset = ( long )( sizeof( TGAHeader ) + header->imageIDLength );
	return true;
}

template<typename T>
inline void AssignWeightedPixel( double *target, T *source, double weight, int bytespp, bool add )
{
	if ( !add )
	{
		target[ 0 ] = ( ( double )source[ 0 ] ) * weight;
		target[ 1 ] = ( ( double )source[ 1 ] ) * weight;
		target[ 2 ] = ( ( double )source[ 2 ] ) * weight;
		if ( bytespp == 4 )
			target[ 3 ] = ( ( double )source[ 3 ] ) * weight;
	}
	else
	{
		target[ 0 ] += ( ( double )source[ 0 ] ) * weight;
		target[ 1 ] += ( ( double )source[ 1 ] ) * weight;
		target[ 2 ] += ( ( double )source[ 2 ] ) * weight;
		if ( bytespp == 4 )
			target[ 3 ] += ( ( double )source[ 3 ] ) * weight;
	}
}

inline unsigned char NormalizeComponent( double val )
{
	if ( val < 0.0 )
		return 0;
	if ( val > ( double )0xFF )
		return 0xFF;
	return ( unsigned char )val;
}

void ScaleImage( unsigned char* rawData, int oldW, int oldH, int newW, int newH, int bytespp, Buffer &target )
{
	if ( oldW == newW && oldH == newH )
	{
		target.length = ( unsigned long )( newW * newH * bytespp );
		target.buf = new char[ target.length ];
		memcpy( target.buf, rawData, target.length );
		return;
	}

	// scale horizontally
	double* temp = new double[ ( unsigned int )( oldH * newW * bytespp ) ];


	if ( oldW == newW )
	{
		for ( int i = 0; i < oldW * oldH * bytespp; i++ )
			temp[ i ] = ( double )rawData[ i ];
	}
	else
	{
		double sum = 0;
		double diffW = ( ( double )oldW / ( double )newW );
		for ( int i = 0; i < newW; i++ )
		{
			double newSum = sum + diffW;
			int pix = ( int )floor( sum );
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
	target.length = ( unsigned long )( newW * newH * bytespp );
	target.buf = new char[ target.length ];
	double* final = new double[ target.length ];


	if ( newH == oldH )
	{
		memcpy( final, temp, target.length * sizeof( double ) );
	}
	else
	{
		double sum = 0;
		double diffH = ( ( double )oldH / ( double )newH );
		for ( int j = 0; j < newH; j++ )
		{
			double newSum = sum + diffH;
			int pix = ( int )floor( sum );
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
		target.buf[ i ] = ( char )NormalizeComponent( final[ i ] );
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
	pixel = ( unsigned char )( ( double )pixel * ( ( double )mask / ( double )0xFF ) );
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

bool CreateJpgBLP( Buffer rawData, Buffer &output, int quality, char const *, int width, int height, int bytespp, int alphaflag, int &maxmipmaps )
{
	Buffer target[ 16 ];
	Buffer scaled[ 16 ];
	Buffer source;
	source.buf = rawData.buf;
	source.length = ( unsigned long )( width * height * 4 );
	if ( bytespp < 4 )
	{
		source.buf = new char[ ( unsigned int )( width * height * 4 ) ];
		for ( int j = 0; j < width * height; j++ )
		{
			memcpy( source.buf + j * 4, rawData.buf + j*bytespp, ( size_t )bytespp );
			source.buf[ j * 4 + 3 ] = '\xFF';
		}
	}

	int truemipmaps = GetRequiredMipMaps( width, height );
	BLPHeader blpHeader;
	memcpy( blpHeader.ident, "BLP1", 4 );
	blpHeader.compress = 0; // jpg compression
	blpHeader.IsAlpha = ( uint32_t )alphaflag;
	blpHeader.sizey = ( unsigned long )height;
	blpHeader.sizex = ( unsigned long )width;
	blpHeader.alphaEncoding = ( uint32_t )( !alphaflag ? 5 : 4 ); // BGR or BGRA
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
				ScaleImage( ( unsigned char* )scaled[ i - 1 ].buf, xdimension * 2, ydimension * 2, xdimension, ydimension, 4, scaled[ i ] );
			//if ( !ConvertToJpg( scaled[ i ], target[ i ], xdimension, ydimension, 4, quality, true ) )
			if ( !Jpeg.Write( scaled[ i ], target[ i ], xdimension, ydimension, quality ) )
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
				if ( i > 0 )
				{
					blpHeader.poffs[ i ] = blpHeader.poffs[ i - 1 ];
					blpHeader.psize[ i ] = blpHeader.psize[ i - 1 ];
				}
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

bool CreatePalettedBLP( Buffer rawData, Buffer &output, int colors, char const *, int width, int height, int bytespp, int alphaflag, int &maxmipmaps )
{
	CQuantizer* q = new CQuantizer( ( unsigned int )colors, 8 );
	q->ProcessImage( ( unsigned char* )rawData.buf, ( unsigned long )( width * height ), ( unsigned char )bytespp, 0x00 );
	int truemipmaps = GetRequiredMipMaps( width, height );
	BLPHeader blpHeader;
	memcpy( blpHeader.ident, "BLP1", 4 );
	blpHeader.compress = 1; // paletted
	blpHeader.IsAlpha = 8;
	blpHeader.sizey = ( unsigned long )height;
	blpHeader.sizex = ( unsigned long )width;
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
				ScaleImage( ( unsigned char* )bufs[ i - 1 ].buf, xdimension * 2, ydimension * 2, xdimension, ydimension, bytespp, bufs[ i ] );
			blpHeader.poffs[ i ] = output.length;
			blpHeader.psize[ i ] = ( unsigned long )( xdimension * ydimension * 2 ); //(q->NeedsAlphaChannel() ? 2 : 1);
			output.length += blpHeader.psize[ i ];
		}
		else
		{
			if ( i < truemipmaps )
			{ // war3 requires at least 8 mipmaps for the alpha channel to work
				if ( i > 0 )
				{
					blpHeader.poffs[ i ] = blpHeader.poffs[ i - 1 ];
					blpHeader.psize[ i ] = blpHeader.psize[ i - 1 ];
				}
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

	output.buf = new char[ output.length ];
	unsigned char* blpData = ( unsigned char* )output.buf;
	memcpy( blpData, &blpHeader, sizeof( BLPHeader ) );
	memset( blpData + sizeof( BLPHeader ), 0, sizeof( BGRAPix ) * 256 );
	unsigned char* blp = blpData + sizeof( BLPHeader ) + sizeof( BGRAPix ) * 256;
	BGRAPix *palette = ( BGRAPix* )( blpData + sizeof( BLPHeader ) );
	q->SetColorTable( palette );
	for ( int i = 0; i <= 16; i++ )
	{
		if ( i < maxmipmaps && width > 0 && height > 0 )
		{
			BGRAPix* raw = ( BGRAPix* )bufs[ i ].buf;
			memset( blp, 0xFF, ( size_t )( width * height * 2 ) ); //(q->NeedsAlphaChannel() ? 2 : 1);
			q->FloydSteinbergDither( ( unsigned char* )bufs[ i ].buf, width, height, ( unsigned char )bytespp, blp, palette );
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

bool TGA2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, const char* filename )
{
	TGAHeader* header = ( TGAHeader* )input.buf;
	if ( header->colorMapType != 0 || header->imageType != 2 || header->width == 0 || header->height == 0 )
		return false;

	if ( !IsPowerOfTwo( header->width ) || !IsPowerOfTwo( header->height ) )
		return false;

	if ( header->bpp != 32 && header->bpp != 24 )
		return false;

	bpp = 4;
	if ( header->bpp < 4 )
		bpp = 3;
	width = header->width;
	height = header->height;

	output.length = ( unsigned long )( width*height*bpp );
	output.buf = new char[ output.length ];
	memcpy( output.buf, input.buf + sizeof( TGAHeader ) + header->imageIDLength, output.length );

	return true;
}

bool RAW2Tga( Buffer input, Buffer &output, int width, int height, int bpp, const char* filename )
{
	TGAHeader header;
	memset( &header, 0, sizeof( TGAHeader ) );

	header.imageType = 2;
	header.width = width;
	header.height = height;
	header.bpp = bpp * 8;
	header.imagedescriptor = bpp == 4 ? 8 : 0;
	output.length = sizeof( TGAHeader ) + width * height * bpp;
	output.buf = new char[ output.length ];

	memcpy( output.buf, &header, sizeof( TGAHeader ) );
	memcpy( output.buf + sizeof( TGAHeader ), input.buf, output.length - sizeof( TGAHeader ) );

	return true;
}

bool BMP2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, char const *filename )
{
	BITMAPFILEHEADER *FileHeader = ( BITMAPFILEHEADER* )input.buf;
	BITMAPINFOHEADER *InfoHeader = ( BITMAPINFOHEADER* )( FileHeader + 1 );
	if ( FileHeader->bfType != 0x4D42 )
		return false;

	if ( !IsPowerOfTwo( InfoHeader->biWidth ) || !IsPowerOfTwo( InfoHeader->biHeight ) )
		return false;

	if ( InfoHeader->biBitCount != 32 && InfoHeader->biBitCount != 24 )
		return false;

	bpp = 4;
	if ( InfoHeader->biBitCount < 32 )
		bpp = 3;
	width = InfoHeader->biWidth;
	height = InfoHeader->biHeight;

	output.length = ( unsigned long )( width*height*bpp );
	output.buf = new char[ output.length ];
	memcpy( output.buf, input.buf + FileHeader->bfOffBits, output.length );

	if ( bpp == 4 ) // invert alpha
		for ( int i = 0; i < width*height; i++ )
			output.buf[ i*bpp + 3 ] = 0xFF - output.buf[ i*bpp + 3 ];
	return true;
}

void SwapBLPHeader( BLPHeader *header )
{
	header->compress = ( unsigned long )_blp_swap_int32( header->compress );
	header->IsAlpha = ( unsigned long )_blp_swap_int32( header->IsAlpha );
	header->sizex = ( unsigned long )_blp_swap_int32( header->sizex );
	header->sizey = ( unsigned long )_blp_swap_int32( header->sizey );
	header->alphaEncoding = ( unsigned long )_blp_swap_int32( header->alphaEncoding );
	header->flags2 = ( unsigned long )_blp_swap_int32( header->flags2 );

	int i = 0;
	for ( ; i < 16; i++ )
	{
		( header->poffs )[ i ] = ( unsigned long )_blp_swap_int32( ( header->poffs )[ i ] );
		( header->psize )[ i ] = ( unsigned long )_blp_swap_int32( ( header->psize )[ i ] );
	}
}


void textureInvertRBInPlace( RGBAPix *bufsrc, unsigned long srcsize )
{
	for ( unsigned long i = 0; i < ( srcsize / 4 ); i++ )
	{
		unsigned char red = bufsrc[ i ].B;
		bufsrc[ i ].B = bufsrc[ i ].R;
		bufsrc[ i ].R = red;
	}
}


void flip_vertically( unsigned char *pixels, const size_t width, const size_t height, const size_t bytes_per_pixel )
{
	const size_t stride = width * bytes_per_pixel;
	unsigned char *row = ( unsigned char * )malloc( stride );
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
				pDst->a = ( uint8_t )( 0xFF - *pAlpha );
			else
				pDst->a = *pAlpha;

			++pIndices;
			++pAlpha;
			++pDst;
		}
	}
	flip_vertically( ( unsigned char* )pBuffer, width, height, 4 );

	return pBuffer;
}

RGBAPix * blp1_convert_paletted_separated_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height, bool invertAlpha )
{
	RGBAPix * outrgba = ( RGBAPix * )blp1_convert_paletted_separated_alpha_BGRA( pSrc, ( tBGRAPixel * )pInfos, width, height, invertAlpha );

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
			pDst->a = ( uint8_t )( 0xFF - pDst->a );

			++pIndices;
			++pDst;
		}
	}

	return pBuffer;
}

RGBAPix* blp1_convert_paletted_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height )
{
	RGBAPix * outrgba = ( RGBAPix * )blp1_convert_paletted_alpha_BGRA( pSrc, ( tBGRAPixel* )pInfos, width, height );
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
	flip_vertically( ( unsigned char* )pBuffer, width, height, 4 );

	return pBuffer;
}

RGBAPix* blp1_convert_paletted_no_alpha( uint8_t* pSrc, RGBAPix* pInfos, unsigned int width, unsigned int height )
{
	RGBAPix * outrgba = ( RGBAPix * )blp1_convert_paletted_no_alpha_BGRA( pSrc, ( tBGRAPixel* )pInfos, width, height );
	return outrgba;
}

unsigned long Blp2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, int &mipmaps, int & alphaflag, int & compresstype, int & pictype, char const *filename )
{
	BLPHeader blph;
	bpp = 4;
	width = 0;
	height = 0;
	unsigned long curpos = 0;
	unsigned long textureSize = 0;
	if ( input.buf == NULL || input.length == NULL || input.length < sizeof( BLPHeader ) )
		return 0;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif

	memcpy( &blph, input.buf, sizeof( BLPHeader ) );

	if ( memcmp( blph.ident, "BLP1", 4 ) != 0 )
		return 0;

	mipmaps = 0;
	for ( int i = 0; i < 15; i++ )
	{
		if ( blph.poffs[ i ] > 0 )
		{
			mipmaps++;
		}
	}


	alphaflag = ( int )blph.IsAlpha;


	curpos += sizeof( BLPHeader );
	textureSize = blph.sizex * blph.sizey * 4;
	compresstype = ( int )blph.compress;

	pictype = ( int )blph.alphaEncoding;

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
	if ( blph.compress == 1 )
	{

#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		if ( input.length < curpos + 256 * 4 )
		{
			return 0;
		}

		RGBAPix Pal[ 256 ];
		memcpy( Pal, input.buf + curpos, 256 * 4 );
		curpos += 256 * 4;

		int offset = ( int )blph.poffs[ 0 ];
		int size = ( int )blph.psize[ 0 ];

		if ( alphaflag > 0 && ( blph.alphaEncoding == 4 || blph.alphaEncoding == 3 ) )
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			if ( input.length < curpos + blph.sizex * blph.sizey * 2 )
				return 0;


			uint8_t* tdata = new uint8_t[ ( unsigned int )size ];
			memcpy( tdata, input.buf + offset, ( size_t )size );

			RGBAPix *pic = blp1_convert_paletted_separated_alpha( ( uint8_t* )tdata, Pal, blph.sizex, blph.sizey, 0 );

			delete[ ] tdata;


			output.length = textureSize;
			output.buf = new char[ textureSize ];
			memcpy( output.buf, pic, textureSize );
			delete[ ] pic;

			width = ( int )blph.sizex;
			height = ( int )blph.sizey;

#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			return textureSize;
		}
		else if ( alphaflag > 0 && blph.alphaEncoding == 5 )
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			if ( input.length < curpos + blph.sizex*blph.sizey )
				return 0;



			uint8_t* tdata = new uint8_t[ ( unsigned int )size ];
			memcpy( tdata, input.buf + offset, ( size_t )size );
			RGBAPix *pic = blp1_convert_paletted_alpha( ( uint8_t* )tdata, Pal, blph.sizex, blph.sizey );

			delete[ ] tdata;
			output.length = textureSize;
			output.buf = new char[ textureSize ];
			memcpy( output.buf, pic, textureSize );
			delete[ ] pic;
			width = ( int )blph.sizex;
			height = ( int )blph.sizey;

#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			return textureSize;
		}
		else
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			if ( input.length < curpos + blph.sizex*blph.sizey )
				return 0;



			uint8_t* tdata = new uint8_t[ ( unsigned int )size ];
			memcpy( tdata, input.buf + offset, ( size_t )size );
			RGBAPix *pic = blp1_convert_paletted_no_alpha( ( uint8_t* )tdata, Pal, blph.sizex, blph.sizey );

			delete[ ] tdata;
			output.length = textureSize;
			output.buf = new char[ textureSize ];
			memcpy( output.buf, pic, textureSize );
			delete[ ] pic;
			width = ( int )blph.sizex;
			height = ( int )blph.sizey;

#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			return textureSize;
		}

	}
	// JPEG compressed
	else if ( blph.compress == 0 )
	{

#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		unsigned long JPEGHeaderSize;
		memcpy( &JPEGHeaderSize, input.buf + curpos, 4 );
		JPEGHeaderSize = _blp_swap_int32( JPEGHeaderSize );

		Buffer tempdata;
		tempdata.length = blph.psize[ 0 ] + JPEGHeaderSize;
		tempdata.buf = new char[ blph.psize[ 0 ] + JPEGHeaderSize ];
		memcpy( tempdata.buf, input.buf + curpos + 4, ( size_t )JPEGHeaderSize );




		curpos = blph.poffs[ 0 ];
		memcpy( ( tempdata.buf + JPEGHeaderSize ), input.buf + curpos, blph.psize[ 0 ] );
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		if ( !JPG2Raw( tempdata, output, width, height, bpp, filename ) )
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			tempdata.Clear( );
			width = 0;
			height = 0;

#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
			return ( 0 );
		}
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		tempdata.Clear( );



		// Output should be RGBA, BLPs use BGRA
		//textureInvertRBInPlace( ( RGBAPix* )output.buf, output.length );

		width = ( int )blph.sizex;
		height = ( int )blph.sizey;
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		return textureSize;
	}

	return 0;
}

bool JPG2Raw( Buffer input, Buffer &output, int &width, int &height, int &bpp, char const *filename )
{
	bpp = 4;
	//if ( !DecompressJpg( input, output, width, height, bpp ) )
	if ( !Jpeg.Read( input, output, &width, &height ) )
		return false;

	//if ( !IsPowerOfTwo( width ) || !IsPowerOfTwo( height ) )
	//{
	//	delete[ ] output.buf;
	//	return false;
	//}
	//if ( bpp != 4 && bpp != 3 )
	//{
	//	delete[ ] output.buf;
	//	return false;
	//}
	return true;
}



int ArrayXYtoId( int width, int x, int y )
{
	return  width * y + x;
}