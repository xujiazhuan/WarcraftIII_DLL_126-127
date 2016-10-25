
#include "Main.h"
extern "C" { FILE __iob_func[ 3 ] = { *stdin,*stdout,*stderr }; }
#pragma comment(lib,"legacy_stdio_definitions.lib")
#include "blpaletter.h"


#define MASK_56 (((u_int64_t)1<<56)-1) /* i.e., (u_int64_t)0xffffffffffffff */

#include "fnv.h"

u_int64_t GetBufHash( char * data, size_t data_len )
{
	u_int64_t hash;
	hash = fnv_64_buf( data, ( size_t ) data_len, FNV1_64_INIT );
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


BOOL GetFromIconMdlCache( char * filename, ICONMDLCACHE * iconhelperout )
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

void ReplaceIconPathIfNeed( )
{
	AddNewLineToDotaHelperLog( "ReplaceIconPathIfNeed" );
	char * tmpstr = 0;
	if ( strstr( MPQFilePath, DisabledIconSignature2 ) )
	{
		tmpstr = repl_string( MPQFilePath, DisabledIconSignature2, "Disabled\\DIS" );
		SMemZero( MPQFilePath, 4000 );
		sprintf_s( MPQFilePath, 4000, "%s", tmpstr );
		free( tmpstr );
	}
	else if ( strstr( MPQFilePath, DisabledIconSignature ) )
	{
		tmpstr = repl_string( MPQFilePath, DisabledIconSignature, "\\" );
		SMemZero( MPQFilePath, 4000 );
		sprintf_s( MPQFilePath, 4000, "%s", tmpstr );
		free( tmpstr );
	}
}

GameGetFile GameGetFile_org;
GameGetFile GameGetFile_ptr;

int idddd = 0;

void ApplyTerrainFilter( char * filename, int * OutDataPointer, size_t * OutSize, BOOL IsTga )
{
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
			if ( !IsFileRedirected( filename ) && !IsMemInCache( *OutDataPointer ) )
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
			if ( !IsFileRedirected( filename ) && !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* ) *OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int ) tmpih.buf;
			*OutSize = tmpih.size;
		}
	}

}




void ApplyIconFilter( char * filename, int * OutDataPointer, size_t * OutSize )
{
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


		for ( int i = 0; i < h; i++ )
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

		}

		/*for ( unsigned long i = 0; i < OutBuffer.length / 4; i++ )
		{
			if ( OutImage[ i ].A == 0xFF && ( OutImage[ i ].G > 20 || OutImage[ i ].B > 20 || OutImage[ i ].R > 20 ) )
			{
				OutImage[ i ].R /= 2;
				OutImage[ i ].G /= 2;
				OutImage[ i ].B /= 2;
			}
			else if ( OutImage[ i ].A == 0xFF )
			{
				OutImage[ i ].R = 0;
				OutImage[ i ].G = 0;
				OutImage[ i ].B = 0;
			}
		}
		*/


		for ( unsigned long i = 0; i < OutBuffer.length / 4; i++ )
		{
			BGRAPix CurPix = OutImage[ i ];


			int ave = (( min( min( CurPix.R, CurPix.G ), CurPix.B ) + max( max( CurPix.R, CurPix.G ), CurPix.B ) ) / 2);
			CurPix.R = ( unsigned char ) ( ( ave + CurPix.R ) / 4 );
			CurPix.G = ( unsigned char ) ( ( ave + CurPix.G ) / 4 );
			CurPix.B = ( unsigned char ) ( ( ave + CurPix.B ) / 4 );
			OutImage[ i ] = CurPix;
			/*if ( OutImage[ i ].A == 0xFF && ( OutImage[ i ].G > 20 || OutImage[ i ].B > 20 || OutImage[ i ].R > 20 ) )
			{
				OutImage[ i ].R /= 2;
				OutImage[ i ].G /= 2;
				OutImage[ i ].B /= 2;
			}
			else if ( OutImage[ i ].A == 0xFF )
			{
				OutImage[ i ].R = 0;
				OutImage[ i ].G = 0;
				OutImage[ i ].B = 0;
			}*/
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
			tmpih.buf = ResultBuffer.buf;
			tmpih.size = ResultBuffer.length;
			tmpih.hashlen = strlen( filename );
			tmpih.hash = GetBufHash( filename, tmpih.hashlen );
			ICONMDLCACHELIST.push_back( tmpih );
			if ( !IsFileRedirected( filename ) && !IsMemInCache( *OutDataPointer ) )
				Storm_403_org( ( void* ) *OutDataPointer, "delete", -1, 0 );
			*OutDataPointer = ( int ) tmpih.buf;
			*OutSize = tmpih.size;
		}
	}

}

