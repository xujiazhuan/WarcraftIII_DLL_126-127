
#include <experimental/filesystem>
#include "Main.h"
extern "C" { FILE __iob_func[ 3 ] = { *stdin,*stdout,*stderr }; }
#pragma comment(lib,"legacy_stdio_definitions.lib")
#include "blpaletter.h"

#define MASK_56 (((u_int64_t)1<<56)-1) /* i.e., (u_int64_t)0xffffffffffffff */

#include "fnv.h"

u_int64_t GetBufHash( const char * data, size_t data_len )
{
	u_int64_t hash;
	hash = fnv_64_buf( ( void * )data, ( size_t )data_len, FNV1_64_INIT );
	hash = ( hash >> 56 ) ^ ( hash & MASK_56 );
	return hash;
}

struct ICONMDLCACHE
{
	u_int64_t hash;
	size_t hashlen;
	char * buf;
	size_t size;
};

vector<ICONMDLCACHE> ICONMDLCACHELIST;
vector<FileRedirectStruct> FileRedirectList;




BOOL GetFromIconMdlCache( string filename, ICONMDLCACHE * iconhelperout )
{
	size_t filelen = filename.length( );
	u_int64_t hash = GetBufHash( filename.c_str( ), filelen );
	for ( ICONMDLCACHE & ih : ICONMDLCACHELIST )
	{
		if ( ih.hashlen == filelen && ih.hash == hash )
		{
			*iconhelperout = ih;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsFileRedirected( string filename )
{
	for ( FileRedirectStruct & DotaRedirectHelp : FileRedirectList )
	{
		if ( filename == DotaRedirectHelp.NewFilePath )
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsMemInCache( int addr )
{
	for ( ICONMDLCACHE & ih : ICONMDLCACHELIST )
	{
		if ( ( int )ih.buf == addr )
			return TRUE;
	}
	return FALSE;
}

void FreeAllIHelpers( )
{
	if ( !ICONMDLCACHELIST.empty( ) )
	{
		for ( ICONMDLCACHE & ih : ICONMDLCACHELIST )
		{
			if ( ih.buf )
				delete[ ] ih.buf;
		}

		ICONMDLCACHELIST.clear( );
	}
	if ( !FileRedirectList.empty( ) )
		FileRedirectList.clear( );

	if ( !FakeFileList.empty( ) )
		FakeFileList.clear( );

	ClearAllRawImages( );
}



void WINAPI SMemZero( LPVOID lpDestination, DWORD dwLength )
{
	DWORD dwPrevLen = dwLength;
	LPDWORD lpdwDestination = ( LPDWORD )lpDestination;
	LPBYTE lpbyDestination;

	dwLength >>= 2;

	while ( dwLength-- )
		*lpdwDestination++ = 0;

	lpbyDestination = ( LPBYTE )lpdwDestination;

	dwLength = dwPrevLen;
	dwLength &= 3;

	while ( dwLength-- )
		*lpbyDestination++ = 0;
}

LPVOID WINAPI SMemAlloc( LPVOID lpvMemory, DWORD dwSize )
{
	LPVOID lpMemory = VirtualAlloc( lpvMemory, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );// malloc( dwSize );
	if ( lpMemory ) SMemZero( lpMemory, dwSize );
	return lpMemory;
}

void WINAPI SMemFree( LPVOID lpvMemory )
{
	if ( lpvMemory ) VirtualFree( lpvMemory, 0, MEM_RELEASE );// Storm_403_org( lpvMemory, "delete", -1, 0 );
}

DWORD WINAPI SMemCopy( LPVOID lpDestination, LPCVOID lpSource, DWORD dwLength )
{
	DWORD dwPrevLen = dwLength;
	LPDWORD lpdwDestination = ( LPDWORD )lpDestination, lpdwSource = ( LPDWORD )lpSource;
	LPBYTE lpbyDestination, lpbySource;

	dwLength >>= 2;

	while ( dwLength-- )
		*lpdwDestination++ = *lpdwSource++;

	lpbyDestination = ( LPBYTE )lpdwDestination;
	lpbySource = ( LPBYTE )lpdwSource;

	dwLength = dwPrevLen;
	dwLength &= 3;

	while ( dwLength-- )
		*lpbyDestination++ = *lpbySource++;

	return dwPrevLen;
}


char MPQFilePath[ 4000 ];

const char * DisabledIconSignature = "Disabled\\DIS";
const char * DisabledIconSignature2 = "Disabled\\DISDIS";
const char * CommandButtonsDisabledIconSignature = "CommandButtonsDisabled\\DIS";


BOOL replaceAll( std::string& str, const std::string& from, const std::string& to )
{
	BOOL Replaced = FALSE;
	if ( from.empty( ) )
		return Replaced;
	size_t start_pos = 0;
	while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos )
	{
		str.replace( start_pos, from.length( ), to );
		start_pos += to.length( );
		Replaced = TRUE;
	}
	return Replaced;
}


GameGetFile GameGetFile_org = NULL;
GameGetFile GameGetFile_ptr;

int idddd = 0;

void ApplyTerrainFilter( string filename, int * OutDataPointer, size_t * OutSize, BOOL IsTga )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "ApplyTerrainFilter" );
#endif


	char * originfiledata = ( char * )( int )*OutDataPointer;
	size_t sz = *OutSize;


	int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 8, compress = 1, alphaenconding = 5;
	unsigned long rawImageSize = 0;
	Buffer InBuffer;
	InBuffer.buf = ( char* )originfiledata;
	InBuffer.length = sz;
	Buffer OutBuffer;
	if ( IsTga )
		rawImageSize = ( unsigned long )TGA2Raw( InBuffer, OutBuffer, w, h, bpp, filename.c_str( ) );
	else
		rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename.c_str( ) );
	if ( rawImageSize > 0 )
	{
		BGRAPix * OutImage = ( BGRAPix* )OutBuffer.buf;
		for ( unsigned long i = 0; i < OutBuffer.length / 4; i++ )
		{
			if ( /*OutImage[ i ].A == 0xFF && */( OutImage[ i ].G > 40 || OutImage[ i ].B > 40 || OutImage[ i ].R > 40 ) )
			{
				if ( OutImage[ i ].R < 235 && OutImage[ i ].G < 235 && OutImage[ i ].B < 235 )
				{
					OutImage[ i ].R += 25;
					OutImage[ i ].G += 25;
					OutImage[ i ].B += 25;
				}
				OutImage[ i ].R = 0;
				OutImage[ i ].G = 0;
				OutImage[ i ].B = 0;
			}
			else if ( true /*OutImage[ i ].A == 0xFF*/ )
			{
				if ( OutImage[ i ].R > 0 && OutImage[ i ].R < 250 )
					OutImage[ i ].R += 5;
				if ( OutImage[ i ].G > 0 && OutImage[ i ].G < 250 )
					OutImage[ i ].G += 5;
				if ( OutImage[ i ].B > 0 && OutImage[ i ].B < 250 )
					OutImage[ i ].B += 5;

				OutImage[ i ].R = 170;
				OutImage[ i ].G = 170;
				OutImage[ i ].B = 170;
				OutImage[ i ].R = 0;
				OutImage[ i ].G = 0;
				OutImage[ i ].B = 0;
			}
		}


		Buffer ResultBuffer;

		CreatePalettedBLP( OutBuffer, ResultBuffer, 256, filename.c_str( ), w, h, bpp, alphaflag, mipmaps );

		if ( OutBuffer.buf != NULL )
		{
			OutBuffer.length = 0;
			delete[ ] OutBuffer.buf;
			OutBuffer.buf = 0;
		}

		if ( ResultBuffer.buf != NULL )
		{

			ICONMDLCACHE tmpih;
			tmpih.buf = ResultBuffer.buf;
			tmpih.size = ResultBuffer.length;
			tmpih.hashlen = filename.length( );
			tmpih.hash = GetBufHash( filename.c_str( ), tmpih.hashlen );
			ICONMDLCACHELIST.push_back( tmpih );
			if ( !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* )*OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int )tmpih.buf;
			*OutSize = tmpih.size;


		}
	}

}



int __stdcall ApplyTerrainFilterDirectly( char * filename, int * OutDataPointer, size_t * OutSize, BOOL IsTga )
{
	ApplyTerrainFilter( filename, OutDataPointer, OutSize, IsTga );
	return 0;
}


void ApplyIconFilter( string filename, int * OutDataPointer, size_t * OutSize )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "ApplyIconFilter" );
#endif

	char * originfiledata = ( char * )( int )*OutDataPointer;
	size_t sz = *OutSize;


	int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 0, compress = 0, alphaenconding = 0;
	unsigned long rawImageSize = 0;
	Buffer InBuffer;
	InBuffer.buf = ( char* )originfiledata;
	InBuffer.length = sz;
	Buffer OutBuffer;

	rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename.c_str( ) );
	if ( rawImageSize > 0 )
	{
		BGRAPix * OutImage = ( BGRAPix* )OutBuffer.buf;
		BGRAPix BlackPix;

		BlackPix.A = 0xFF;
		BlackPix.R = 0;
		BlackPix.G = 0;
		BlackPix.B = 0;

		for ( int x = 0; x < 4; x++ )
		{
			for ( int y = 0; y < 64; y++ )
			{
				OutImage[ x * 64 + y ] = BlackPix;//верх
				OutImage[ y * 64 + x ] = BlackPix;//лево
				OutImage[ ( 63 - x ) * 64 + y ] = BlackPix;//низ
				OutImage[ y * 64 + 63 - x ] = BlackPix;//право
			}
		}

		for ( int x = 4; x < 60; x++ )
		{
			for ( int y = 4; y < 60; y++ )
			{
				int id = x * 64 + y;
				int ave = ( min( min( OutImage[ id ].R, OutImage[ id ].G ), OutImage[ id ].B ) + max( max( OutImage[ id ].R, OutImage[ id ].G ), OutImage[ id ].B ) ) / 2;
				OutImage[ id ].R = FixBounds( ( ave + OutImage[ id ].R ) / 4 );
				OutImage[ id ].G = FixBounds( ( ave + OutImage[ id ].G ) / 4 );
				OutImage[ id ].B = FixBounds( ( ave + OutImage[ id ].B ) / 4 );
			}
		}

		//градиентные рамки
		//8 полос градиента
		for ( int x = 4; x < 12; x++ )
		{
			for ( int y = x; y < 64 - x; y++ )
			{
				double colorfix = ( x - 3.0 ) / 9.0;

				OutImage[ x * 64 + y ].R = FixBounds( colorfix * OutImage[ x * 64 + y ].R );//верх
				OutImage[ x * 64 + y ].G = FixBounds( colorfix * OutImage[ x * 64 + y ].G );//верх
				OutImage[ x * 64 + y ].B = FixBounds( colorfix * OutImage[ x * 64 + y ].B );//верх

				OutImage[ y * 64 + x ].R = FixBounds( colorfix * OutImage[ y * 64 + x ].R );//лево
				OutImage[ y * 64 + x ].G = FixBounds( colorfix * OutImage[ y * 64 + x ].G );//лево
				OutImage[ y * 64 + x ].B = FixBounds( colorfix * OutImage[ y * 64 + x ].B );//лево

				OutImage[ ( 63 - x ) * 64 + y ].R = FixBounds( colorfix * OutImage[ ( 63 - x ) * 64 + y ].R );//низ
				OutImage[ ( 63 - x ) * 64 + y ].G = FixBounds( colorfix * OutImage[ ( 63 - x ) * 64 + y ].G );//низ
				OutImage[ ( 63 - x ) * 64 + y ].B = FixBounds( colorfix * OutImage[ ( 63 - x ) * 64 + y ].B );//низ

				OutImage[ y * 64 + 63 - x ].R = FixBounds( colorfix * OutImage[ y * 64 + 63 - x ].R );//право
				OutImage[ y * 64 + 63 - x ].G = FixBounds( colorfix * OutImage[ y * 64 + 63 - x ].G );//право
				OutImage[ y * 64 + 63 - x ].B = FixBounds( colorfix * OutImage[ y * 64 + 63 - x ].B );//право
			}
		}

		Buffer ResultBuffer;

		CreatePalettedBLP( OutBuffer, ResultBuffer, 256, filename.c_str( ), w, h, bpp, alphaflag, mipmaps );

		OutBuffer.Clear( );

		if ( ResultBuffer.buf != NULL )
		{
			ICONMDLCACHE tmpih;
			tmpih.buf = ResultBuffer.buf;
			tmpih.size = ResultBuffer.length;
			tmpih.hashlen = filename.length( );
			tmpih.hash = GetBufHash( filename.c_str( ), tmpih.hashlen );
			ICONMDLCACHELIST.push_back( tmpih );
			if ( !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* )*OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int )tmpih.buf;
			*OutSize = tmpih.size;
		}
	}

}


