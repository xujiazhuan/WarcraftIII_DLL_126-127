
#include "Main.h"
extern "C" { FILE __iob_func[ 3 ] = { *stdin,*stdout,*stderr }; }
#pragma comment(lib,"legacy_stdio_definitions.lib")
#include "blpaletter.h"


#define MASK_56 (((u_int64_t)1<<56)-1) /* i.e., (u_int64_t)0xffffffffffffff */

#include "fnv.h"

u_int64_t GetBufHash( const char * data, size_t data_len )
{
	u_int64_t hash;
	hash = fnv_64_buf( ( void * ) data, ( size_t ) data_len, FNV1_64_INIT );
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


BOOL GetFromIconMdlCache( const char * filename, ICONMDLCACHE * iconhelperout )
{
	size_t filelen = strlen( filename );
	u_int64_t hash = GetBufHash( filename, filelen );
	for ( ICONMDLCACHE ih : ICONMDLCACHELIST )
	{
		if ( ih.hashlen == filelen && ih.hash == hash )
		{
			*iconhelperout = ih;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsFileRedirected( char * filename )
{
	for ( FileRedirectStruct DotaRedirectHelp : FileRedirectList )
	{
		if ( _stricmp( filename, DotaRedirectHelp.NewFilePath ) == 0 )
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsMemInCache( int addr )
{
	for ( ICONMDLCACHE ih : ICONMDLCACHELIST )
	{
		if ( ( int ) ih.buf == addr )
			return TRUE;
	}
	return FALSE;
}


void SaveNewIHelperIcon( char * filename, char * buf, size_t buflen )
{
	size_t filelen = strlen( filename );
	u_int64_t hash = GetBufHash( filename, filelen );
	ICONMDLCACHE tmpih;
	tmpih.buf = buf;
	tmpih.hashlen = filelen;
	tmpih.hash = hash;
	tmpih.size = buflen;
	ICONMDLCACHELIST.push_back( tmpih );
}

void FreeAllIHelpers( )
{
	for ( ICONMDLCACHE ih : ICONMDLCACHELIST )
	{
		delete[ ] ih.buf;
	}

	ICONMDLCACHELIST.clear( );
	FileRedirectList.clear( );
}



void WINAPI SMemZero( LPVOID lpDestination, DWORD dwLength )
{
	DWORD dwPrevLen = dwLength;
	LPDWORD lpdwDestination = ( LPDWORD ) lpDestination;
	LPBYTE lpbyDestination;

	dwLength >>= 2;

	while ( dwLength-- )
		*lpdwDestination++ = 0;

	lpbyDestination = ( LPBYTE ) lpdwDestination;

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
	LPDWORD lpdwDestination = ( LPDWORD ) lpDestination, lpdwSource = ( LPDWORD ) lpSource;
	LPBYTE lpbyDestination, lpbySource;

	dwLength >>= 2;

	while ( dwLength-- )
		*lpdwDestination++ = *lpdwSource++;

	lpbyDestination = ( LPBYTE ) lpdwDestination;
	lpbySource = ( LPBYTE ) lpdwSource;

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

// Функция замены текста в строке.
char *repl_string( const char *str, const char *from, const char *to )
{
	AddNewLineToDotaHelperLog( "repl_string" );
	size_t cache_sz_inc = 16;

	const size_t cache_sz_inc_factor = 3;

	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	ptrdiff_t *pos_cache = NULL;
	size_t cache_sz = 0;
	size_t cpylen, orglen, retlen, tolen = 0, fromlen = strlen( from );

	while ( ( pstr2 = strstr( pstr, from ) ) != NULL )
	{
		count++;
		if ( cache_sz < count )
		{
			cache_sz += cache_sz_inc;
			pos_cache = ( ptrdiff_t * ) realloc( pos_cache, sizeof( *pos_cache ) * cache_sz );
			if ( !pos_cache )
			{
				goto end_repl_string;
			}
			cache_sz_inc *= cache_sz_inc_factor;
			if ( cache_sz_inc > cache_sz_inc_max )
			{
				cache_sz_inc = cache_sz_inc_max;
			}
		}

		pos_cache[ count - 1 ] = pstr2 - str;
		pstr = pstr2 + fromlen;
	}

	orglen = pstr - str + strlen( pstr );
	if ( count > 0 )
	{
		tolen = strlen( to );
		retlen = orglen + ( tolen - fromlen ) * count;
	}
	else	retlen = orglen;
	ret = ( char* ) malloc( retlen + 1 );
	if ( ret == NULL )
	{
		goto end_repl_string;
	}

	if ( count == 0 )
	{
		CopyMemory( ret, str, retlen + 1 );
	}
	else
	{
		pret = ret;
		memcpy( pret, str, ( size_t ) pos_cache[ 0 ] );
		pret += pos_cache[ 0 ];
		for ( i = 0; i < count; i++ )
		{
			memcpy( pret, to, tolen );
			pret += tolen;
			pstr = str + pos_cache[ i ] + fromlen;
			cpylen = ( i == count - 1 ? orglen : pos_cache[ i + 1 ] ) - pos_cache[ i ] - fromlen;
			memcpy( pret, pstr, cpylen );
			pret += cpylen;
		}
		ret[ retlen ] = '\0';
	}

	end_repl_string:
	free( pos_cache );
	return ret;
}


GameGetFile GameGetFile_org = NULL;
GameGetFile GameGetFile_ptr;

int idddd = 0;

void ApplyTerrainFilter( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL IsTga )
{
	AddNewLineToDotaHelperLog( "ApplyTerrainFilter" );
	ICONMDLCACHE tmpih;
	BOOL FoundOldHelper = GetFromIconMdlCache( filename, &tmpih );
	if ( FoundOldHelper )
	{
		*OutDataPointer = ( int ) tmpih.buf;
		*OutSize = tmpih.size;
		return;
	}

	char * originfiledata = ( char * ) ( int ) *OutDataPointer;
	size_t sz = *OutSize;


	int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 8, compress = 1, alphaenconding = 5;
	unsigned long rawImageSize = 0;
	Buffer InBuffer;
	InBuffer.buf = ( char* ) originfiledata;
	InBuffer.length = sz;
	Buffer OutBuffer;
	if ( IsTga )
		rawImageSize = ( unsigned long ) TGA2Raw( InBuffer, OutBuffer, w, h, bpp, filename );
	else
		rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename );
	if ( rawImageSize > 0 )
	{
		BGRAPix * OutImage = ( BGRAPix* ) OutBuffer.buf;
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

		CreatePalettedBLP( OutBuffer, ResultBuffer, 256, filename, w, h, bpp, alphaflag, mipmaps );

		if ( OutBuffer.buf != NULL )
		{
			OutBuffer.length = 0;
			delete[ ] OutBuffer.buf;
			OutBuffer.buf = 0;
		}

		if ( ResultBuffer.buf != NULL )
		{
			//	MessageBox( 0, "OK5", "OK5", 0 );
			tmpih.buf = ResultBuffer.buf;
			tmpih.size = ResultBuffer.length;
			tmpih.hashlen = strlen( filename );
			tmpih.hash = GetBufHash( filename, tmpih.hashlen );
			ICONMDLCACHELIST.push_back( tmpih );
			if ( !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* ) *OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int ) tmpih.buf;
			*OutSize = tmpih.size;
		}
	}

}



__declspec( dllexport ) int __stdcall ApplyTerrainFilterDirectly( char * filename, int * OutDataPointer, size_t * OutSize, BOOL IsTga )
{
	ApplyTerrainFilter( filename, OutDataPointer, OutSize, IsTga );
	return 0;
}


void ApplyUnitFilter( char * filename, int * OutDataPointer, size_t * OutSize )
{
	AddNewLineToDotaHelperLog( "ApplyUnitFilter" );
	ICONMDLCACHE tmpih;
	BOOL FoundOldHelper = GetFromIconMdlCache( filename, &tmpih );
	if ( FoundOldHelper )
	{
		*OutDataPointer = ( int ) tmpih.buf;
		*OutSize = tmpih.size;
		return;
	}

	char * originfiledata = ( char * ) ( int ) *OutDataPointer;
	size_t sz = *OutSize;


	int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 0, compress = 0, alphaenconding = 0;
	unsigned long rawImageSize = 0;
	Buffer InBuffer;
	InBuffer.buf = ( char* ) originfiledata;
	InBuffer.length = sz;
	Buffer OutBuffer;

	rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename );
	if ( rawImageSize > 0 )
	{
		BGRAPix * OutImage = ( BGRAPix* ) OutBuffer.buf;

		for ( unsigned long i = 0; i < OutBuffer.length / 4; i++ )
		{
			if ( OutImage[ i ].A == 0xFF && ( OutImage[ i ].G > 20 || OutImage[ i ].B > 20 || OutImage[ i ].R > 20 ) )
			{
				OutImage[ i ].R = 5;
				OutImage[ i ].G = 5;
				OutImage[ i ].B = 5;
			}
			else if ( OutImage[ i ].A == 0xFF )
			{
				OutImage[ i ].R = 70;
				OutImage[ i ].G = 70;
				OutImage[ i ].B = 70;
			}
		}


		Buffer ResultBuffer;

		CreatePalettedBLP( OutBuffer, ResultBuffer, 256, filename, w, h, bpp, alphaflag, mipmaps );

		if ( OutBuffer.buf != NULL )
		{
			OutBuffer.length = 0;
			delete[ ] OutBuffer.buf;
			OutBuffer.buf = 0;
		}

		if ( ResultBuffer.buf != NULL )
		{
			//	MessageBox( 0, "OK5", "OK5", 0 );
			tmpih.buf = ResultBuffer.buf;
			tmpih.size = ResultBuffer.length;
			tmpih.hashlen = strlen( filename );
			tmpih.hash = GetBufHash( filename, tmpih.hashlen );
			ICONMDLCACHELIST.push_back( tmpih );
			if ( !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* ) *OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int ) tmpih.buf;
			*OutSize = tmpih.size;
		}
	}

}




void ApplyIconFilter( const char * filename, int * OutDataPointer, size_t * OutSize )
{
	AddNewLineToDotaHelperLog( "ApplyIconFilter" );
	ICONMDLCACHE tmpih;
	BOOL FoundOldHelper = GetFromIconMdlCache( filename, &tmpih );
	if ( FoundOldHelper )
	{
		*OutDataPointer = ( int ) tmpih.buf;
		*OutSize = tmpih.size;
		return;
	}

	char * originfiledata = ( char * ) ( int ) *OutDataPointer;
	size_t sz = *OutSize;


	int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 0, compress = 0, alphaenconding = 0;
	unsigned long rawImageSize = 0;
	Buffer InBuffer;
	InBuffer.buf = ( char* ) originfiledata;
	InBuffer.length = sz;
	Buffer OutBuffer;

	rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename );
	if ( rawImageSize > 0 )
	{
		BGRAPix * OutImage = ( BGRAPix* ) OutBuffer.buf;
		BGRAPix BlackPix;

		BlackPix.A = 0xFF;
		BlackPix.R = 0;
		BlackPix.G = 0;
		BlackPix.B = 0;


		/*	for ( int i = 0; i < h; i++ )
			{
				if ( i < 5 )
				{
					for ( int n = 0; n < w; n++ )
					{
						OutImage[ n + i * w ] = BlackPix;
					}
				}


				if ( h - 5 < i )
				{
					for ( int n = 0; n < w; n++ )
					{
						OutImage[ n + i * w ] = BlackPix;
					}
				}

				for ( int n = 1; n < 5; n++ )
				{
					OutImage[ n + i * w ] = BlackPix;
				}


				for ( int n = 1; n < 5; n++ )
				{
					OutImage[ w - n + i * w ] = BlackPix;
				}

			}*/


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


		/*	for ( int x = 0; x < 8; x++ )
			{
				for ( int y = 0; y < 8; y++ )
				{
					// x+4

				}
			}*/

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

		/*for ( unsigned long i = 0; i < OutBuffer.length / 4; i++ )
		{
			BGRAPix CurPix = OutImage[ i ];
			int ave = (( min( min( CurPix.R, CurPix.G ), CurPix.B ) + max( max( CurPix.R, CurPix.G ), CurPix.B ) ) / 2);
			CurPix.R = ( unsigned char ) ( ( ave + CurPix.R ) / 4 );
			CurPix.G = ( unsigned char ) ( ( ave + CurPix.G ) / 4 );
			CurPix.B = ( unsigned char ) ( ( ave + CurPix.B ) / 4 );
			OutImage[ i ] = CurPix;
		}*/



		Buffer ResultBuffer;

		CreatePalettedBLP( OutBuffer, ResultBuffer, 256, filename, w, h, bpp, alphaflag, mipmaps );

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
			tmpih.hashlen = strlen( filename );
			tmpih.hash = GetBufHash( filename, tmpih.hashlen );
			ICONMDLCACHELIST.push_back( tmpih );
			if ( !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* ) *OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int ) tmpih.buf;
			*OutSize = tmpih.size;
		}
	}

}