BOOL FixDisabledIconPath( char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{
	if ( !strstr( filename, "blp" ) )
		return 0;


	BOOL CreateDarkIcon = FALSE;
	BOOL result = FALSE;
	if ( strstr( filename, DisabledIconSignature2 ) )
	{
		char * tmpstr = repl_string( filename, DisabledIconSignature2, "Disabled\\DIS" );
		result = GameGetFile_ptr( tmpstr, OutDataPointer, OutSize, unknown );
		free( tmpstr );
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
			CreateDarkIcon = TRUE;
		}

	}

	if ( !result )
	{
		char * tmpstr = repl_string( filename, CommandButtonsDisabledIconSignature, "PassiveButtons\\" );
		result = GameGetFile_ptr( tmpstr, OutDataPointer, OutSize, unknown );
		free( tmpstr );
		CreateDarkIcon = TRUE;
	}


	if ( result && CreateDarkIcon )
	{
		ApplyIconFilter( filename, OutDataPointer, OutSize );
	}
	//else MessageBox( 0, filename, "Bad file path:", 0 );

	return result;
}




vector<ModelCollisionFixStruct> ModelCollisionFixList;
vector<ModelTextureFixStruct> ModelTextureFixList;
vector<ModelPatchStruct> ModelPatchList;

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
	sprintf_s( tmpModelFix.NewTexturePath, 0x100, "%s", texturenew );
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




vector<BYTE> FullPatchData;


void ProcessMdx( char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{
	char * ModelBytes = ( char* ) *OutDataPointer;
	size_t sz = *OutSize;

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

			ModelBytes = tmpih->buf;
			sz = tmpih->size;

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
				while ( offset < sz )
				{
					offset += 4;
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
				while ( offset < sz )
				{
					offset += 4;
					if ( memcmp( &ModelBytes[ offset ], "TEXS", 4 ) == 0 )
					{
						//char TextNameBuf[ 0x100 ];
						offset += 4;
						size_t newoffset = offset + *( int* ) &ModelBytes[ offset ];

						while ( offset < newoffset )
						{
							TextureID++;
							offset += 4;
							int * replaceableid = ( int* ) &ModelBytes[ offset ];
							offset += 4;
							//	memcpy( TextNameBuf, &ModelBytes[ offset ], 0x100 );
							if ( mdlfix.TextureID == TextureID )
							{
								memset( &ModelBytes[ offset ], 0, 0x100 );
								*replaceableid = 0;
								sprintf_s( &ModelBytes[ offset ], 0x100, "%s", mdlfix.NewTexturePath );
							}
							offset += 0x100;
							//	int unknownvalue = *( int* ) &ModelBytes[ offset ];
							offset += 4;
							//	int wraptype = *( int* ) &ModelBytes[ offset ];
						}
						offset = newoffset + 4;
					}
					else
					{
						offset += 4;
						offset += *( int* ) &ModelBytes[ offset ];
					}
				}
			}

			ModelTextureFixList.erase( ModelTextureFixList.begin( ) + ( int ) i );
			i--;
		}

	}

	if ( IsKeyPressed( '0' ) && FileExist( ".\\Test1234.mdx" ) )
	{
		FILE *f;
		fopen_s( &f, ".\\Test1234.mdx", "wb" );
		fwrite( ModelBytes, 1, sz, f );
		fclose( f );
	}
}


__declspec( dllexport ) int __stdcall RedirectFile( const char * NewFilePath, const char * RealFilePath )
{
	FileRedirectStruct tmpModelFix;
	sprintf_s( tmpModelFix.NewFilePath, 512, "%s", NewFilePath );
	sprintf_s( tmpModelFix.RealFilePath, 512, "%s", RealFilePath );
	FileRedirectList.push_back( tmpModelFix );
	return 0;
}


BOOL ProcessFile( char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown, BOOL IsFileExistOld )
{
	BOOL IsFileExist = IsFileExistOld;

	string FilePathLower = ToLower( filename );
	string FilePathOrigin( filename );
	size_t PathLen = FilePathLower.length( );

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
				if ( strstr( FilePathLower.c_str( ), "terrainart" ) == FilePathLower.c_str( ) ||
					  strstr( FilePathLower.c_str( ), "replaceabletextures\\cliff" ) == FilePathLower.c_str( ) ) 
				ApplyTerrainFilter( filename, OutDataPointer, OutSize, FALSE );
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
	return IsFileExist;
}


BOOL __fastcall GameGetFile_my( char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown )
{
	BOOL IsFileExist = GameGetFile_ptr( filename, OutDataPointer, OutSize, unknown );

	if ( !*InGame && !MainFuncWork )
		return IsFileExist;

	if ( filename && *filename != '\0' )
	{
		IsFileExist = ProcessFile( filename, OutDataPointer, OutSize, unknown, IsFileExist );
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
					IsFileExist = GameGetFile_ptr( DotaRedirectHelp.RealFilePath, OutDataPointer, OutSize, unknown );
					if ( IsFileExist )
					{
						ProcessFile( filename, OutDataPointer, OutSize, unknown, IsFileExist );
					}
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

	return IsFileExist;
}