void ApplyIconFrameFilter( string filename, int * OutDataPointer, size_t * OutSize );


void ApplyTestFilter( string filename, int * OutDataPointer, size_t * OutSize )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "ApplyIconFilter" );
#endif

	ICONMDLCACHE tmpih;


	char * originfiledata = ( char * )( int )*OutDataPointer;
	size_t sz = *OutSize;


	int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 0, compress = 0, alphaenconding = 0;
	unsigned long rawImageSize = 0;
	Buffer InBuffer;
	InBuffer.buf = ( char* )originfiledata;
	InBuffer.length = sz;
	Buffer OutBuffer;

	rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename.c_str( ) );
	if ( rawImageSize > 0 && w > 9 && h > 9 )
	{
		BGRAPix * OutImage = ( BGRAPix* )OutBuffer.buf;
		BGRAPix BlackPix;

		BlackPix.A = 0xFF;
		BlackPix.R = 70;
		BlackPix.G = 70;
		BlackPix.B = 70;

		bool FoundTransparentTexture = false;

		int id = 0;
		vector<BGRAPix> BGRAPixList;
		BGRAPixList.assign( &OutImage[ 0 ], &OutImage[ w*h - 1 ] );


		for ( BGRAPix & pix : BGRAPixList )
		{
			int R = pix.R;
			int G = pix.G;
			int B = pix.B;

			//pix.G = FixBounds( ( max( max( pix.R, pix.G ), pix.B ) + min( min( pix.R, pix.G ), pix.B ) ) / 2 );
			//pix.B = FixBounds( ( max( max( pix.R, pix.G ), pix.B ) + min( min( pix.R, pix.G ), pix.B ) ) / 2 );
			//pix.R = FixBounds( ( max( max( pix.R, pix.G), pix.B ) + min( min( pix.R, pix.G), pix.B ) ) / 2 );

			pix.R = FixBounds( 0.2126*R + 0.7152*G + 0.0722*B );
			pix.G = FixBounds( 0.2126*R + 0.7152*G + 0.0722*B );
			pix.B = FixBounds( 0.2126*R + 0.7152*G + 0.0722*B );

			/*pix.R = FixBounds( ( pix.R * .393 ) + ( pix.G *.769 ) + ( pix.B * .189 ) );
			pix.G = FixBounds( ( pix.R * .349 ) + ( pix.G *.686 ) + ( pix.B * .168 ) );
			pix.B = FixBounds( ( pix.R * .272 ) + ( pix.G *.534 ) + ( pix.B * .131 ) );
			*/


			id++;
		}

		memcpy( &OutImage[ 0 ], &BGRAPixList[ 0 ], 4 * w*h - 4 );

		/*

		for ( int x = 0; x < h; x++ )
		{
			for ( int y = 0; y < w; x++ )
			{
				BlackPix.A = OutImage[ x * h + y ].A;
				if ( BlackPix.A < 0xFF )
				{
					FoundTransparentTexture = true;
					break;
				}
			}
		}


		if ( !FoundTransparentTexture )
			std::fill( &OutImage[ 0 ], &OutImage[ h * w - 1 ], BlackPix );

		BlackPix.R = 0;
		BlackPix.G = 0;
		BlackPix.B = 0;
		*/
		//
		//for ( int x = 0; x < h; x++ )
		//{
		//	for ( int y = 0; y < w; x++ )
		//	{
		//		BlackPix.A = OutImage[ x * h + y ].A;
		//		OutImage[ x * h + y ] = BlackPix;//верх
		//	}
		//}

		Buffer ResultBuffer;

		CreatePalettedBLP( OutBuffer, ResultBuffer, 256, filename.c_str( ), w, h, bpp, alphaflag, mipmaps );

		if ( OutBuffer.buf != NULL )
		{
			OutBuffer.length = 0;
			delete[ ] OutBuffer.buf;
			OutBuffer.buf = 0;
		}

		if ( ResultBuffer.buf != NULL )
		{
			tmpih.buf = ResultBuffer.buf;
			tmpih.size = ResultBuffer.length;
			tmpih.hashlen = filename.length( );
			tmpih.hash = GetBufHash( filename.c_str( ), tmpih.hashlen );
			ICONMDLCACHELIST.push_back( tmpih );
			if ( !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* )*OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int )tmpih.buf;
			*OutSize = tmpih.size;
		}
	}

}

BOOL FixDisabledIconPath( string _filename, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{
	string filename = _filename;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "FixDisabledIconPath" );
#endif

	BOOL CreateDarkIcon = FALSE;
	BOOL result = FALSE;
	if ( filename.find( DisabledIconSignature2 ) != string::npos )
	{
		if ( replaceAll( filename, DisabledIconSignature2, "\\" ) )
		{
			result = GameGetFile_ptr( filename.c_str( ), OutDataPointer, OutSize, unknown );
			if ( result )
				CreateDarkIcon = TRUE;
		}
	}


	if ( !result )
	{
		filename = _filename;
		if ( filename.find( DisabledIconSignature ) != string::npos )
		{
			if ( replaceAll( filename, DisabledIconSignature, "\\" ) )
			{
				result = GameGetFile_ptr( filename.c_str( ), OutDataPointer, OutSize, unknown );
				if ( result )
					CreateDarkIcon = TRUE;
			}
		}
	}

	if ( !result )
	{
		filename = _filename;
		if ( filename.find( DisabledIconSignature ) != string::npos )
		{
			if ( replaceAll( filename, CommandButtonsDisabledIconSignature, "PassiveButtons\\" ) )
			{
				result = GameGetFile_ptr( filename.c_str( ), OutDataPointer, OutSize, unknown );
				if ( result )
					CreateDarkIcon = TRUE;
			}
		}
	}


	if ( !result )
	{
		filename = _filename;
		if ( filename.find( DisabledIconSignature ) != string::npos )
		{
			if ( replaceAll( filename, CommandButtonsDisabledIconSignature, "AutoCastButtons\\" ) )
			{
				result = GameGetFile_ptr( filename.c_str( ), OutDataPointer, OutSize, unknown );
				if ( result )
					CreateDarkIcon = TRUE;
			}
		}
	}


	if ( CreateDarkIcon )
	{
		ApplyIconFilter( _filename, OutDataPointer, OutSize );
	}
	//else MessageBoxA( 0, filename, "Bad file path:", 0 );

	return result;
}




vector<ModelCollisionFixStruct> ModelCollisionFixList;
vector<ModelTextureFixStruct> ModelTextureFixList;
vector<ModelPatchStruct> ModelPatchList;
vector<ModelRemoveTagStruct> ModelRemoveTagList;
vector<ModelSequenceReSpeedStruct> ModelSequenceReSpeedList;
vector<ModelSequenceValueStruct> ModelSequenceValueList;
vector<ModelScaleStruct> ModelScaleList;

int __stdcall FixModelCollisionSphere( const char * mdlpath, float X, float Y, float Z, float Radius )
{
	ModelCollisionFixStruct tmpModelFix;
	tmpModelFix.FilePath = mdlpath;
	tmpModelFix.X = X;
	tmpModelFix.Y = Y;
	tmpModelFix.Z = Z;
	tmpModelFix.Radius = Radius;
	ModelCollisionFixList.push_back( tmpModelFix );
	return 0;
}


int __stdcall FixModelTexturePath( const char * mdlpath, int textureid, const char * texturenew )
{
	ModelTextureFixStruct tmpModelFix;
	tmpModelFix.FilePath = mdlpath;
	tmpModelFix.TextureID = textureid;
	tmpModelFix.NewTexturePath = texturenew;
	ModelTextureFixList.push_back( tmpModelFix );
	return 0;
}



int __stdcall PatchModel( const char * mdlpath, const char * pathPatch )
{
	ModelPatchStruct tmpModelFix;
	tmpModelFix.FilePath = mdlpath;
	tmpModelFix.patchPath = pathPatch;
	ModelPatchList.push_back( tmpModelFix );
	return 0;
}

int __stdcall RemoveTagFromModel( const char * mdlpath, const char * tagname )
{
	ModelRemoveTagStruct tmpModelFix;
	tmpModelFix.FilePath = mdlpath;
	tmpModelFix.TagName = tagname;
	ModelRemoveTagList.push_back( tmpModelFix );
	return 0;
}

int __stdcall ChangeAnimationSpeed( const char * mdlpath, const char * SeqenceName, float Speed )
{
	ModelSequenceReSpeedStruct tmpModelFix;
	tmpModelFix.FilePath = mdlpath;
	tmpModelFix.AnimationName = SeqenceName;
	tmpModelFix.SpeedUp = Speed;
	ModelSequenceReSpeedList.push_back( tmpModelFix );
	return 0;
}



int __stdcall SetSequenceValue( const char * mdlpath, const char * SeqenceName, int Indx, float Value )
{
	if ( Indx < 0 || Indx > 6 )
		return -1;

	ModelSequenceValueStruct tmpModelFix;
	tmpModelFix.FilePath = mdlpath;
	tmpModelFix.AnimationName = SeqenceName;
	tmpModelFix.Indx = Indx;
	tmpModelFix.Value = Value;
	ModelSequenceValueList.push_back( tmpModelFix );
	return 0;
}


int __stdcall SetModelScale( const char * mdlpath, float Scale )
{
	ModelScaleStruct tmpModelFix;
	tmpModelFix.FilePath = mdlpath;
	tmpModelFix.Scale = Scale;
	ModelScaleList.push_back( tmpModelFix );
	return 0;
}



struct Mdx_Texture        //NrOfTextures = ChunkSize / 268
{
	int ReplaceableId;
	CHAR FileName[ 260 ];
	unsigned int Flags;                       //#1 - WrapWidth
									   //#2 - WrapHeight
};

struct Mdx_FLOAT3
{
	float x;
	float y;
	float z;
};

struct Mdx_Sequence      //NrOfSequences = ChunkSize / 132
{
	CHAR Name[ 80 ];

	int IntervalStart;
	int IntervalEnd;
	FLOAT MoveSpeed;
	unsigned int Flags;                       //0 - Looping
									   //1 - NonLooping
	FLOAT Rarity;
	unsigned int SyncPoint;

	FLOAT BoundsRadius;
	Mdx_FLOAT3 MinimumExtent;
	Mdx_FLOAT3 MaximumExtent;
};

vector<BYTE> FullPatchData;

struct Mdx_SequenceTime
{
	int * IntervalStart;
	int * IntervalEnd;
};



struct Mdx_Track
{
	int NrOfTracks;
	int InterpolationType;             //0 - None
										 //1 - Linear
										 //2 - Hermite
										 //3 - Bezier
	unsigned int GlobalSequenceId;
};

struct Mdx_Tracks
{
	int NrOfTracks;
	unsigned int GlobalSequenceId;

};
struct Mdx_Node
{
	unsigned int InclusiveSize;

	CHAR Name[ 80 ];

	unsigned int ObjectId;
	unsigned int ParentId;
	unsigned int Flags;                         //0        - Helper
										 //#1       - DontInheritTranslation
										 //#2       - DontInheritRotation
										 //#4       - DontInheritScaling
										 //#8       - Billboarded
										 //#16      - BillboardedLockX
										 //#32      - BillboardedLockY
										 //#64      - BillboardedLockZ
										 //#128     - CameraAnchored
										 //#256     - Bone
										 //#512     - Light
										 //#1024    - EventObject
										 //#2048    - Attachment
										 //#4096    - ParticleEmitter
										 //#8192    - CollisionShape
										 //#16384   - RibbonEmitter
										 //#32768   - Unshaded / EmitterUsesMdl
										 //#65536   - SortPrimitivesFarZ / EmitterUsesTga
										 //#131072  - LineEmitter
										 //#262144  - Unfogged
										 //#524288  - ModelSpace
										 //#1048576 - XYQuad
};