BOOL FixDisabledIconPath( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{
	AddNewLineToDotaHelperLog( "FixDisabledIconPath" );


	if ( !strstr( filename, "blp" ) )
		return 0;


	BOOL CreateDarkIcon = FALSE;
	BOOL result = FALSE;
	if ( strstr( filename, DisabledIconSignature2 ) )
	{
		char * tmpstr = repl_string( filename, DisabledIconSignature2, "\\" );
		result = GameGetFile_ptr( tmpstr, OutDataPointer, OutSize, unknown );
		free( tmpstr );
		if ( result )
			CreateDarkIcon = TRUE;
	}


	if ( !result )
	{
		CreateDarkIcon = FALSE;
		if ( strstr( filename, DisabledIconSignature ) )
		{
			char * tmpstr = repl_string( filename, DisabledIconSignature, "\\" );
			result = GameGetFile_ptr( tmpstr, OutDataPointer, OutSize, unknown );
			free( tmpstr );
			if ( result )
				CreateDarkIcon = TRUE;
		}

	}

	if ( !result )
	{
		char * tmpstr = repl_string( filename, CommandButtonsDisabledIconSignature, "PassiveButtons\\" );
		result = GameGetFile_ptr( tmpstr, OutDataPointer, OutSize, unknown );
		free( tmpstr );
		if ( result )
			CreateDarkIcon = TRUE;
	}


	if ( !result )
	{
		char * tmpstr = repl_string( filename, CommandButtonsDisabledIconSignature, "AutoCastButtons\\" );
		result = GameGetFile_ptr( tmpstr, OutDataPointer, OutSize, unknown );
		free( tmpstr );
		if ( result )
			CreateDarkIcon = TRUE;
	}


	if ( CreateDarkIcon )
	{
		ApplyIconFilter( filename, OutDataPointer, OutSize );
	}
	//else MessageBox( 0, filename, "Bad file path:", 0 );

	return result;
}




vector<ModelCollisionFixStruct> ModelCollisionFixList;
vector<ModelTextureFixStruct> ModelTextureFixList;
vector<ModelPatchStruct> ModelPatchList;
vector<ModelRemoveTagStruct> ModelRemoveTagList;
vector<ModelSequenceReSpeedStruct> ModelSequenceReSpeedList;
vector<ModelSequenceValueStruct> ModelSequenceValueList;


__declspec( dllexport ) int __stdcall FixModelCollisionSphere( const char * mdlpath, float X, float Y, float Z, float Radius )
{
	ModelCollisionFixStruct tmpModelFix;
	sprintf_s( tmpModelFix.FilePath, 512, "%s", mdlpath );
	tmpModelFix.X = X;
	tmpModelFix.Y = Y;
	tmpModelFix.Z = Z;
	tmpModelFix.Radius = Radius;
	ModelCollisionFixList.push_back( tmpModelFix );
	return 0;
}


__declspec( dllexport ) int __stdcall FixModelTexturePath( const char * mdlpath, int textureid, const char * texturenew )
{
	ModelTextureFixStruct tmpModelFix;
	sprintf_s( tmpModelFix.FilePath, 512, "%s", mdlpath );
	tmpModelFix.TextureID = textureid;
	sprintf_s( tmpModelFix.NewTexturePath, 260, "%s", texturenew );
	ModelTextureFixList.push_back( tmpModelFix );
	return 0;
}



__declspec( dllexport ) int __stdcall PatchModel( const char * mdlpath, const char * pathPatch )
{
	ModelPatchStruct tmpModelFix;
	sprintf_s( tmpModelFix.FilePath, 512, "%s", mdlpath );
	sprintf_s( tmpModelFix.patchPath, 512, "%s", pathPatch );
	ModelPatchList.push_back( tmpModelFix );
	return 0;
}

__declspec( dllexport ) int __stdcall RemoveTagFromModel( const char * mdlpath, const char * tagname )
{
	ModelRemoveTagStruct tmpModelFix;
	sprintf_s( tmpModelFix.FilePath, 512, "%s", mdlpath );
	sprintf_s( tmpModelFix.TagName, 5, "%s", tagname );
	ModelRemoveTagList.push_back( tmpModelFix );
	return 0;
}

__declspec( dllexport ) int __stdcall ChangeAnimationSpeed( const char * mdlpath, const char * SeqenceName, float Speed )
{
	ModelSequenceReSpeedStruct tmpModelFix;
	sprintf_s( tmpModelFix.FilePath, 512, "%s", mdlpath );
	sprintf_s( tmpModelFix.AnimationName, 100, "%s", SeqenceName );
	tmpModelFix.SpeedUp = Speed;
	ModelSequenceReSpeedList.push_back( tmpModelFix );
	return 0;
}



__declspec( dllexport ) int __stdcall SetSequenceValue( const char * mdlpath, const char * SeqenceName, int Indx, float Value )
{
	if ( Indx < 0 || Indx > 6 )
		return -1;

	ModelSequenceValueStruct tmpModelFix;
	sprintf_s( tmpModelFix.FilePath, 512, "%s", mdlpath );
	sprintf_s( tmpModelFix.AnimationName, 100, "%s", SeqenceName );
	tmpModelFix.Indx = Indx;
	tmpModelFix.Value = Value;
	ModelSequenceValueList.push_back( tmpModelFix );
	return 0;
}


struct Mdx_Texture        //NrOfTextures = ChunkSize / 268
{
	int ReplaceableId;
	CHAR FileName[ 260 ];
	UINT Flags;                       //#1 - WrapWidth
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
	UINT Flags;                       //0 - Looping
									   //1 - NonLooping
	FLOAT Rarity;
	UINT SyncPoint;

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
	UINT GlobalSequenceId;
};

struct Mdx_Tracks
{
	int NrOfTracks;
	UINT GlobalSequenceId;

};
struct Mdx_Node
{
	UINT InclusiveSize;

	CHAR Name[ 80 ];

	UINT ObjectId;
	UINT ParentId;
	UINT Flags;                         //0        - Helper
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
	UINT InclusiveSize;

	FLOAT Alpha;
	UINT Flags;                       //#1 - DropShadow
									   //#2 - Color
	Mdx_FLOAT3 Color;

	UINT GeosetId;

};

void ProcessNodeAnims( BYTE * ModelBytes, size_t _offset, vector<int *> & TimesForReplace )
{
	Mdx_Track tmpTrack;
	size_t offset = _offset;
	if ( memcmp( &ModelBytes[ offset ], "KGTR", 4 ) == 0 )
	{
		offset += 4;
		memcpy( &tmpTrack, &ModelBytes[ offset ], sizeof( Mdx_Track ) );
		offset += sizeof( Mdx_Track );
		for ( int i = 0; i < tmpTrack.NrOfTracks; i++ )
		{
			TimesForReplace.push_back( ( int * ) &ModelBytes[ offset ] );
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
			TimesForReplace.push_back( ( int * ) &ModelBytes[ offset ] );
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
			TimesForReplace.push_back( ( int * ) &ModelBytes[ offset ] );
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
			TimesForReplace.push_back( ( int * ) &ModelBytes[ offset ] );
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
			TimesForReplace.push_back( ( int * ) &ModelBytes[ offset ] );
			offset += ( tmpTrack.InterpolationType > 1 ? 16 : 8 );
		}
	}

}

void ProcessMdx( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{
	AddNewLineToDotaHelperLog( "ProcessModel" );
	BYTE * ModelBytes = ( BYTE* ) *OutDataPointer;
	size_t sz = *OutSize;



	for ( unsigned int i = 0; i < ModelSequenceValueList.size( ); i++ )
	{
		ModelSequenceValueStruct mdlfix = ModelSequenceValueList[ i ];
		if ( _stricmp( filename, mdlfix.FilePath ) == 0 )
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

						size_t currenttagsize = *( size_t* ) &ModelBytes[ offset ];
						size_t SequencesCount = currenttagsize / sizeof( Mdx_Sequence );

						size_t newoffset = offset + currenttagsize;
						offset += 4;
						while ( SequencesCount > 0 )
						{
							SequencesCount--;
							memcpy( &tmpSequence, &ModelBytes[ offset ], sizeof( Mdx_Sequence ) );

							if ( mdlfix.AnimationName == 0 || *mdlfix.AnimationName == '\0' || _stricmp( mdlfix.AnimationName, tmpSequence.Name ) == 0 )
							{
								size_t NeedPatchOffset = offset + 104 + ( mdlfix.Indx * 4 );
								*( float* ) &ModelBytes[ NeedPatchOffset ] = mdlfix.Value;
							}

							offset += sizeof( Mdx_Sequence );
						}
						offset = newoffset;
					}
					else
					{
						offset += 4;
						offset += *( int* ) &ModelBytes[ offset ];
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
				MessageBox( 0, "Ok dump", "DUMP", 0 );
			}


			ModelSequenceValueList.erase( ModelSequenceValueList.begin( ) + ( int ) i );
			i--;
		}

	}

	for ( unsigned int i = 0; i < ModelSequenceReSpeedList.size( ); i++ )
	{
		ModelSequenceReSpeedStruct mdlfix = ModelSequenceReSpeedList[ i ];
		if ( _stricmp( filename, mdlfix.FilePath ) == 0 )
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

						size_t currenttagsize = *( size_t* ) &ModelBytes[ offset ];
						size_t SequencesCount = currenttagsize / sizeof( Mdx_Sequence );

						size_t newoffset = offset + currenttagsize;
						offset += 4;
						while ( SequencesCount > 0 )
						{
							SequencesCount--;
							memcpy( &tmpSequence, &ModelBytes[ offset ], sizeof( Mdx_Sequence ) );


							if ( _stricmp( mdlfix.AnimationName, tmpSequence.Name ) == 0 )
							{
								ReplaceSequenceID = SequenceID;
							}


							Mdx_SequenceTime CurrentSequenceTime;
							CurrentSequenceTime.IntervalStart = ( int* ) &ModelBytes[ offset + 80 ];
							CurrentSequenceTime.IntervalEnd = ( int* ) &ModelBytes[ offset + 84 ];
							Sequences.push_back( CurrentSequenceTime );

							offset += sizeof( Mdx_Sequence );
							SequenceID++;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "BONE", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
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
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
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
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* ) &ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "ATCH", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* ) &ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "PREM", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* ) &ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "PRE2", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* ) &ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "GEOA", 4 ) == 0 )
					{
						Mdx_GeosetAnimation tmpGeosetAnimation;
						offset += 4;
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
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
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							size_t size_of_this_struct = *( size_t* ) &ModelBytes[ offset ];
							offset += 4;
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else if ( memcmp( &ModelBytes[ offset ], "EVTS", 4 ) == 0 )
					{
						offset += 4;
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
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
									TimesForReplace.push_back( ( int* ) &ModelBytes[ offset ] );
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
						size_t newoffset = offset + *( size_t* ) &ModelBytes[ offset ];
						offset += 4;
						Mdx_Node tmpNode;
						while ( newoffset > offset )
						{
							memcpy( &tmpNode, &ModelBytes[ offset ], sizeof( Mdx_Node ) );
							ProcessNodeAnims( ModelBytes, offset + sizeof( Mdx_Node ), TimesForReplace );
							offset += tmpNode.InclusiveSize;
							UINT size_of_this_struct = *( UINT* ) &ModelBytes[ offset ];
							offset += 4;
							size_of_this_struct = size_of_this_struct == 0 ? 24u : 16u;
							offset += size_of_this_struct;
						}
						offset = newoffset;
					}
					else
					{
						offset += 4;
						offset += *( int* ) &ModelBytes[ offset ];
					}

					offset += 4;
				}
			}

			if ( ReplaceSequenceID != -1 )
			{

				int SeqEndTime = *Sequences[ ( unsigned int ) ReplaceSequenceID ].IntervalEnd;
				int SeqStartTime = *Sequences[ ( unsigned int ) ReplaceSequenceID ].IntervalStart;
				int NewEndTime = SeqStartTime + ( int ) ( ( SeqEndTime - SeqStartTime ) / mdlfix.SpeedUp );
				int AddTime = NewEndTime - SeqEndTime;

				for ( unsigned int n = 0; n < Sequences.size( ); n++ )
				{
					if ( *Sequences[ n ].IntervalStart >= SeqEndTime )
					{
						*Sequences[ n ].IntervalStart += AddTime;
						*Sequences[ n ].IntervalEnd += AddTime;
					}
				}

				*Sequences[ ( unsigned int ) ReplaceSequenceID ].IntervalEnd = NewEndTime;

				for ( int * dwTime : TimesForReplace )
				{
					if ( *dwTime >= SeqEndTime )
					{
						*dwTime += AddTime;
					}
					else if ( *dwTime <= SeqEndTime && *dwTime >= SeqStartTime )
					{
						*dwTime = ( int ) SeqStartTime + ( int ) ( ( float ) ( *dwTime - SeqStartTime ) / mdlfix.SpeedUp );
					}
				}


				if ( IsKeyPressed( '0' ) && FileExist( ".\\Test1234.mdx" ) )
				{
					FILE *f;
					fopen_s( &f, ".\\Test1234.mdx", "wb" );
					fwrite( ModelBytes, sz, 1, f );
					fclose( f );
					MessageBox( 0, "Ok dump", "DUMP", 0 );
				}

			}


			TimesForReplace.clear( );
			Sequences.clear( );

			ModelSequenceReSpeedList.erase( ModelSequenceReSpeedList.begin( ) + ( int ) i );
			i--;
		}

	}


	for ( unsigned int i = 0; i < ModelRemoveTagList.size( ); i++ )
	{
		ModelRemoveTagStruct mdlfix = ModelRemoveTagList[ i ];
		if ( _stricmp( filename, mdlfix.FilePath ) == 0 )
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
					if ( memcmp( &ModelBytes[ offset ], mdlfix.TagName, 4 ) == 0 )
					{

						TagFound = TRUE;
						TagStartOffset = offset;
						offset += 4;
						offset += *( int* ) &ModelBytes[ offset ];
						TagSize = offset - TagStartOffset;

					}
					else
					{
						offset += 4;
						offset += *( int* ) &ModelBytes[ offset ];
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


			ModelRemoveTagList.erase( ModelRemoveTagList.begin( ) + ( int ) i );
			i--;
		}
	}




	if ( !FullPatchData.empty( ) )
		FullPatchData.clear( );
	for ( unsigned int i = 0; i < ModelPatchList.size( ); i++ )
	{
		ModelPatchStruct mdlfix = ModelPatchList[ i ];
		if ( _stricmp( filename, mdlfix.FilePath ) == 0 )
		{
			int PatchFileData;
			size_t PatchFileSize;

			if ( GameGetFile_ptr( mdlfix.patchPath, &PatchFileData, &PatchFileSize, unknown ) )
			{
				FullPatchData.insert( FullPatchData.end( ), ( char* ) ( PatchFileData ), ( char* ) ( PatchFileData + PatchFileSize ) );

			}

			ModelPatchList.erase( ModelPatchList.begin( ) + ( int ) i );
			i--;
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
			*OutDataPointer = ( int ) tmpih->buf;
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

			tmpih->hashlen = strlen( filename );
			tmpih->hash = GetBufHash( filename, tmpih->hashlen );

			ICONMDLCACHELIST.push_back( *tmpih );


			Storm_403_org( ( void* ) *OutDataPointer, "delete", -1, 0 );


			*OutDataPointer = ( int ) tmpih->buf;
			*OutSize = tmpih->size;

			ModelBytes = ( BYTE * ) tmpih->buf;
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
		if ( _stricmp( filename, mdlfix.FilePath ) == 0 )
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
						size_t newoffset = offset + *( int* ) &ModelBytes[ offset ];
						offset += 4;
						offset += *( int* ) &ModelBytes[ offset ];
						int shapetype = *( int* ) &ModelBytes[ offset ];

						if ( shapetype == 2 )
						{
							offset += 4;
							*( float* ) &ModelBytes[ offset ] = mdlfix.X;
							offset += 4;
							*( float* ) &ModelBytes[ offset ] = mdlfix.Y;
							offset += 4;
							*( float* ) &ModelBytes[ offset ] = mdlfix.Z;
							offset += 4;
							*( float* ) &ModelBytes[ offset ] = mdlfix.Radius;
						}
						offset = newoffset + 4;
					}
					else
					{
						offset += 4;
						offset += *( int* ) &ModelBytes[ offset ];
					}
					offset += 4;
				}
			}
			ModelCollisionFixList.erase( ModelCollisionFixList.begin( ) + ( int ) i );
			i--;
		}

	}


	for ( unsigned int i = 0; i < ModelTextureFixList.size( ); i++ )
	{
		ModelTextureFixStruct mdlfix = ModelTextureFixList[ i ];
		int TextureID = 0;
		if ( _stricmp( filename, mdlfix.FilePath ) == 0 )
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
						int TagSize = *( int* ) &ModelBytes[ offset ];
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
								if ( strlen( mdlfix.NewTexturePath ) > 3 )
								{
									tmpTexture.ReplaceableId = 0;
									sprintf_s( tmpTexture.FileName, 260, "%s", mdlfix.NewTexturePath );
								}
								else
								{
									tmpTexture.ReplaceableId = atoi( mdlfix.NewTexturePath );
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
						offset += *( int* ) &ModelBytes[ offset ];
					}
					offset += 4;
				}
			}

			ModelTextureFixList.erase( ModelTextureFixList.begin( ) + ( int ) i );
			i--;
		}

	}


}


