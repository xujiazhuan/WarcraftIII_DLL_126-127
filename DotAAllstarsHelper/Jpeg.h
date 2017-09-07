// Thanks Magos ( http://home.magosx.com/index.php?topic=6.0 ) for JPEG class :)


//+-----------------------------------------------------------------------------
//| Inclusion guard
//+-----------------------------------------------------------------------------
#ifndef MAGOS_JPEG_H
#define MAGOS_JPEG_H


//+-----------------------------------------------------------------------------
//| Prevents stupid redefinitions
//+-----------------------------------------------------------------------------
#define XMD_H


//+-----------------------------------------------------------------------------
//| Included files
//+-----------------------------------------------------------------------------
#include "Buffer.h"
#include "blpaletter.h"

//+-----------------------------------------------------------------------------
//| Included JPEG files (requires C inclusion)
//+-----------------------------------------------------------------------------

#pragma comment(lib,"jpeg-static.lib")
extern "C"
{
#include <jpeglib.h>
}

#define _blp_swap_int16(x) (x)
#define _blp_swap_int32(x) (x)


//+-----------------------------------------------------------------------------
//| Source manager structure
//+-----------------------------------------------------------------------------
struct JPEG_SOURCE_MANAGER
{
	JPEG_SOURCE_MANAGER( )
	{
		SourceBuffer = NULL;
		SourceBufferSize = 0;
		Buffer = NULL;
		Manager = jpeg_source_mgr( );
	}

	jpeg_source_mgr Manager;
	UCHAR* SourceBuffer;
	LONG SourceBufferSize;
	JOCTET* Buffer;
};


//+-----------------------------------------------------------------------------
//| Destination manager structure
//+-----------------------------------------------------------------------------
struct JPEG_DESTINATION_MANAGER
{
	JPEG_DESTINATION_MANAGER( )
	{
		DestinationBuffer = NULL;
		DestinationBufferSize = 0;
		Buffer = NULL;
		Manager = jpeg_destination_mgr( );
	}

	jpeg_destination_mgr Manager;
	UCHAR* DestinationBuffer;
	LONG DestinationBufferSize;
	JOCTET* Buffer;
};


//+-----------------------------------------------------------------------------
//| Jpeg class
//+-----------------------------------------------------------------------------
class JPEG
{
public:
	JPEG( );
	virtual ~JPEG( );

	BOOL Write( Buffer& SourceBuffer, Buffer& TargetBuffer, INT Width, INT Height, INT Quality );
	BOOL Read( Buffer& SourceBuffer, Buffer& TargetBuffer, INT* Width = NULL, INT* Height = NULL );

protected:
	static VOID SetMemorySource( jpeg_decompress_struct* Info, UCHAR* Buffer, ULONG Size );
	static VOID SetMemoryDestination( jpeg_compress_struct* Info, UCHAR* Buffer, ULONG Size );

	static VOID SourceInit( jpeg_decompress_struct* Info );
	static BOOLEAN SourceFill( jpeg_decompress_struct* Info );
	static VOID SourceSkip( jpeg_decompress_struct* Info, LONG NrOfBytes );
	static VOID SourceTerminate( jpeg_decompress_struct* Info );

	static VOID DestinationInit( jpeg_compress_struct* Info );
	static BOOLEAN DestinationEmpty( jpeg_compress_struct* Info );
	static VOID DestinationTerminate( jpeg_compress_struct* Info );
};


//+-----------------------------------------------------------------------------
//| Global objects
//+-----------------------------------------------------------------------------
extern JPEG Jpeg;


//+-----------------------------------------------------------------------------
//| End of inclusion guard
//+-----------------------------------------------------------------------------
#endif