struct Mdx_GeosetAnimation
{
	unsigned int InclusiveSize;

	FLOAT Alpha;
	unsigned int Flags;                       //#1 - DropShadow
									   //#2 - Color
	Mdx_FLOAT3 Color;

	unsigned int GeosetId;

};

void ProcessNodeAnims( BYTE * ModelBytes, size_t _offset, vector<int *> & TimesForReplace )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "ProcessNodeAnims" );
#endif
	Mdx_Track tmpTrack;
	size_t offset = _offset;
	if ( memcmp( &ModelBytes[ offset ], "KGTR", 4 ) == 0 )
	{
		offset += 4;
		memcpy( &tmpTrack, &ModelBytes[ offset ], sizeof( Mdx_Track ) );
		offset += sizeof( Mdx_Track );
		for ( int i = 0; i < tmpTrack.NrOfTracks; i++ )
		{
			TimesForReplace.push_back( ( int * )&ModelBytes[ offset ] );
			offset += ( tmpTrack.InterpolationType > 1 ? 40 : 16 );
		}
	}

	if ( memcmp( &ModelBytes[ offset ], "KGRT", 4 ) == 0 )
	{
		offset += 4;
		memcpy( &tmpTrack, &ModelBytes[ offset ], sizeof( Mdx_Track ) );
		offset += sizeof( Mdx_Track );
		for ( int i = 0; i < tmpTrack.NrOfTracks; i++ )
		{
			TimesForReplace.push_back( ( int * )&ModelBytes[ offset ] );
			offset += ( tmpTrack.InterpolationType > 1 ? 52 : 20 );
		}
	}

	if ( memcmp( &ModelBytes[ offset ], "KGSC", 4 ) == 0 )
	{
		offset += 4;
		memcpy( &tmpTrack, &ModelBytes[ offset ], sizeof( Mdx_Track ) );
		offset += sizeof( Mdx_Track );
		for ( int i = 0; i < tmpTrack.NrOfTracks; i++ )
		{
			TimesForReplace.push_back( ( int * )&ModelBytes[ offset ] );
			offset += ( tmpTrack.InterpolationType > 1 ? 40 : 16 );
		}
	}


	if ( memcmp( &ModelBytes[ offset ], "KGAO", 4 ) == 0 )
	{
		offset += 4;
		memcpy( &tmpTrack, &ModelBytes[ offset ], sizeof( Mdx_Track ) );
		offset += sizeof( Mdx_Track );
		for ( int i = 0; i < tmpTrack.NrOfTracks; i++ )
		{
			TimesForReplace.push_back( ( int * )&ModelBytes[ offset ] );
			offset += ( tmpTrack.InterpolationType > 1 ? 16 : 8 );
		}
	}

	if ( memcmp( &ModelBytes[ offset ], "KGAC", 4 ) == 0 )
	{
		offset += 4;
		memcpy( &tmpTrack, &ModelBytes[ offset ], sizeof( Mdx_Track ) );
		offset += sizeof( Mdx_Track );
		for ( int i = 0; i < tmpTrack.NrOfTracks; i++ )
		{
			TimesForReplace.push_back( ( int * )&ModelBytes[ offset ] );
			offset += ( tmpTrack.InterpolationType > 1 ? 16 : 8 );
		}
	}

}


BYTE HelperBytesPart1[ ] = {
							0x42,0x4F,0x4E,0x45,0x88,0x00,0x00,0x00,0x80,0x00,
							0x00,0x00,0x42,0x6F,0x6E,0x65,0x5F,0x52,0x6F,0x6F,
							0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00 };