__declspec( dllexport ) int __stdcall RedirectFile( const char * RealFilePath, const char * NewFilePath )
{
	FileRedirectStruct tmpModelFix;
	sprintf_s( tmpModelFix.NewFilePath, 512, "%s", NewFilePath );
	sprintf_s( tmpModelFix.RealFilePath, 512, "%s", RealFilePath );
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

BOOL ProcessFile( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown, BOOL IsFileExistOld )
{
	BOOL IsFileExist = IsFileExistOld;

	AddNewLineToDotaHelperLog( "ProcessFile" );
	if ( strlen( filename ) > 4 )
	{
		string FilePathLower = ToLower( filename );
		string FilePathOrigin( filename );
		size_t PathLen = FilePathLower.length( );
		AddNewLineToDotaHelperLog( "ProcessFileStart" );
		if ( PathLen > 4 )
		{
			if ( strcmp( FilePathLower.c_str( ) + ( PathLen - 4 ), ".tga" ) == 0 )
			{

			}
			else if ( strcmp( FilePathLower.c_str( ) + ( PathLen - 4 ), ".blp" ) == 0 )
			{
				if ( !IsFileExist )
				{
					IsFileExist = FixDisabledIconPath( filename, OutDataPointer, OutSize, unknown );
				}
				else
				{
					/*	if ( strstr( FilePathLower.c_str( ), "terrainart" ) == FilePathLower.c_str( ) ||
							 strstr( FilePathLower.c_str( ), "replaceabletextures\\cliff" ) == FilePathLower.c_str( ) )
							ApplyTerrainFilter( filename, OutDataPointer, OutSize, FALSE );*/
				}
			}
			else if ( strcmp( FilePathLower.c_str( ) + ( PathLen - 4 ), ".mdx" ) == 0 )
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
		}
	}
	else 
		AddNewLineToDotaHelperLog( "BADFILENAMEFOUND" );
	
		return IsFileExist;
}


BOOL __fastcall GameGetFile_my( const char * filename_, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{

	const char * filename = filename_;

	if ( filename && *filename != '\0' )
		AddNewLineToDotaHelperLog( "FileHelper:" + string( filename_ ) );
	else 
		AddNewLineToDotaHelperLog( "FileHelper(BADFILENAME)" );

	BOOL IsFileExist = GameGetFile_ptr( filename, OutDataPointer, OutSize, unknown );

	if ( !*InGame && !MainFuncWork )
		return IsFileExist;


	if ( !IsFileExist )
	{
		AddNewLineToDotaHelperLog( "NoFileFound" );
	}
	else
	{
		AddNewLineToDotaHelperLog( "FileFound" );
	}

	if ( filename && *filename != '\0' )
	{
		IsFileExist = ProcessFile( filename, OutDataPointer, OutSize, unknown, IsFileExist );
	}

	AddNewLineToDotaHelperLog( "ProcessFileENDING" );

	if ( !IsFileExist )
	{
		AddNewLineToDotaHelperLog( "NoFileFound" );
	}
	else
	{
		AddNewLineToDotaHelperLog( "FileFound" );
	}
	/*if ( IsFileExist == 1 && filename && *filename != '\0' )
	{
		if ( strstr( filename, "TerrainArt" ) || strstr( filename, "Cliff" ) )
			ApplyTerrainFilter( filename, OutDataPointer, OutSize );
		if ( strstr( ToLower( filename ).c_str( ), "units" ) )
		{
			//	MessageBox( 0, filename, filename, 0 );
				//ApplyUnitFilter( filename, OutDataPointer, OutSize );
		}
	}
	else if ( IsFileExist != 1 && filename && *filename != '\0' )
	{
		IsFileExist = FixDisabledIconPath( filename, OutDataPointer, OutSize, unknown );
	}*/

	if ( !IsFileExist )
	{
		if ( filename && *filename != '\0' )
		{
			for ( FileRedirectStruct DotaRedirectHelp : FileRedirectList )
			{
				if ( _stricmp( filename, DotaRedirectHelp.NewFilePath ) == 0 )
				{
					ICONMDLCACHE * tmpih = new ICONMDLCACHE( );
					BOOL FoundOldHelper = GetFromIconMdlCache( DotaRedirectHelp.NewFilePath, tmpih );

					if ( !FoundOldHelper )
					{
						IsFileExist = GameGetFile_ptr( DotaRedirectHelp.RealFilePath, OutDataPointer, OutSize, unknown );
						if ( IsFileExist )
						{
							char * DataPointer = ( char * ) *OutDataPointer;
							size_t DataSize = *OutSize;

							Buffer ResultBuffer;
							ResultBuffer.buf = new char[ DataSize ];
							ResultBuffer.length = DataSize;

							memcpy( &ResultBuffer.buf[ 0 ], DataPointer, DataSize );

							tmpih->buf = ResultBuffer.buf;
							tmpih->size = ResultBuffer.length;

							tmpih->hashlen = strlen( DotaRedirectHelp.NewFilePath );
							tmpih->hash = GetBufHash( DotaRedirectHelp.NewFilePath, tmpih->hashlen );

							ICONMDLCACHELIST.push_back( *tmpih );

							*OutDataPointer = ( int ) tmpih->buf;
							*OutSize = tmpih->size;

							ProcessFile( DotaRedirectHelp.NewFilePath, OutDataPointer, OutSize, unknown, IsFileExist );
							return IsFileExist;
						}
					}
					else
					{

						*OutDataPointer = ( int ) tmpih->buf;
						*OutSize = tmpih->size;

						ProcessFile( DotaRedirectHelp.NewFilePath, OutDataPointer, OutSize, unknown, IsFileExist );
						return TRUE;
					}

					delete[ ] tmpih;
				}
			}

		}
	}

	if ( !IsFileExist )
	{

		/*if ( filename && *filename != '\0' )
		{
			MessageBox( 0, filename, "File not found", 0 );
		}*/
	}

	AddNewLineToDotaHelperLog( "ProcessFileEND" );

	return IsFileExist;
}