BYTE HelperBytesPart2[ ] = { 0xFF,0xFF,0xFF,0xFF,0x00,0x01,0x00,0x00,0x4B,
							0x47,0x53,0x43,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
							0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

BYTE HelperBytesPart3[ ] = { 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF };


void ProcessMdx( string filename, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "ProcessModel" );
#endif
	BYTE * ModelBytes = ( BYTE* )*OutDataPointer;
	size_t sz = *OutSize;



	for ( unsigned int i = 0; i < ModelSequenceValueList.size( ); i++ )
	{
		ModelSequenceValueStruct mdlfix = ModelSequenceValueList[ i ];
		if ( filename == mdlfix.FilePath )
		{
			size_t offset = 0;
			if ( memcmp( &ModelBytes[ offset ], "MDLX", 4 ) == 0 )
			{
				offset += 4;
				while ( offset < sz )
				{
					if ( memcmp( &ModelBytes[ offset ], "SEQS", 4 ) == 0 )
					{
						Mdx_Sequence tmpSequence;

						offset += 4;

						size_t currenttagsize = *( size_t* )&ModelBytes[ offset ];
						size_t SequencesCount = currenttagsize / sizeof( Mdx_Sequence );

						size_t newoffset = offset + currenttagsize;
						offset += 4;
						while ( SequencesCount > 0 )
						{
							SequencesCount--;
							memcpy( &tmpSequence, &ModelBytes[ offset ], sizeof( Mdx_Sequence ) );

							if ( mdlfix.AnimationName.length( ) == 0 || mdlfix.AnimationName == tmpSequence.Name )
							{
								size_t NeedPatchOffset = offset + 104 + ( mdlfix.Indx * 4 );
								*( float* )&ModelBytes[ NeedPatchOffset ] = mdlfix.Value;
							}

							offset += sizeof( Mdx_Sequence );
						}
						offset = newoffset;
					}
					else
					{
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
					}

					offset += 4;
				}

			}


			if ( IsKeyPressed( '0' ) && FileExist( ".\\Test1234.mdx" ) )
			{
				FILE *f;
				fopen_s( &f, ".\\Test1234.mdx", "wb" );
				fwrite( ModelBytes, sz, 1, f );
				fclose( f );
				MessageBoxA( 0, "Ok dump", "DUMP", 0 );
			}


			ModelSequenceValueList.erase( ModelSequenceValueList.begin( ) + ( int )i );

		}

	}

	for ( unsigned int i = 0; i < ModelSequenceReSpeedList.size( ); i++ )
	{
		ModelSequenceReSpeedStruct mdlfix = ModelSequenceReSpeedList[ i ];
		if ( filename == mdlfix.FilePath )
		{

			int SequenceID = 0;
			int ReplaceSequenceID = -1;

			// First find Animation and shift others
			vector<Mdx_SequenceTime> Sequences;

			// Next find all objects with Node struct and shift 
			vector<int *> TimesForReplace;

			// Shift any others animations
			// Next need search and shift needed animation
			size_t offset = 0;
			if ( memcmp( &ModelBytes[ offset ], "MDLX", 4 ) == 0 )
			{
				offset += 4;
				while ( offset < sz )
				{
					if ( memcmp( &ModelBytes[ offset ], "SEQS", 4 ) == 0 )
					{

						Mdx_Sequence tmpSequence;

						offset += 4;

						size_t currenttagsize = *( size_t* )&ModelBytes[ offset ];
						size_t SequencesCount = currenttagsize / sizeof( Mdx_Sequence );

						size_t newoffset = offset + currenttagsize;
						offset += 4;
						while ( SequencesCount > 0 )
						{
							SequencesCount--;
							memcpy( &tmpSequence, &ModelBytes[ offset ], sizeof( Mdx_Sequence ) );


							if ( mdlfix.AnimationName == tmpSequence.Name )
							{
								ReplaceSequenceID = SequenceID;
							}


							Mdx_SequenceTime CurrentSequenceTime;
							CurrentSequenceTime.IntervalStart = ( int* )&ModelBytes[ offset + 80 ];
							CurrentSequenceTime.IntervalEnd = ( int* )&ModelBytes[ offset + 84 ];
							Sequences.push_back( CurrentSequenceTime );

							offset += sizeof( Mdx_Sequence );
							SequenceID++;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "BONE", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += tmpNode.InclusiveSize + 8;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "HELP", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += tmpNode.InclusiveSize;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "LITE", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "ATCH", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "PREM", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "PRE2", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "GEOA", 4 ) == 0 )
					{
						Mdx_GeosetAnimation tmpGeosetAnimation;
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						while ( newoffset > offset )
						{
							memcpy( &tmpGeosetAnimation, &ModelBytes[ offset ], sizeof( Mdx_GeosetAnimation ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_GeosetAnimation ), TimesForReplace );

							offset += tmpGeosetAnimation.InclusiveSize;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "RIBB", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "EVTS", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						Mdx_Tracks tmpTracks;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += tmpNode.InclusiveSize;
							if ( memcmp( &ModelBytes[ offset ], "KEVT", 4 ) == 0 )
							{
								offset += 4;
								memcpy( &tmpTracks, &ModelBytes[ offset ], sizeof( Mdx_Tracks ) );
								offset += sizeof( Mdx_Tracks );
								for ( int n = 0; n < tmpTracks.NrOfTracks; n++ )
								{
									TimesForReplace.push_back( ( int* )&ModelBytes[ offset ] );
									offset += 4;
								}
							}
							else offset += 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "CLID", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += tmpNode.InclusiveSize;
							unsigned int size_of_this_struct = *( unsigned int* )&ModelBytes[ offset ];
							offset += 4;
							size_of_this_struct = size_of_this_struct == 0 ? 24u : 16u;
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else
					{
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
					}

					offset += 4;
				}
			}

			if ( ReplaceSequenceID != -1 )
			{

				int SeqEndTime = *Sequences[ ( unsigned int )ReplaceSequenceID ].IntervalEnd;
				int SeqStartTime = *Sequences[ ( unsigned int )ReplaceSequenceID ].IntervalStart;
				int NewEndTime = SeqStartTime + ( int )( ( SeqEndTime - SeqStartTime ) / mdlfix.SpeedUp );
				int AddTime = NewEndTime - SeqEndTime;

				for ( unsigned int n = 0; n < Sequences.size( ); n++ )
				{
					if ( *Sequences[ n ].IntervalStart >= SeqEndTime )
					{
						*Sequences[ n ].IntervalStart += AddTime;
						*Sequences[ n ].IntervalEnd += AddTime;
					}
				}

				*Sequences[ ( unsigned int )ReplaceSequenceID ].IntervalEnd = NewEndTime;

				for ( int * dwTime : TimesForReplace )
				{
					if ( *dwTime >= SeqEndTime )
					{
						*dwTime += AddTime;
					}
					else if ( *dwTime <= SeqEndTime && *dwTime >= SeqStartTime )
					{
						*dwTime = ( int )SeqStartTime + ( int )( ( float )( *dwTime - SeqStartTime ) / mdlfix.SpeedUp );
					}
				}


				if ( IsKeyPressed( '0' ) && FileExist( ".\\Test1234.mdx" ) )
				{
					FILE *f;
					fopen_s( &f, ".\\Test1234.mdx", "wb" );
					fwrite( ModelBytes, sz, 1, f );
					fclose( f );
					MessageBoxA( 0, "Ok dump", "DUMP", 0 );
				}

			}

			if ( !TimesForReplace.empty( ) )
				TimesForReplace.clear( );
			if ( !Sequences.empty( ) )
				Sequences.clear( );

			ModelSequenceReSpeedList.erase( ModelSequenceReSpeedList.begin( ) + ( int )i );

		}

	}


	for ( unsigned int i = 0; i < ModelRemoveTagList.size( ); i++ )
	{
		ModelRemoveTagStruct mdlfix = ModelRemoveTagList[ i ];
		if ( filename == mdlfix.FilePath )
		{
			BOOL TagFound = FALSE;
			size_t TagStartOffset = 0;
			size_t TagSize = 0;
			size_t offset = 0;
			if ( memcmp( &ModelBytes[ offset ], "MDLX", 4 ) == 0 )
			{
				offset += 4;
				while ( offset < sz )
				{
					if ( memcmp( &ModelBytes[ offset ], mdlfix.TagName.c_str( ), 4 ) == 0 )
					{

						TagFound = TRUE;
						TagStartOffset = offset;
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
						TagSize = offset - TagStartOffset;

					}
					else
					{
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
					}

					offset += 4;
				}

			}

			if ( TagFound )
			{
				memcpy( &ModelBytes[ TagStartOffset ], &ModelBytes[ TagStartOffset + TagSize + 4 ], sz - ( TagStartOffset + TagSize ) );
				memset( &ModelBytes[ sz - TagSize - 4 ], 0xFF, TagSize );

				sz = sz - TagSize - 4;
				*OutSize = sz;
			}


			ModelRemoveTagList.erase( ModelRemoveTagList.begin( ) + ( int )i );

		}
	}


	for ( unsigned int i = 0; i < ModelScaleList.size( ); i++ )
	{
		ModelScaleStruct mdlfix = ModelScaleList[ i ];
		if ( filename == mdlfix.FilePath )
		{
			if ( !FullPatchData.empty( ) )
				FullPatchData.clear( );

			char TagName[ 5 ];
			memset( TagName, 0, 5 );
			size_t offset = 0;

			DWORD MaxObjectId = 0;

			std::vector<DWORD *> parents;

			DWORD OffsetToInsertPivotPoint = 0;

			BOOL FoundGLBS = FALSE;
			char * strGLBS = "GLBS";

			if ( memcmp( &ModelBytes[ offset ], "MDLX", 4 ) == 0 )
			{
				offset += 4;
				while ( offset < sz )
				{
					memcpy( TagName, &ModelBytes[ offset ], 4 );
					if ( memcmp( &ModelBytes[ offset ], strGLBS, 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						FoundGLBS = TRUE;

						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "PIVT", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];

						*( size_t* )&ModelBytes[ offset ] = 12 + *( size_t* )&ModelBytes[ offset ];

						OffsetToInsertPivotPoint = newoffset + 4;

						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "BONE", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;

						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );

							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );

							offset += tmpNode.InclusiveSize + 8;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "HELP", 4 ) == 0 )
					{

						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );


							offset += tmpNode.InclusiveSize;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "LITE", 4 ) == 0 )
					{

						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );

							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "ATCH", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						//Mdx_Tracks tmpTracks;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;

							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );

							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );
							//*( DWORD* )&ModelBytes[ offset + 88 ] = 0xFFFFFFFF;
							/*offset += tmpNode.InclusiveSize;


							char * attchname = ( char * )&ModelBytes[ offset ];
							offset += 260;


							DWORD attchid = *( DWORD * )&ModelBytes[ offset ];

							offset += 4;

							if ( memcmp( &ModelBytes[ offset ], "KATV", 4 ) == 0 )
							{
							offset += 4;
							Mdx_Track tmpTrack;
							memcpy( &tmpTrack, &ModelBytes[ offset ], sizeof( Mdx_Track ) );
							offset += sizeof( Mdx_Track );
							for ( DWORD i = 0; i < tmpTrack.NrOfTracks; i++ )
							{
							offset += ( tmpTrack.InterpolationType > 1 ? 16 : 8 );
							}
							}*/

							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "PREM", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );

							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "PRE2", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );

							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );

							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "RIBB", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* )&ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );

							offset += size_of_this_struct - 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "EVTS", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						Mdx_Tracks tmpTracks;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );

							offset += tmpNode.InclusiveSize;
							if ( memcmp( &ModelBytes[ offset ], "KEVT", 4 ) == 0 )
							{
								offset += 4;
								memcpy( &tmpTracks, &ModelBytes[ offset ], sizeof( Mdx_Tracks ) );
								offset += sizeof( Mdx_Tracks );
								for ( int n = 0; n < tmpTracks.NrOfTracks; n++ )
								{
									offset += 4;
								}
							}
							else offset += 4;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "CLID", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* )&ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );

							if ( tmpNode.ObjectId != 0xFFFFFFFF && tmpNode.ObjectId > MaxObjectId )
							{
								MaxObjectId = tmpNode.ObjectId;
							}
							parents.push_back( ( DWORD* )&ModelBytes[ offset + 88 ] );

							offset += tmpNode.InclusiveSize;
							unsigned int size_of_this_struct = *( unsigned int* )&ModelBytes[ offset ];
							offset += 4;
							size_of_this_struct = size_of_this_struct == 0 ? 24 : 16;
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else
					{
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
					}

					offset += 4;
				}
			}


			MaxObjectId++;

			for ( DWORD * parOffsets : parents )
			{
				DWORD curparent = *parOffsets;
				if ( curparent == 0xFFFFFFFF )
				{
					*parOffsets = MaxObjectId;
				}
			}

			FullPatchData.insert( FullPatchData.end( ), ( BYTE* )( ModelBytes ), ( BYTE* )( ModelBytes + sz ) );

			if ( OffsetToInsertPivotPoint != 0 )
			{
				char ZeroPos[ 12 ];
				memset( ZeroPos, 0, sizeof( ZeroPos ) );
				FullPatchData.insert( FullPatchData.begin( ) + OffsetToInsertPivotPoint, ZeroPos, ZeroPos + 12 );
			}

			FullPatchData.insert( FullPatchData.end( ), ( BYTE* )( HelperBytesPart1 ), ( BYTE* )( HelperBytesPart1 + sizeof( HelperBytesPart1 ) ) );
			BYTE * patchbytes = ( BYTE * )&MaxObjectId;

			FullPatchData.push_back( patchbytes[ 0 ] );
			FullPatchData.push_back( patchbytes[ 1 ] );
			FullPatchData.push_back( patchbytes[ 2 ] );
			FullPatchData.push_back( patchbytes[ 3 ] );

			FullPatchData.insert( FullPatchData.end( ), ( BYTE* )( HelperBytesPart2 ), ( BYTE* )( HelperBytesPart2 + sizeof( HelperBytesPart2 ) ) );


			float scaleall = mdlfix.Scale;

			patchbytes = ( BYTE * )&scaleall;


			FullPatchData.push_back( patchbytes[ 0 ] );
			FullPatchData.push_back( patchbytes[ 1 ] );
			FullPatchData.push_back( patchbytes[ 2 ] );
			FullPatchData.push_back( patchbytes[ 3 ] );

			FullPatchData.push_back( patchbytes[ 0 ] );
			FullPatchData.push_back( patchbytes[ 1 ] );
			FullPatchData.push_back( patchbytes[ 2 ] );
			FullPatchData.push_back( patchbytes[ 3 ] );

			FullPatchData.push_back( patchbytes[ 0 ] );
			FullPatchData.push_back( patchbytes[ 1 ] );
			FullPatchData.push_back( patchbytes[ 2 ] );
			FullPatchData.push_back( patchbytes[ 3 ] );

			FullPatchData.insert( FullPatchData.end( ), ( BYTE* )( HelperBytesPart3 ), ( BYTE* )( HelperBytesPart3 + sizeof( HelperBytesPart3 ) ) );

			if ( !FoundGLBS )
			{
				FullPatchData.push_back( strGLBS[ 0 ] );
				FullPatchData.push_back( strGLBS[ 1 ] );
				FullPatchData.push_back( strGLBS[ 2 ] );
				FullPatchData.push_back( strGLBS[ 3 ] );
				DWORD szGLBS = 4;
				patchbytes = ( BYTE * )&szGLBS;
				FullPatchData.push_back( patchbytes[ 0 ] );
				FullPatchData.push_back( patchbytes[ 1 ] );
				FullPatchData.push_back( patchbytes[ 2 ] );
				FullPatchData.push_back( patchbytes[ 3 ] );
				szGLBS = 0;
				patchbytes = ( BYTE * )&szGLBS;
				FullPatchData.push_back( patchbytes[ 0 ] );
				FullPatchData.push_back( patchbytes[ 1 ] );
				FullPatchData.push_back( patchbytes[ 2 ] );
				FullPatchData.push_back( patchbytes[ 3 ] );
			}


			if ( IsKeyPressed( '0' ) && FileExist( ".\\Test1234.mdx" ) )
			{
				FILE *f;
				fopen_s( &f, ".\\Test1234.mdx", "wb" );
				fwrite( &FullPatchData[ 0 ], FullPatchData.size( ), 1, f );
				fclose( f );
				MessageBoxA( 0, "Ok dump", "DUMP", 0 );
			}


			ICONMDLCACHE * tmpih = new ICONMDLCACHE( );

			Buffer ResultBuffer;
			ResultBuffer.buf = new char[ FullPatchData.size( ) ];
			ResultBuffer.length = FullPatchData.size( );

			memcpy( &ResultBuffer.buf[ 0 ], &FullPatchData[ 0 ], FullPatchData.size( ) );

			tmpih->buf = ResultBuffer.buf;
			tmpih->size = ResultBuffer.length;
			tmpih->hashlen = filename.length( );
			tmpih->hash = GetBufHash( filename.c_str( ), tmpih->hashlen );

			ICONMDLCACHELIST.push_back( *tmpih );


			Storm_403_org( ( void* )*OutDataPointer, "delete", -1, 0 );

			*OutDataPointer = ( int )tmpih->buf;
			*OutSize = tmpih->size;

			ModelBytes = ( BYTE * )tmpih->buf;
			sz = tmpih->size;

			delete tmpih;

			ModelScaleList.erase( ModelScaleList.begin( ) + ( int )i );

		}
	}


	if ( !FullPatchData.empty( ) )
		FullPatchData.clear( );
	for ( unsigned int i = 0; i < ModelPatchList.size( ); i++ )
	{
		ModelPatchStruct mdlfix = ModelPatchList[ i ];
		if ( filename == mdlfix.FilePath )
		{
			int PatchFileData;
			size_t PatchFileSize;

			if ( GameGetFile_ptr( mdlfix.patchPath.c_str( ), &PatchFileData, &PatchFileSize, unknown ) )
			{
				FullPatchData.insert( FullPatchData.end( ), ( char* )( PatchFileData ), ( char* )( PatchFileData + PatchFileSize ) );
			}


			ModelPatchList.erase( ModelPatchList.begin( ) + ( int )i );

		}
	}

	if ( !FullPatchData.empty( ) )
	{

		ICONMDLCACHE * tmpih = new ICONMDLCACHE( );
		BOOL FoundOldHelper = GetFromIconMdlCache( filename, tmpih );


		if ( FoundOldHelper )
		{
			Buffer ResultBuffer;
			ResultBuffer.buf = new char[ tmpih->size + FullPatchData.size( ) ];

			ResultBuffer.length = tmpih->size + FullPatchData.size( );

			memcpy( &ResultBuffer.buf[ 0 ], tmpih->buf, sz );

			memcpy( &ResultBuffer.buf[ sz ], &FullPatchData[ 0 ], FullPatchData.size( ) );


			delete[ ] tmpih->buf;
			tmpih->buf = ResultBuffer.buf;
			tmpih->size = ResultBuffer.length;
			*OutDataPointer = ( int )tmpih->buf;
			*OutSize = tmpih->size;
		}
		else
		{
			Buffer ResultBuffer;
			ResultBuffer.buf = new char[ sz + FullPatchData.size( ) ];
			ResultBuffer.length = sz + FullPatchData.size( );

			memcpy( &ResultBuffer.buf[ 0 ], ModelBytes, sz );
			memcpy( &ResultBuffer.buf[ sz ], &FullPatchData[ 0 ], FullPatchData.size( ) );

			tmpih->buf = ResultBuffer.buf;
			tmpih->size = ResultBuffer.length;

			tmpih->hashlen = filename.length( );
			tmpih->hash = GetBufHash( filename.c_str( ), tmpih->hashlen );

			ICONMDLCACHELIST.push_back( *tmpih );


			Storm_403_org( ( void* )*OutDataPointer, "delete", -1, 0 );


			*OutDataPointer = ( int )tmpih->buf;
			*OutSize = tmpih->size;

			ModelBytes = ( BYTE * )tmpih->buf;
			sz = tmpih->size;

		}

		if ( IsKeyPressed( '0' ) && FileExist( ".\\Test1234.mdx" ) )
		{
			FILE *f;
			fopen_s( &f, ".\\Test1234.mdx", "wb" );
			fwrite( ModelBytes, sz, 1, f );
			fclose( f );
		}

		delete tmpih;
		FullPatchData.clear( );
	}


	for ( unsigned int i = 0; i < ModelCollisionFixList.size( ); i++ )
	{
		ModelCollisionFixStruct mdlfix = ModelCollisionFixList[ i ];
		if ( filename == mdlfix.FilePath )
		{
			size_t offset = 0;
			if ( memcmp( &ModelBytes[ offset ], "MDLX", 4 ) == 0 )
			{
				offset += 4;
				while ( offset < sz )
				{
					if ( memcmp( &ModelBytes[ offset ], "CLID", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( int* )&ModelBytes[ offset ];
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
						int shapetype = *( int* )&ModelBytes[ offset ];

						if ( shapetype == 2 )
						{
							offset += 4;
							*( float* )&ModelBytes[ offset ] = mdlfix.X;
							offset += 4;
							*( float* )&ModelBytes[ offset ] = mdlfix.Y;
							offset += 4;
							*( float* )&ModelBytes[ offset ] = mdlfix.Z;
							offset += 4;
							*( float* )&ModelBytes[ offset ] = mdlfix.Radius;
						}
						offset = newoffset + 4;
					}
					else
					{
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
					}
					offset += 4;
				}
			}

			ModelCollisionFixList.erase( ModelCollisionFixList.begin( ) + ( int )i );

		}

	}


	for ( unsigned int i = 0; i < ModelTextureFixList.size( ); i++ )
	{
		ModelTextureFixStruct mdlfix = ModelTextureFixList[ i ];
		int TextureID = 0;
		if ( filename == mdlfix.FilePath )
		{
			size_t offset = 0;
			if ( memcmp( &ModelBytes[ offset ], "MDLX", 4 ) == 0 )
			{
				offset += 4;
				while ( offset < sz )
				{
					if ( memcmp( &ModelBytes[ offset ], "TEXS", 4 ) == 0 )
					{
						Mdx_Texture tmpTexture;

						//char TextNameBuf[ 0x100 ];
						offset += 4;
						int TagSize = *( int* )&ModelBytes[ offset ];
						size_t newoffset = offset + TagSize;
						int TexturesCount = TagSize / ( int ) sizeof( Mdx_Texture );
						offset += 4;


						while ( TexturesCount > 0 )
						{
							TexturesCount--;
							memcpy( &tmpTexture, &ModelBytes[ offset ], sizeof( Mdx_Texture ) );
							TextureID++;

							if ( mdlfix.TextureID == TextureID )
							{
								if ( mdlfix.NewTexturePath.length( ) > 3 )
								{
									tmpTexture.ReplaceableId = 0;
									sprintf_s( tmpTexture.FileName, 260, "%s", mdlfix.NewTexturePath.c_str( ) );
								}
								else
								{
									tmpTexture.ReplaceableId = atoi( mdlfix.NewTexturePath.c_str( ) );
									memset( tmpTexture.FileName, 0, 260 );
								}
								memcpy( &ModelBytes[ offset ], &tmpTexture, sizeof( Mdx_Texture ) );
							}
							offset += sizeof( Mdx_Texture );
						}
						offset = newoffset + 4;
					}
					else
					{
						offset += 4;
						offset += *( int* )&ModelBytes[ offset ];
					}
					offset += 4;
				}
			}



			ModelTextureFixList.erase( ModelTextureFixList.begin( ) + ( int )i );

		}

	}


}


int __stdcall RedirectFile( const char * RealFilePath, const char * NewFilePath )
{
	FileRedirectStruct tmpModelFix;
	tmpModelFix.NewFilePath = NewFilePath;
	tmpModelFix.RealFilePath = RealFilePath;
	FileRedirectList.push_back( tmpModelFix );
	return 0;
}

void PrintLog( const char * str )
{
	FILE * f;

	fopen_s( &f, ".\\text.txt", "a+" );
	if ( f != NULL )
	{
		fprintf_s( f, "%s\n", str );
		fclose( f );
	}
}


BOOL ProcessFile( string filename, int * OutDataPointer, size_t * OutSize, BOOL unknown, BOOL IsFileExistOld )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "ProcessFile start..." );
#endif
	BOOL IsFileExist = IsFileExistOld;

	if ( !OutDataPointer || !OutSize || filename.length( ) < 3 )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "Bad file found" );
#endif
		return IsFileExist;
	}

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "Read from cache..." );
#endif
	ICONMDLCACHE tmpih;
	BOOL FoundOldHelper = GetFromIconMdlCache( filename.c_str( ), &tmpih );
	if ( FoundOldHelper )
	{
		*OutDataPointer = ( int )tmpih.buf;
		*OutSize = tmpih.size;
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "Read from cache [OK]." );
#endif
		return TRUE;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "Not found in cache. Process next..." );
#endif

	for ( RawImageStruct & s : ListOfRawImages )
	{
		if ( s.ingame )
		{
			if ( ToLower( filename ) == ToLower( s.filename ) )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( "Print filename1:" + filename );
#endif
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( "Print filename2:" + s.filename );
#endif
				*OutDataPointer = ( int )s.ingamebuffer.buf;
				*OutSize = s.ingamebuffer.length;
				return TRUE;
			}
		}
	}

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "Print filename:" + filename );
#endif


	string FileExtension = ToLower( fs::path( filename ).extension( ).string( ) );

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "ProcessFileStart...[" + FileExtension + "], Path:" + ToLower( fs::path( filename ).extension( ).string( ) ) );
#endif


	if ( FileExtension == string( ".tga" ) )
	{

	}
	else if ( FileExtension == string( ".blp" ) )
	{
		if ( !IsFileExist )
		{
			IsFileExist = FixDisabledIconPath( filename, OutDataPointer, OutSize, unknown );
		}
		else
		{
			//ApplyTestFilter( filename, OutDataPointer, OutSize );
			/*	if ( strstr( FilePathLower.c_str( ), "terrainart" ) == FilePathLower.c_str( ) ||
					 strstr( FilePathLower.c_str( ), "replaceabletextures\\cliff" ) == FilePathLower.c_str( ) )
					ApplyTerrainFilter( filename, OutDataPointer, OutSize, FALSE );*/
		}
	}
	else if ( FileExtension == string( ".mdx" ) )
	{
		if ( IsFileExist )
		{
			ProcessMdx( filename, OutDataPointer, OutSize, unknown );
		}
		else
		{
			//return GameGetFile_ptr( "Objects\\InvalidObject\\InvalidObject.mdx", OutDataPointer, OutSize, unknown );
		}
	}




	for ( FileRedirectStruct & DotaRedirectHelp : FileRedirectList )
	{
		if ( filename == DotaRedirectHelp.NewFilePath )
		{
			ICONMDLCACHE *tmpih2 = new ICONMDLCACHE( );

			IsFileExist = GameGetFile_ptr( DotaRedirectHelp.RealFilePath.c_str( ), OutDataPointer, OutSize, unknown );
			if ( IsFileExist )
			{
				char * DataPointer = ( char * )*OutDataPointer;
				size_t DataSize = *OutSize;

				Buffer ResultBuffer;
				ResultBuffer.buf = new char[ DataSize ];
				ResultBuffer.length = DataSize;

				memcpy( &ResultBuffer.buf[ 0 ], DataPointer, DataSize );

				tmpih2->buf = ResultBuffer.buf;
				tmpih2->size = ResultBuffer.length;

				tmpih2->hashlen = DotaRedirectHelp.NewFilePath.length( );
				tmpih2->hash = GetBufHash( DotaRedirectHelp.NewFilePath.c_str( ), tmpih2->hashlen );

				ICONMDLCACHELIST.push_back( *tmpih2 );

				*OutDataPointer = ( int )tmpih2->buf;
				*OutSize = tmpih2->size;

				IsFileExist = ProcessFile( DotaRedirectHelp.NewFilePath, OutDataPointer, OutSize, unknown, IsFileExist );

				return IsFileExist;
			}


			delete tmpih2;
		}
	}

	return IsFileExist;
}

vector<FakeFileStruct> FakeFileList;

void AddNewFakeFile( char * filename, BYTE * buffer, size_t FileSize )
{
	FakeFileStruct tmpstr;
	tmpstr.buffer = buffer;
	tmpstr.filename = filename;
	tmpstr.size = FileSize;
	FakeFileList.push_back( tmpstr );
}



BOOL __fastcall GameGetFile_my( const char * filename, int * OutDataPointer, unsigned int * OutSize, BOOL unknown )
{



#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( string( "Start File Helper..[" ) + to_string( unknown ) + "]" );
#endif

	if ( TerminateStarted )
	{

#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "TerminateStarted" );
#endif

		return GameGetFile_ptr( filename, OutDataPointer, OutSize, unknown );
	}

	if ( !OutDataPointer || !OutSize )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "Bad Pointers" );
#endif
		return GameGetFile_ptr( filename, OutDataPointer, OutSize, unknown );
	}

	for ( FakeFileStruct & fs : FakeFileList )
	{
		if ( _stricmp( filename, fs.filename ) == 0 )
		{
			*OutDataPointer = ( int )fs.buffer;
			*OutSize = fs.size;
			return TRUE;
		}
	}

#ifdef DOTA_HELPER_LOG
	if ( filename && *filename != '\0' )
		AddNewLineToDotaHelperLog( string( "FileHelper:" ) + string( filename ) );
	else
		AddNewLineToDotaHelperLog( "FileHelper(BADFILENAME)" );
#endif

	BOOL IsFileExist = GameGetFile_ptr( filename, OutDataPointer, OutSize, unknown );



	if ( !*InGame && !MainFuncWork )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "Game not found or main not set" );
#endif

		return IsFileExist;
	}

	if ( filename == NULL || *filename == '\0' )
	{

#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "Bad file name" );
#endif

		return IsFileExist;
	}

	if ( !IsFileExist )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "NoFileFound" );
#endif
	}
	else
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( "FileFound" );
#endif
	}
#ifdef DOTA_HELPER_LOG
	try
	{
#endif
		IsFileExist = ProcessFile( filename, OutDataPointer, OutSize, unknown, IsFileExist );
#ifdef DOTA_HELPER_LOG
	}
	catch ( std::exception e )
	{
		MessageBoxA( 0, e.what( ), "ProcessFile Перехвачена ошибка! Catch Error!", 0 );
	}
	catch ( ... )
	{
		MessageBoxA( 0, "Неизвестная ошибка.", "ProcessFile Перехвачена ошибка! Catch Error!", 0 );
	}
#endif
#ifdef DOTA_HELPER_LOG
	if ( !IsFileExist )
	{
		AddNewLineToDotaHelperLog( "NoFileFound" );
	}
	else
	{
		AddNewLineToDotaHelperLog( "FileFound" );
	}
#endif



	return IsFileExist;
}



//iconpath + _frame.blp
int __stdcall CreateIconFrameMask( const char * iconpath )
{

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "CreateIconFrameMask" );
	cout << "CreateIconFrameMask:" << iconpath << endl;
#endif

	ApplyIconFrameFilter( iconpath, 0, 0 );



#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "CreateIconFrameMask:OK2" );
#endif
	return TRUE;
}



//Storm_279 Storm_279_org;
//Storm_279 Storm_279_ptr;
//int __stdcall Storm_279_my( const char * filename, int arg1, int arg2, size_t arg3, int arg4 )
//{
//
//	if ( NeedOpenConfigWindow )
//	{
//		if ( ToLower( "UI\\FrameDef\\Glue\\DialogWar3.fdf" ) == ToLower( filename ) )
//		{
//			return Storm_279_ptr( "DotaConfigEditor.fdf", arg1, arg2, arg3, arg4 );
//		}
//	}
//
//	return Storm_279_ptr( filename, arg1, arg2, arg3, arg4 );
//}



/*

	Draw images

*/

/*struct RawImageStruct
{
	int width;
	int height;
	Buffer img;
	Buffer ingamebuffer;
	bool ingame;
	string filename;
};*/

vector<RawImageStruct> ListOfRawImages;


double pDistance( int x1, int y1, int x2, int y2 )
{
	return sqrt( ( x2 - x1 )*( x2 - x1 ) + ( y2 - y1 )*( y2 - y1 ) );
}

// Создает RawImage (RGBA) с указанным цветом
int __stdcall CreateRawImage( int width, int height, RGBAPix defaultcolor )
{
	int resultid = ListOfRawImages.size( );
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "CreateRawImage" );
	cout << "CreateRawImage:" << endl;
#endif

	RawImageStruct tmpRawImage = RawImageStruct( );
	Buffer tmpRawImageBuffer = Buffer( );
	tmpRawImageBuffer.Resize( width * height * 4 );

	for ( int i = 0; i < width * height; i++ )
	{
		*( RGBAPix* )&tmpRawImageBuffer[ i * 4 ] = defaultcolor;
	}

	tmpRawImage.img = tmpRawImageBuffer;
	tmpRawImage.width = width;
	tmpRawImage.height = height;
	tmpRawImage.filename = string( );
	tmpRawImage.RawImage = resultid;
	ListOfRawImages.push_back( tmpRawImage );

	return resultid;
}

// Загружает RawImage из filename (tga,blp)
int __stdcall LoadRawImage( const char * filename )
{
	int resultid = ListOfRawImages.size( );
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "LoadRawImage" );
	cout << "LoadRawImage:" << endl;
#endif

	int filenamelen = strlen( filename );


	int PatchFileData = 0;
	size_t PatchFileSize = 0;
	GameGetFile_org( filename, &PatchFileData, &PatchFileSize, TRUE );
	if ( !PatchFileData || !PatchFileSize )
	{
		GameGetFile_org( ( filename + string( ".tga" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
		if ( !PatchFileData || !PatchFileSize )
		{
			GameGetFile_org( ( filename + string( ".blp" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
			if ( !PatchFileData || !PatchFileSize )
			{
				int filenamelen = strlen( filename );
				if ( filenamelen >= 4 )
				{
					char * tmpfilename = new char[ filenamelen ];
					memset( tmpfilename, 0, filenamelen );
					memcpy( tmpfilename, filename, filenamelen - 4 );
					GameGetFile_org( ( tmpfilename + string( ".blp" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
					if ( !PatchFileData || !PatchFileSize )
					{
						GameGetFile_org( ( tmpfilename + string( ".tga" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
					}

					delete[ ] tmpfilename;
				}

			}
		}
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "LoadRawImage2" );
	cout << "LoadRawImage2:" << endl;
#endif
	if ( PatchFileData &&  PatchFileSize > 5 )
	{
		BOOL IsBlp = memcmp( ( LPCVOID )PatchFileData, "BLP1", 4 ) == 0;
		int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 8, compress = 1, alphaenconding = 5;
		unsigned long rawImageSize = 0;

		Buffer OutBuffer = Buffer( );
		Buffer InBuffer( ( char * )PatchFileData, PatchFileSize );

		if ( !IsBlp )
			rawImageSize = ( unsigned long )TGA2Raw( InBuffer, OutBuffer, w, h, bpp, filename );
		else
			rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename );

		if ( rawImageSize > 0 )
		{
			RawImageStruct tmpRawImage = RawImageStruct( );
			tmpRawImage.img = OutBuffer;
			tmpRawImage.width = w;
			tmpRawImage.height = h;
			tmpRawImage.filename = filename;
			tmpRawImage.RawImage = resultid;
			ListOfRawImages.push_back( tmpRawImage );
		}
	}
	else return 0;

	return resultid;
}


enum BlendModes : int
{
	BlendNormal,
	BlendAdd,
	BlendSubtract,
	BlendMultiple

};

// Рисует RawImage2 на RawImage
int __stdcall RawImage_DrawImg( int RawImage, int RawImage2, int drawx, int drawy, int blendmode )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}


	if ( RawImage2 >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RawImageStruct & tmpRawImage2 = ListOfRawImages[ RawImage2 ];


	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	RGBAPix* RawImageData2 = ( RGBAPix* )tmpRawImage2.img.buf;

	for ( int x = drawx, x2 = 0; x < tmpRawImage.width && x2 < tmpRawImage2.width; x++, x2++ )
	{
		for ( int y = drawy, y2 = 0; y < tmpRawImage.height && y2 < tmpRawImage2.height; y++, y2++ )
		{
			if ( blendmode == BlendModes::BlendNormal )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] = RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else if ( blendmode == BlendModes::BlendAdd )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] + RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else if ( blendmode == BlendModes::BlendSubtract )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] - RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else if ( blendmode == BlendModes::BlendMultiple )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] * RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] / RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
		}
	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}

// Заполняет выбранный пиксель указанным цветом
int __stdcall RawImage_DrawPixel( int RawImage, int x, int y, RGBAPix color )//RGBAPix = unsigned int
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	if ( x >= 0 && y >= 0 && x < tmpRawImage.width && y < tmpRawImage.height )
	{
		RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] = color;
	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}

// Рисует прямоугольник с указанным цветом и размером
int __stdcall RawImage_DrawRect( int RawImage, int drawx, int drawy, int widthsize, int heightsize, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	for ( int xsize = 0; xsize < widthsize; xsize++ )
	{
		for ( int ysize = 0; ysize < heightsize; ysize++ )
		{
			RawImage_DrawPixel( RawImage, drawx + xsize, drawy + ysize, color );
		}
	}

	return TRUE;
}

#pragma region DrawLineAlgorithm

/*
*
* @date 25.03.2013
* @author Armin Joachimsmeyer
* https://github.com/ArminJo/STMF3-Discovery-Demos/blob/master/lib/graphics/src/thickLine.cpp
*
*/

#define LINE_OVERLAP_NONE 0 	// No line overlap, like in standard Bresenham
#define LINE_OVERLAP_MAJOR 0x01 // Overlap - first go major then minor direction. Pixel is drawn as extension after actual line
#define LINE_OVERLAP_MINOR 0x02 // Overlap - first go minor then major direction. Pixel is drawn as extension before next line
#define LINE_OVERLAP_BOTH 0x03  // Overlap - both

#define LINE_THICKNESS_MIDDLE 0                 // Start point is on the line at center of the thick line
#define LINE_THICKNESS_DRAW_CLOCKWISE 1         // Start point is on the counter clockwise border line
#define LINE_THICKNESS_DRAW_COUNTERCLOCKWISE 2  // Start point is on the clockwise border line

void drawLineOverlap( int RawImage, int aXStart, int aYStart, int aXEnd, int aYEnd, uint8_t aOverlap,
	RGBAPix aColor ) {
	int16_t tDeltaX, tDeltaY, tDeltaXTimes2, tDeltaYTimes2, tError, tStepX, tStepY;
	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	int maxwidth = tmpRawImage.width;
	int maxheight = tmpRawImage.height;
	/*
	* Clip to display size
	*/
	if ( aXStart >= maxwidth ) {
		aXStart = maxwidth - 1;
	}
	if ( aXStart < 0 ) {
		aXStart = 0;
	}
	if ( aXEnd >= maxwidth ) {
		aXEnd = maxwidth - 1;
	}
	if ( aXEnd < 0 ) {
		aXEnd = 0;
	}
	if ( aYStart >= maxheight ) {
		aYStart = maxheight - 1;
	}
	if ( aYStart < 0 ) {
		aYStart = 0;
	}
	if ( aYEnd >= maxheight ) {
		aYEnd = maxheight - 1;
	}
	if ( aYEnd < 0 ) {
		aYEnd = 0;
	}

	if ( ( aXStart == aXEnd ) || ( aYStart == aYEnd ) ) {
		//horizontal or vertical line -> fillRect() is faster

		if ( aXEnd >= aXStart && aYEnd >= aYStart )
		{
			RawImage_DrawRect( RawImage, aXStart, aYStart, aXEnd - aXStart + 1, aYEnd - aYStart + 1, aColor );
		}
		else if ( aXEnd >= aXStart )
		{
			RawImage_DrawRect( RawImage, aXStart, aYEnd, aXEnd - aXStart + 1, aYStart - aYEnd + 1, aColor );
		}
		else if ( aYEnd >= aYStart )
		{
			RawImage_DrawRect( RawImage, aXEnd, aYStart, aXStart - aXEnd + 1, aYEnd - aYStart + 1, aColor );
		}
	}
	else {
		//calculate direction
		tDeltaX = aXEnd - aXStart;
		tDeltaY = aYEnd - aYStart;
		if ( tDeltaX < 0 ) {
			tDeltaX = -tDeltaX;
			tStepX = -1;
		}
		else {
			tStepX = +1;
		}
		if ( tDeltaY < 0 ) {
			tDeltaY = -tDeltaY;
			tStepY = -1;
		}
		else {
			tStepY = +1;
		}
		tDeltaXTimes2 = tDeltaX << 1;
		tDeltaYTimes2 = tDeltaY << 1;
		//draw start pixel
		RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
		if ( tDeltaX > tDeltaY ) {
			// start value represents a half step in Y direction
			tError = tDeltaYTimes2 - tDeltaX;
			while ( aXStart != aXEnd ) {
				// step in main direction
				aXStart += tStepX;
				if ( tError >= 0 ) {
					if ( aOverlap & LINE_OVERLAP_MAJOR ) {
						// draw pixel in main direction before changing
						RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
					}
					// change Y
					aYStart += tStepY;
					if ( aOverlap & LINE_OVERLAP_MINOR ) {
						// draw pixel in minor direction before changing
						RawImage_DrawPixel( RawImage, aXStart - tStepX, aYStart, aColor );
					}
					tError -= tDeltaXTimes2;
				}
				tError += tDeltaYTimes2;
				RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
			}
		}
		else {
			tError = tDeltaXTimes2 - tDeltaY;
			while ( aYStart != aYEnd ) {
				aYStart += tStepY;
				if ( tError >= 0 ) {
					if ( aOverlap & LINE_OVERLAP_MAJOR ) {
						// draw pixel in main direction before changing
						RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
					}
					aXStart += tStepX;
					if ( aOverlap & LINE_OVERLAP_MINOR ) {
						// draw pixel in minor direction before changing
						RawImage_DrawPixel( RawImage, aXStart, aYStart - tStepY, aColor );
					}
					tError -= tDeltaYTimes2;
				}
				tError += tDeltaXTimes2;
				RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
			}
		}
	}
}

/**
* Bresenham with thickness
* no pixel missed and every pixel only drawn once!
*/
void drawThickLine( int RawImage, int aXStart, int aYStart, int aXEnd, int aYEnd, int aThickness,
	uint8_t aThicknessMode, RGBAPix aColor ) {
	int16_t i, tDeltaX, tDeltaY, tDeltaXTimes2, tDeltaYTimes2, tError, tStepX, tStepY;
	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	int maxwidth = tmpRawImage.width;
	int maxheight = tmpRawImage.height;

	if ( aThickness <= 1 ) {
		drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, LINE_OVERLAP_NONE, aColor );
	}
	/*
	* Clip to display size
	*/
	if ( aXStart >= maxwidth ) {
		aXStart = maxwidth - 1;
	}
	if ( aXStart < 0 ) {
		aXStart = 0;
	}
	if ( aXEnd >= maxwidth ) {
		aXEnd = maxwidth - 1;
	}
	if ( aXEnd < 0 ) {
		aXEnd = 0;
	}
	if ( aYStart >= maxheight ) {
		aYStart = maxheight - 1;
	}
	if ( aYStart < 0 ) {
		aYStart = 0;
	}
	if ( aYEnd >= maxheight ) {
		aYEnd = maxheight - 1;
	}
	if ( aYEnd < 0 ) {
		aYEnd = 0;
	}

	/**
	* For coordinate system with 0.0 top left
	* Swap X and Y delta and calculate clockwise (new delta X inverted)
	* or counterclockwise (new delta Y inverted) rectangular direction.
	* The right rectangular direction for LINE_OVERLAP_MAJOR toggles with each octant
	*/
	tDeltaY = aXEnd - aXStart;
	tDeltaX = aYEnd - aYStart;
	// mirror 4 quadrants to one and adjust deltas and stepping direction
	bool tSwap = true; // count effective mirroring
	if ( tDeltaX < 0 ) {
		tDeltaX = -tDeltaX;
		tStepX = -1;
		tSwap = !tSwap;
	}
	else {
		tStepX = +1;
	}
	if ( tDeltaY < 0 ) {
		tDeltaY = -tDeltaY;
		tStepY = -1;
		tSwap = !tSwap;
	}
	else {
		tStepY = +1;
	}
	tDeltaXTimes2 = tDeltaX << 1;
	tDeltaYTimes2 = tDeltaY << 1;
	bool tOverlap;
	// adjust for right direction of thickness from line origin
	int tDrawStartAdjustCount = aThickness / 2;
	if ( aThicknessMode == LINE_THICKNESS_DRAW_COUNTERCLOCKWISE ) {
		tDrawStartAdjustCount = aThickness - 1;
	}
	else if ( aThicknessMode == LINE_THICKNESS_DRAW_CLOCKWISE ) {
		tDrawStartAdjustCount = 0;
	}

	// which octant are we now
	if ( tDeltaX >= tDeltaY ) {
		if ( tSwap ) {
			tDrawStartAdjustCount = ( aThickness - 1 ) - tDrawStartAdjustCount;
			tStepY = -tStepY;
		}
		else {
			tStepX = -tStepX;
		}
		/*
		* Vector for draw direction of lines is rectangular and counterclockwise to original line
		* Therefore no pixel will be missed if LINE_OVERLAP_MAJOR is used
		* on changing in minor rectangular direction
		*/
		// adjust draw start point
		tError = tDeltaYTimes2 - tDeltaX;
		for ( i = tDrawStartAdjustCount; i > 0; i-- ) {
			// change X (main direction here)
			aXStart -= tStepX;
			aXEnd -= tStepX;
			if ( tError >= 0 ) {
				// change Y
				aYStart -= tStepY;
				aYEnd -= tStepY;
				tError -= tDeltaXTimes2;
			}
			tError += tDeltaYTimes2;
		}
		//draw start line
		drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, LINE_OVERLAP_NONE, aColor );
		// draw aThickness lines
		tError = tDeltaYTimes2 - tDeltaX;
		for ( i = aThickness; i > 1; i-- ) {
			// change X (main direction here)
			aXStart += tStepX;
			aXEnd += tStepX;
			tOverlap = LINE_OVERLAP_NONE;
			if ( tError >= 0 ) {
				// change Y
				aYStart += tStepY;
				aYEnd += tStepY;
				tError -= tDeltaXTimes2;
				/*
				* change in minor direction reverse to line (main) direction
				* because of choosing the right (counter)clockwise draw vector
				* use LINE_OVERLAP_MAJOR to fill all pixel
				*
				* EXAMPLE:
				* 1,2 = Pixel of first lines
				* 3 = Pixel of third line in normal line mode
				* - = Pixel which will additionally be drawn in LINE_OVERLAP_MAJOR mode
				*           33
				*       3333-22
				*   3333-222211
				* 33-22221111
				*  221111                     /\
				*  11                          Main direction of draw vector
				*  -> Line main direction
				*  <- Minor direction of counterclockwise draw vector
				*/
				tOverlap = LINE_OVERLAP_MAJOR;
			}
			tError += tDeltaYTimes2;
			drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, tOverlap, aColor );
		}
	}
	else {
		// the other octant
		if ( tSwap ) {
			tStepX = -tStepX;
		}
		else {
			tDrawStartAdjustCount = ( aThickness - 1 ) - tDrawStartAdjustCount;
			tStepY = -tStepY;
		}
		// adjust draw start point
		tError = tDeltaXTimes2 - tDeltaY;
		for ( i = tDrawStartAdjustCount; i > 0; i-- ) {
			aYStart -= tStepY;
			aYEnd -= tStepY;
			if ( tError >= 0 ) {
				aXStart -= tStepX;
				aXEnd -= tStepX;
				tError -= tDeltaYTimes2;
			}
			tError += tDeltaXTimes2;
		}
		//draw start line
		drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, LINE_OVERLAP_NONE, aColor );
		tError = tDeltaXTimes2 - tDeltaY;
		for ( i = aThickness; i > 1; i-- ) {
			aYStart += tStepY;
			aYEnd += tStepY;
			tOverlap = LINE_OVERLAP_NONE;
			if ( tError >= 0 ) {
				aXStart += tStepX;
				aXEnd += tStepX;
				tError -= tDeltaYTimes2;
				tOverlap = LINE_OVERLAP_MAJOR;
			}
			tError += tDeltaXTimes2;
			drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, tOverlap, aColor );
		}
	}
}

#pragma endregion

// Рисует линию с указанным цветом и размером
int __stdcall RawImage_DrawLine( int RawImage, int x1, int y1, int x2, int y2, int size, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	drawThickLine( RawImage, x1, y1, x2, y2, size, 0, color );

	return TRUE;
}

// Рисует круг с указанным радиусом и толщиной
int __stdcall RawImage_DrawCircle( int RawImage, int x, int y, int radius, int size, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}
	size /= 2;
	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			double dist = pDistance( x, y, x2, y2 );
			if ( pDistance( x, y, x2, y2 ) >= radius - size && pDistance( x, y, x2, y2 ) <= radius + size )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = color;
			}

		}

	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}




// Заполняет круг указанным цветом
int __stdcall RawImage_FillCircle( int RawImage, int x, int y, int radius, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			if ( pDistance( x, y, x2, y2 ) <= radius )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = color;
			}
		}
	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}


// Оставляет только круг с указанным радиусом
int __stdcall RawImage_EraseCircle( int RawImage, int x, int y, int radius, BOOL inverse )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	RGBAPix tmpPix = RGBAPix( );

	if ( !inverse )
	{
		return RawImage_FillCircle( RawImage, x, y, radius, tmpPix );
	}

	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			if ( pDistance( x, y, x2, y2 ) > radius )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = tmpPix;
			}
		}
	}
	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}

// Делает пиксели с цветом color - прозрачными, power от 0 до 255
int __stdcall RawImage_EraseColor( int RawImage, RGBAPix color, int power )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	RGBAPix tmpPix = RGBAPix( );
	unsigned char// A = color.A,
		R = color.R,
		G = color.G,
		B = color.B;

	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			unsigned char// A2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].A,
				R2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].R,
				G2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].G,
				B2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].B;

			if ( //( A >= A2 - power && A <= A2 + power ) &&
				( R >= R2 - power && R <= R2 + power ) &&
				( G >= G2 - power && G <= G2 + power ) &&
				( B >= B2 - power && B <= B2 + power ) )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = tmpPix;
			}
		}
	}
	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}

	return TRUE;
}

const char * _fontname = "Arial";
int _fontsize = 20;
unsigned int _flags = 0;
// 0x1 = BOLD


// Устанавливает настройки шрифта для RawImage_DrawText
int __stdcall RawImage_LoadFontFromResource( const char * filepath )
{
	int PatchFileData = 0;
	size_t PatchFileSize = 0;
	GameGetFile_org( filepath, &PatchFileData, &PatchFileSize, TRUE );
	DWORD Font;//Globals, this is the Font in the RAM
	AddFontMemResourceEx( ( void* )PatchFileData, PatchFileSize, NULL, &Font );
	return TRUE;
}


// Устанавливает настройки шрифта для RawImage_DrawText
int __stdcall RawImage_SetFontSettings( const char * fontname, int fontsize, unsigned int flags )
{
	_fontname = fontname;
	_fontsize = fontsize;
	_flags = flags;
	return TRUE;
}

// Пишет текст в указанных координатах с указанными цветом и настройками шрифта RawImage_SetFontSettings
int __stdcall RawImage_DrawText( int RawImage, const char * text, int x, int y, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;

	HDC hDC = CreateCompatibleDC( NULL );
	char* pSrcData = 0;
	BITMAPINFO bmi = { sizeof( BITMAPINFOHEADER ), tmpRawImage.width, tmpRawImage.height, 1, 24, BI_RGB, 0, 0, 0, 0, 0 };
	HBITMAP hTempBmp = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, ( void** )&pSrcData, NULL, 0 );
	RECT rect = RECT( );
	rect.left = x;
	rect.top = y;
	rect.bottom = tmpRawImage.height;
	rect.right = tmpRawImage.width;

	HBITMAP hBmpOld = ( HBITMAP )SelectObject( hDC, hTempBmp );
	HFONT NewFont = CreateFontA( _fontsize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
	HFONT TempFont = NULL;

	UINT textcolor = color.ToUINT( );
	UINT oldcolor = color.ToUINT( );


	RGBAPix tmpPix = RGBAPix( );


	SetBkColor( hDC, 0x00000000 );
	SetBkMode( hDC, TRANSPARENT );

	SelectObject( hDC, NewFont );
	SetTextColor( hDC, color.ToUINT( ) );



	int len = strlen( text );
	BOOL boldenabled = FALSE;
	BOOL italicenabled = FALSE;
	BOOL underlineenabled = FALSE;
	BOOL strikeoutenabled = FALSE;
	int i = 0;
	BOOL newline = FALSE;
	for ( int i = 0; i < len; )
	{
		if ( len - i > 1 )
		{
			if ( text[ i ] == '|' && ( text[ i + 1 ] == 'n' || text[ i + 1 ] == 'N' ) )
			{
				i += 2;
				newline = TRUE;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'b' || text[ i + 1 ] == 'B' ) )
			{
				i += 2;
				boldenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'u' || text[ i + 1 ] == 'U' ) )
			{
				i += 2;
				underlineenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 's' || text[ i + 1 ] == 'S' ) )
			{
				i += 2;
				strikeoutenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'i' || text[ i + 1 ] == 'i' ) )
			{
				i += 2;
				italicenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'r' || text[ i + 1 ] == 'R' ) )
			{
				i += 2;
				textcolor = oldcolor;
				SetTextColor( hDC, color.ToUINT( ) );
				TempFont = CreateFontA( _fontsize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				boldenabled = FALSE;
				italicenabled = FALSE;
				underlineenabled = FALSE;
				strikeoutenabled = FALSE;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'c' || text[ i + 1 ] == 'C' ) )
			{
				oldcolor = textcolor;
				i += 2;
				if ( len - i > 7 )
				{
					char colorstr[ 11 ];
					colorstr[ 0 ] = '0';// text[ i + 2 ];
					colorstr[ 1 ] = 'x';//text[ i + 3 ];
					//A
					colorstr[ 2 ] = text[ i ];
					colorstr[ 3 ] = text[ i + 1 ];
					//R
					colorstr[ 4 ] = text[ i + 6 ];
					colorstr[ 5 ] = text[ i + 7 ];
					//G
					colorstr[ 6 ] = text[ i + 4 ];
					colorstr[ 7 ] = text[ i + 5 ];
					//B
					colorstr[ 8 ] = text[ i + 2 ];
					colorstr[ 9 ] = text[ i + 3 ];
					colorstr[ 10 ] = '\0';

					// Смысла от прозрачного текста нет так что считаем что FF это 0 прозрачность
					textcolor = strtoul( colorstr, NULL, 0 );
					if ( ( textcolor & 0xFF000000 ) == 0xFF000000 )
						textcolor -= 0xFF000000;

					SetTextColor( hDC, textcolor );
					i += 8;
				}
				continue;
			}
		}

		ostringstream strfordraw;

		for ( ; i < len; i++ )
		{
			if ( text[ i ] != '|' || len - i < 2 )
				strfordraw << text[ i ];
			else if ( text[ i ] == '|' )
			{
				break;
			}
		}

		if ( strfordraw.str( ).length( ) > 0 )
		{
			//MessageBoxA( 0, strfordraw.str( ).c_str( ), "Draw:", 0 );

			RECT newsize = { 0,0,0,0 };
			DrawTextA( hDC, strfordraw.str( ).c_str( ), -1, &newsize, DT_CALCRECT );
			if ( newline )
			{
				newline = FALSE;
				rect.left = x;
				rect.top += newsize.top + newsize.bottom;
			}
			DrawTextA( hDC, strfordraw.str( ).c_str( ), -1, &rect, DT_LEFT | DT_SINGLELINE );
			rect.left += newsize.right - newsize.left;
			strfordraw.str( "" );
			strfordraw.clear( );
		}
	}

	DeleteObject( NewFont );

	SelectObject( hDC, hBmpOld );
	GdiFlush( );
	ReleaseDC( NULL, hDC );



	Buffer tmpbuf = Buffer( );
	tmpbuf.buf = ( char * )pSrcData;
	tmpbuf.length = tmpRawImage.width * tmpRawImage.height * 3;
	//Buffer newbuf = Buffer( );
	//RAW2Tga( tmpbuf, newbuf, tmpRawImage.width, tmpRawImage.height, 3, "out.tga" );
	//FILE * f;
	//fopen_s( &f, "out.tga", "wb" );
	//fwrite( newbuf.buf, newbuf.length, 1, f );
	//fclose( f );



	RGBPix* tmpBitmapPixList = ( RGBPix* )pSrcData;


	for ( int x0 = 0; x0 < tmpRawImage.width; x0++ )
	{
		for ( int y0 = 0; y0 < tmpRawImage.height; y0++ )
		{
			if ( tmpBitmapPixList[ ArrayXYtoId( tmpRawImage.width, x0, y0 ) ].ToUINT( ) != 0 )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x0, y0 ) ] = tmpBitmapPixList[ ArrayXYtoId( tmpRawImage.width, x0, y0 ) ].ToRGBAPix( );
		}
	}


	DeleteDC( hDC );
	DeleteObject( hBmpOld );
	DeleteObject( hTempBmp );

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}


// Сохраняет RawImage в blp и делает доступным для использования в игре
int __stdcall SaveRawImageToGameFile( int RawImage, const char * filename, BOOL IsTga, BOOL enabled )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}


	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	tmpRawImage.filename = filename;
	Buffer tmpRawImageBuffer = tmpRawImage.img;
	Buffer ResultBuffer = Buffer( );
	if ( tmpRawImage.ingamebuffer.buf )
		tmpRawImage.ingamebuffer.Clear( );

	if ( enabled )
	{
		int mipmaps = 0;
		if ( IsTga )
			RAW2Tga( tmpRawImageBuffer, ResultBuffer, tmpRawImage.width, tmpRawImage.height, 4, filename );
		else
			CreatePalettedBLP( tmpRawImageBuffer, ResultBuffer, 256, filename, tmpRawImage.width, tmpRawImage.height, 4, 8, mipmaps );
		tmpRawImage.ingamebuffer = ResultBuffer;
	}

	tmpRawImage.ingame = enabled;
	return TRUE;
}


// Сохраняет RawImage на диск в TGA по выбранному пути
int __stdcall DumpRawImageToFile( int RawImage, const char * filename )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct tmpRawImage = ListOfRawImages[ RawImage ];
	Buffer outbuffer;
	Buffer inbuffer = tmpRawImage.img;
	RAW2Tga( inbuffer, outbuffer, tmpRawImage.width, tmpRawImage.height, 4, filename );
	FILE * f;
	fopen_s( &f, filename, "wb" );
	if ( f )
	{
		fwrite( outbuffer.buf, outbuffer.length, 1, f );
		fclose( f );
	}


	return TRUE;
}


// Получает RawImage из списка RawImages по имени файла.
int __stdcall GetRawImageByFile( const char * filename )
{
	int id = 0;
	for ( RawImageStruct & s : ListOfRawImages )
	{
		if ( ToLower( s.filename ) == ToLower( filename ) )
			return id;
		id++;
	}

	return 0;
}

// Получает ширину RawImage
int __stdcall RawImage_GetWidth( int RawImage )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return 64;
	}

	return ListOfRawImages[ RawImage ].width;
}

// Получает высоту RawImage
int __stdcall RawImage_GetHeight( int RawImage )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return 64;
	}

	return ListOfRawImages[ RawImage ].width;
}

// Изменяет размер RawImage
int __stdcall RawImage_Resize( int RawImage, int newwidth, int newheight )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	Buffer tmpOldBuffer = tmpRawImage.img;
	Buffer tmpNewBuffer = Buffer( );
	ScaleImage( ( unsigned char * )tmpOldBuffer.buf, tmpRawImage.width, tmpRawImage.height, newwidth, newheight, 4, tmpNewBuffer );
	tmpOldBuffer.Clear( );
	tmpRawImage.img = tmpNewBuffer;
	tmpRawImage.height = newheight;
	tmpRawImage.width = newwidth;
	if ( tmpRawImage.used_for_overlay )
		tmpRawImage.needResetTexture = TRUE;

	return TRUE;
}

// Рисует RawImage по заданным координатам (от 0.0 до 1.0) в игре. 
int __stdcall RawImage_DrawOverlay( int RawImage, BOOL enabled, float xpos, float ypos, float xsize, float ysize )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	tmpRawImage.used_for_overlay = enabled;
	tmpRawImage.overlay_x = xpos;
	tmpRawImage.overlay_y = ypos;
	tmpRawImage.size_x = xsize;
	tmpRawImage.size_y = ysize;

	return TRUE;
}

RawImageCallbackData * GlobalRawImageCallbackData = NULL;

int __stdcall RawImage_AddCallback( int RawImage, const char * MouseActionCallback, RawImageCallbackData * callbackdata, unsigned int events )
{
	GlobalRawImageCallbackData = callbackdata;


	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];

	if ( !MouseActionCallback || MouseActionCallback[ 0 ] == '\0' )
	{
		tmpRawImage.MouseCallback = FALSE;
		tmpRawImage.MouseActionCallback = JassString( );
	}
	else
	{
		tmpRawImage.MouseActionCallback = JassString( );
		str2jstr( &tmpRawImage.MouseActionCallback, MouseActionCallback );
		tmpRawImage.MouseCallback = TRUE;
	}

	tmpRawImage.events = events;
	tmpRawImage.IsMouseDown = FALSE;
	tmpRawImage.IsMouseEntered = FALSE;

	return TRUE;
}


void RawImageGlobalCallbackFunc( RawImageEventType callbacktype, float mousex, float mousey )
{
	if ( !GlobalRawImageCallbackData )
		return;

	GlobalRawImageCallbackData->IsAltPressed = IsKeyPressed( VK_MENU );
	GlobalRawImageCallbackData->IsCtrlPressed = IsKeyPressed( VK_CONTROL );
	GlobalRawImageCallbackData->EventType = callbacktype;
	float ScreenX = *GetWindowXoffset;
	float ScreenY = *GetWindowYoffset;

	float zoomx = ScreenX / DesktopScreen_Width;
	float zoomy = ScreenY / DesktopScreen_Height;

	float mouseposx = mousex / ScreenX;
	float mouseposy = mousey / ScreenY;
	GlobalRawImageCallbackData->mousex = mouseposx;
	GlobalRawImageCallbackData->mousey = mouseposy;

	for ( unsigned int i = ListOfRawImages.size( ) - 1; i >= 0; i-- )
	{
		RawImageStruct & img = ListOfRawImages[ i ];
		if ( img.used_for_overlay &&
			img.MouseCallback &&
			( img.events & ( unsigned int )callbacktype ) > 0 )
		{
			BOOL MouseEnteredInRawImage = FALSE;
			float posx = ScreenX * img.overlay_x;
			float posy = ScreenY * img.overlay_y;
			float sizex = img.width * zoomx;
			float sizey = img.height * zoomy;
			//posy -= sizey;
			GlobalRawImageCallbackData->RawImage = img.RawImage;


			if ( mousex > posx && mousex < posx + sizex && mousey > posy && mousey < posy + sizey )
			{
				MouseEnteredInRawImage = TRUE;
			}


			switch ( callbacktype )
			{
			case RawImageEventType::MouseUp:
				if ( img.IsMouseDown )
				{
					img.IsMouseDown = FALSE;

					if ( MouseEnteredInRawImage )
						GlobalRawImageCallbackData->EventType = RawImageEventType::MouseClick;

					ExecuteFunc( &img.MouseActionCallback );
					return;
				}
				break;
			case RawImageEventType::MouseDown:
				if ( !img.IsMouseDown && MouseEnteredInRawImage )
				{
					img.IsMouseDown = TRUE;
					ExecuteFunc( &img.MouseActionCallback );
					return;
				}
				break;
			case RawImageEventType::MouseClick:
				break;
			case RawImageEventType::MouseEnter:
				break;
			case RawImageEventType::MouseLeave:
				break;
			case RawImageEventType::MouseMove:
				if ( img.IsMouseEntered )
				{
					if ( !MouseEnteredInRawImage )
					{
						img.IsMouseEntered = FALSE;
						GlobalRawImageCallbackData->EventType = RawImageEventType::MouseLeave;
						ExecuteFunc( &img.MouseActionCallback );
					}
				}
				else
				{
					if ( MouseEnteredInRawImage )
					{
						img.IsMouseEntered = TRUE;
						GlobalRawImageCallbackData->EventType = RawImageEventType::MouseEnter;
						ExecuteFunc( &img.MouseActionCallback );
					}
				}
				break;
			case RawImageEventType::ALL:
				if ( img.IsMouseDown )
				{
					img.IsMouseDown = FALSE;
					GlobalRawImageCallbackData->EventType = RawImageEventType::MouseUp;
					ExecuteFunc( &img.MouseActionCallback );
				}
				if ( img.IsMouseEntered )
				{
					img.IsMouseEntered = FALSE;
					GlobalRawImageCallbackData->EventType = RawImageEventType::MouseLeave;
					ExecuteFunc( &img.MouseActionCallback );
				}
				break;
			default:
				break;
			}




		}
	}

}


//
//void ApplyIconFrameFilter2( string filename, int * OutDataPointer, size_t * OutSize )
//{
//	int RawImage = CreateRawImage( 128, 128, RGBAPix( ) );
//	//RawImage_Resize( RawImage, 128, 128 );
//	int RawImage2 = LoadRawImage( filename.c_str( ) );
//	RawImage_DrawImg( RawImage, RawImage2, 32, 32 );
//	RGBAPix tmppix = RGBAPix( );
//	RawImage_EraseCircle( RawImage, 64, 64, 29, TRUE );
//	RawImage_DrawCircle( RawImage, 64, 64, 35, 6, tmppix.RGBAPixWar3( 0, 255, 0, 255 ) );
//
//
//	RawImage_DrawText( RawImage, "|C00FF0000RED|r |CFF00FF00GREEN|r |CFF0000FFBLUE|r", 10, 10, tmppix.RGBAPixWar3( 255, 0, 0, 0 ) );
//
//	SaveRawImageToGameFile( RawImage, ( filename + "_frame.blp" ).c_str( ), FALSE, TRUE );
//	DumpRawImageToFile( RawImage, "temp.tga" );
//
//	RawImage_DrawOverlay( RawImage, TRUE, 0.1f, 0.1f, 0, 0 );
//	ApplyIconFrameFilter3( filename, OutDataPointer, OutSize );
//}
//
//

void ApplyIconFrameFilter( string filename, int * OutDataPointer, size_t * OutSize )
{
	int RawImage = CreateRawImage( 128, 128, RGBAPix( ) );
	int RawImage2 = LoadRawImage( filename.c_str( ) );
	RawImage_DrawImg( RawImage, RawImage2, 32, 32, 0 );
	RGBAPix tmppix = RGBAPix( );
	SaveRawImageToGameFile( RawImage, ( filename + "_frame.blp" ).c_str( ), FALSE, TRUE );
}




void ClearAllRawImages( )
{
	for ( RawImageStruct & s : ListOfRawImages )
	{
		s.used_for_overlay = FALSE;
		if ( s.img.buf )
			s.img.Clear( );
		if ( s.ingame )
		{
			if ( s.ingamebuffer.buf )
				s.ingamebuffer.Clear( );
			s.ingame = FALSE;
		}
	}
	ListOfRawImages.clear( );
	RGBAPix tmppix = RGBAPix( );
	CreateRawImage( 64, 64, tmppix.RGBAPixWar3( 0, 255, 0, 255 ) );
}


float __stdcall GetScreenWidth( int )
{
	return DesktopScreen_Width;
}
float __stdcall GetScreenHeight( int )
{
	return DesktopScreen_Height;
}

float __stdcall GetWindowWidth( int )
{
	if ( *InGame )
		return  *GetWindowXoffset;
	return DesktopScreen_Width;
}
float __stdcall GetWindowHeight( int )
{
	if ( *InGame )
		return  *GetWindowYoffset;
	return DesktopScreen_Height;
}