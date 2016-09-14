#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")


#include "Warcraft.h"

#include <stdint.h>
#include "Crc32Dynamic.h"

bool FileExist( const std::string& name )
{
	ifstream f( name.c_str( ) );
	return f.good( );
}


#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0

HMODULE GetCurrentModule;
#pragma region Game.dll JassNatives
// Проверка являются ли игроки врагами
typedef int( __cdecl * IsPlayerEnemy )( UINT Player1, UINT Player2 ); /*Game+3C9580*/
int IsPlayerEnemyOffset = 0x3C9580;

// Получить хэндл игрока по его номеру слота
typedef UINT( __cdecl * GetPlayerByID )( int PlayerId ); /*Game+3BBB30*/
int GetPlayerByIDOffset = 0x3BBB30;

#pragma endregion


typedef signed int( __fastcall * IsDrawSkillPanel )( void *UnitAddr, int addr1 ); /*Game+34F280*/
IsDrawSkillPanel IsDrawSkillPanel_org;
IsDrawSkillPanel IsDrawSkillPanel_ptr;
int IsDrawSkillPanelOffset = 0x34F280;

typedef signed int( __thiscall * DrawSkillPanel )( void *UnitAddr, int OID ); /*Game+2774C0*/
int DrawSkillPanelOffset = 0x2774C0;



typedef signed int( __fastcall * IsDrawSkillPanelOverlay )( void *UnitAddr, int addr1 ); /*Game+34F2C0*/
IsDrawSkillPanelOverlay IsDrawSkillPanelOverlay_org;
IsDrawSkillPanelOverlay IsDrawSkillPanelOverlay_ptr;
int IsDrawSkillPanelOverlayOffset = 0x34F2C0;

typedef signed int( __thiscall * DrawSkillPanelOverlay )( void *UnitAddr, int OID ); /*Game+277570*/
int DrawSkillPanelOverlayOffset = 0x277570;



typedef int( __thiscall * IsNeedDrawUnitOrigin )( void *UnitAddr );/*Game+285DC0*/
int IsNeedDrawUnitOriginOffset = 0x285DC0;

// Адрес Game.dll
int GameDll = 0;
HMODULE GameDllModule = 0;
int GameVersion = 0;

int GlobalPlayerOffset = 0xAB65F4;

void * GetGlobalPlayerData( )
{
	return ( void * ) *( int * ) ( GlobalPlayerOffset + GameDll );
}


int pW3XGlobalClass = 0;
int pPrintText2 = 0;


void DisplayText( char *szText, float fDuration )
{
	DWORD dwDuration = *( ( DWORD * ) &fDuration );
	__asm
	{
		PUSH 0xFFFFFFFF;
		PUSH fDuration;
		PUSH szText;
		MOV		ECX, [ pW3XGlobalClass ];
		MOV		ECX, [ ECX ];
		MOV		EAX, pPrintText2;
		CALL	EAX;
	}
}


// Получить слот игрока
int GetLocalPlayerId( )
{
	void * gldata = GetGlobalPlayerData( );

	int playerslotaddr = ( int ) gldata + 0x28;

	return ( int ) *( short * ) ( playerslotaddr );
}


// Получить владельца юнита
UINT GetUnitOwnerSlot( int unitaddr )
{
	return *( UINT* ) ( unitaddr + 88 );
}


// Получить имя игрока по его слоту
typedef char *( __fastcall * p_GetPlayerName )( int a1, int a2 );
p_GetPlayerName GetPlayerName = NULL;


// Является ли юнит героем
BOOL IsHero( int unitaddr )
{
	UINT ishero = *( UINT* ) ( unitaddr + 48 );
	ishero = ishero >> 24;
	ishero = ishero - 64;
	return ishero < 0x19;
}


// Является ли юнит зданием
BOOL IsTower( int unitaddr )
{
	UINT istower = *( UINT* ) ( unitaddr + 0x5C );
	return ( istower & 0x10000 );
}


int UnitVtable = 0;


// Проверяет юнит или не юнит
BOOL IsNotBadUnit( int unitaddr )
{
	if ( unitaddr > 0 )
	{
		int xaddraddr = ( int ) &UnitVtable;

		if ( *( BYTE* ) xaddraddr != *( BYTE* ) unitaddr )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 1 ) != *( BYTE* ) ( unitaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 2 ) != *( BYTE* ) ( unitaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 3 ) != *( BYTE* ) ( unitaddr + 3 ) )
			return FALSE;

		unsigned int unitflag = *( unsigned int* ) ( unitaddr + 0x5C );

		BOOL returnvalue = unitflag > 1 && unitflag != 0x1001u /*&& ( unitflag & 0x40000000u ) == 0*/;

		return returnvalue;
	}

	return FALSE;
}

int ItemVtable = 0;


// Проверяет предмет или не предмет
BOOL IsNotBadItem( int itemaddr )
{
	if ( itemaddr > 0 )
	{
		int xaddraddr = ( int ) &ItemVtable;

		if ( *( BYTE* ) xaddraddr != *( BYTE* ) itemaddr )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 1 ) != *( BYTE* ) ( itemaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 2 ) != *( BYTE* ) ( itemaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 3 ) != *( BYTE* ) ( itemaddr + 3 ) )
			return FALSE;

		return TRUE;
	}

	return FALSE;
}



// Проверяет враг юнит локальному игроку или нет
BOOL __stdcall IsEnemy( int UnitAddr )
{
	if ( !UnitAddr )
		return TRUE;

	int unitownerslot = GetUnitOwnerSlot( ( int ) UnitAddr );
	if ( unitownerslot <= 15 )
	{
		UINT Player1 = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( unitownerslot );
		UINT Player2 = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetLocalPlayerId( ) );
		return ( ( ( IsPlayerEnemy ) ( GameDll + IsPlayerEnemyOffset ) )( Player1, Player2 ) );
	}
	return 1;
}

signed int __fastcall  IsDrawSkillPanel_my( void *UnitAddr, int addr1 )
{
	signed int result; // eax@2
	int GETOID; // eax@3
	int OID; // esi@4

	if ( addr1 )
	{
		GETOID = *( int * ) ( addr1 + 444 );
		if ( GETOID <= 0 )
			OID = 852290;
		else
			OID = *( int * ) ( GETOID + 8 );

		UINT Player1_UnitOwner = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetUnitOwnerSlot( ( int ) UnitAddr ) );
		UINT Player2_LocalPlayer = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetLocalPlayerId( ) );

		// Сначала вызвать оригинальную функцию
		if ( ( ( IsNeedDrawUnitOrigin ) ( GameDll + IsNeedDrawUnitOriginOffset ) )( UnitAddr ) )
		{
			( ( DrawSkillPanel ) ( GameDll + DrawSkillPanelOffset ) )( UnitAddr, OID );
		}
		// Затем дополнительную которая отрисует скилы всем союзным героям.
		else if ( !( ( ( IsPlayerEnemy ) ( GameDll + IsPlayerEnemyOffset ) )( Player1_UnitOwner, Player2_LocalPlayer ) ) )
		{
			if ( IsHero( ( int ) UnitAddr ) )
				( ( DrawSkillPanel ) ( GameDll + DrawSkillPanelOffset ) )( UnitAddr, OID );
		}
		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
}


signed int __fastcall  IsDrawSkillPanelOverlay_my( void *UnitAddr, int addr1 )
{
	signed int result; // eax@2
	int GETOID; // eax@3
	int OID; // esi@4

	if ( addr1 )
	{
		GETOID = *( int * ) ( addr1 + 444 );
		if ( GETOID <= 0 )
			OID = 852290;
		else
			OID = *( int * ) ( GETOID + 8 );

		UINT Player1_UnitOwner = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetUnitOwnerSlot( ( int ) UnitAddr ) );
		UINT Player2_LocalPlayer = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetLocalPlayerId( ) );

		// Сначала вызвать оригинальную функцию
		if ( ( ( IsNeedDrawUnitOrigin ) ( GameDll + IsNeedDrawUnitOriginOffset ) )( UnitAddr ) )
		{
			( ( DrawSkillPanelOverlay ) ( GameDll + DrawSkillPanelOverlayOffset ) )( UnitAddr, OID );
		}
		// Затем дополнительную которая отрисует скилы всем союзным героям.
		else if ( !( ( ( IsPlayerEnemy ) ( GameDll + IsPlayerEnemyOffset ) )( Player1_UnitOwner, Player2_LocalPlayer ) ) )
		{
			if ( IsHero( ( int ) UnitAddr ) )
				( ( DrawSkillPanelOverlay ) ( GameDll + DrawSkillPanelOverlayOffset ) )( UnitAddr, OID );
		}
		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
}


typedef int( __thiscall * IsNeedDrawUnit2 )( int UnitAddr ); /* Game + 28E1D0*/
IsNeedDrawUnit2 IsNeedDrawUnit2org;
IsNeedDrawUnit2 IsNeedDrawUnit2ptr;
int IsNeedDrawUnit2offset = 0x28E1D0;

int IsNeedDrawUnit2offsetRetAddress = 0x2F9BB4;


int __fastcall IsNeedDrawUnit2_my( int UnitAddr, int unused/* converted from thiscall to fastcall*/ )
{
	int retaddr = ( int ) _ReturnAddress( ) - GameDll;


	if ( retaddr + 2000 < IsNeedDrawUnit2offsetRetAddress && retaddr > IsNeedDrawUnit2offsetRetAddress )
	{

		UINT Player1_UnitOwner = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetUnitOwnerSlot( ( int ) UnitAddr ) );
		UINT Player2_LocalPlayer = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetLocalPlayerId( ) );
		if ( !( ( ( IsPlayerEnemy ) ( GameDll + IsPlayerEnemyOffset ) )( Player1_UnitOwner, Player2_LocalPlayer ) ) )
		{
			if ( IsHero( ( int ) UnitAddr ) )
				return 1;
		}
	}

	return IsNeedDrawUnit2ptr( UnitAddr );
}


HMODULE StormDll;


typedef int( __stdcall * pStorm_279 )( const char*  a1, int a2, int a3, size_t Size, int a5 );
pStorm_279 Storm_279_org;
pStorm_279 Storm_279_ptr;


char MPQFilePath[ 2048 ];

const char * DisabledIconSignature = "Disabled\\DIS";

// Функция замены текста в строке.
char *repl_string( const char *str, const char *from, const char *to )
{
	size_t cache_sz_inc = 16;

	const size_t cache_sz_inc_factor = 3;

	const size_t cache_sz_inc_max = 1048576;

	char *pret, *ret = NULL;
	const char *pstr2, *pstr = str;
	size_t i, count = 0;
	ptrdiff_t *pos_cache = NULL;
	size_t cache_sz = 0;
	size_t cpylen, orglen, retlen, tolen, fromlen = strlen( from );

	while ( ( pstr2 = strstr( pstr, from ) ) != NULL )
	{
		count++;
		if ( cache_sz < count )
		{
			cache_sz += cache_sz_inc;
			pos_cache = ( ptrdiff_t * ) realloc( pos_cache, sizeof( *pos_cache ) * cache_sz );
			if ( pos_cache == NULL )
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
		memcpy( pret, str, pos_cache[ 0 ] );
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


// просто для тестов не обращать внимания.
void ReplaceIconPathIfNeed( )
{
	/*char * tmpstr = 0;
	if ( strstr( MPQFilePath, "Disabled\\DIS" ) )
	{

	tmpstr = repl_string( MPQFilePath, "Disabled\\DIS", "\\" );
	//MessageBox( 0, MPQFilePath, tmpstr, 0 );
	sprintf_s( MPQFilePath, 2048, "%s\0", tmpstr );
	free( tmpstr );
	}*/

	/*if ( strstr( MPQFilePath, ".blp" ) )
	{
	sprintf_s( MPQFilePath, 2048, "%s\0", "Textures\\BloodElfBallz.blp" );
	}*/
	/*else if ( strstr( MPQFilePath, ".mdx" ) )
	{
	sprintf_s( MPQFilePath, 2048, "%s\0", "Units\\Undead\\Skeleton\\Skeleton.mdx" );
	}*/
}

// Функция открытия файла и получения его размера.
int __stdcall Storm_279my( const char* a1, int a2, int a3, size_t Size, int a5 )
{
	sprintf_s( MPQFilePath, 2048, "%s\0", a1 );
	//ReplaceIconPathIfNeed( );
	return Storm_279_ptr( MPQFilePath, a2, a3, Size, a5 );
}





char szHeaders[ ] = "Content-Type: application/x-www-form-urlencoded\r\nCache-Control: no-cache, no-store, must-revalidate\r\nPragma: no-cache\r\nExpires: 0\r\n";


int DownStatus = 0;
int DownProgress = 0;
std::string LatestDownloadedString;

std::string DownloadBytesGet( char* szUrl, char * getRequest )
{
	DownStatus = 0;
	std::string returnvalue = "";
	WSADATA wsaData;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
	{
		DownStatus = -1;

		return returnvalue;
	}
	SOCKET Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( !Socket )
	{
		WSACleanup( );
		DownStatus = -1;

		return returnvalue;
	}


	struct hostent *host;
	host = gethostbyname( szUrl );

	if ( !host )
	{
		closesocket( Socket );
		WSACleanup( );
		DownStatus = -1;

		return returnvalue;
	}

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons( 80 );
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *( ( unsigned long* ) host->h_addr );
	if ( connect( Socket, ( SOCKADDR* ) ( &SockAddr ), sizeof( SockAddr ) ) != 0 )
	{
		closesocket( Socket );
		WSACleanup( );
		DownStatus = -1;

		return returnvalue;
	}

	char sendbuffer[ 512 ];
	sprintf_s( sendbuffer, 512, "%s%s%s%s\r\nConnection: close\r\n\r\n", "GET ", getRequest, " HTTP/1.1\r\nHost: ", szUrl );

	DownProgress = 20;

	if ( send( Socket, sendbuffer, strlen( sendbuffer ), 0 ) == SOCKET_ERROR )
	{
		closesocket( Socket );
		WSACleanup( );
		DownStatus = -1;

		return returnvalue;
	}

	DownProgress = 40;

	char buffer[ 10000 ];
	int nDataLength;
	while ( ( nDataLength = recv( Socket, buffer, 10000, 0 ) ) > 0 )
	{
		int i = 0;
		DownProgress = 60;
		returnvalue.append( buffer, nDataLength );
	}

	DownProgress = 80;

	while ( returnvalue.size( ) )
	{
		if ( returnvalue.size( ) > 0 )
		{
			if ( returnvalue.c_str( )[ 0 ] != '\r' )
			{
				returnvalue.erase( returnvalue.begin( ) );
			}
			else if ( returnvalue.size( ) > 5 )
			{

				if ( returnvalue.c_str( )[ 1 ] == '\n'
					 && returnvalue.c_str( )[ 2 ] == '\r'
					 && returnvalue.c_str( )[ 3 ] == '\n' )
				{
					returnvalue.erase( returnvalue.begin( ) );
					returnvalue.erase( returnvalue.begin( ) );
					returnvalue.erase( returnvalue.begin( ) );
					returnvalue.erase( returnvalue.begin( ) );
					break;
				}
				returnvalue.erase( returnvalue.begin( ) );
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	DownProgress = 100;

	LatestDownloadedString = returnvalue;
	if ( returnvalue.size( ) > 0 )
		DownStatus = 1;
	else
		DownStatus = -1;
	return returnvalue;
}

char CurrentMapPath[ MAX_PATH ];
char NewMapPath[ MAX_PATH ];

int MapNameOffset1 = 0;
int MapNameOffset2 = 0;

void SaveCurrentMapPath( )
{
	memset( CurrentMapPath, 0, MAX_PATH );
	int offset1 = *( int* ) MapNameOffset1;
	if ( offset1 > 0 )
	{
		const char * mPath = ( const char * ) ( offset1 + MapNameOffset2 );
		sprintf_s( CurrentMapPath, MAX_PATH, "%s", mPath );
	}
}


void BuildFilePath( char * fname )
{
	SaveCurrentMapPath( );


	unsigned int MapPathSize = strlen( CurrentMapPath );
	if ( MapPathSize > 8 )// "Maps\a.w3x"
	{
		for ( unsigned int i = MapPathSize - 1; i > 0; i-- )
		{
			if ( CurrentMapPath[ i ] != '\\' )
			{
				CurrentMapPath[ i ] = '\0';
			}
			else
				break;
		}

		MapPathSize = strlen( CurrentMapPath );
		if ( MapPathSize > 4 ) // "Maps\"
		{

			memset( NewMapPath, 0, MAX_PATH );
			if ( fname != NULL && *fname != 0 )
				sprintf_s( NewMapPath, MAX_PATH, "%s%s", CurrentMapPath, fname );
			else
				sprintf_s( NewMapPath, MAX_PATH, "%s", CurrentMapPath );

		}
	}
}


void DownloadNewMapToFile( char* szUrl, char * filepath )
{
	DownStatus = 0;
	HINTERNET hOpen = NULL;
	HINTERNET hFile = NULL;
	DWORD dataSize = 0;
	DWORD dwBytesRead = 0;
	vector<unsigned char> OutData;
	FILE * outfile = NULL;
	BOOL AllOkay = FALSE;

	if ( filepath == NULL || filepath[ 0 ] == '\0' || FileExist( filepath ) )
	{
		DownStatus = 2;
		return;
	}

	hOpen = InternetOpen( "Microsoft Internet Explorer", NULL, NULL, NULL, 0 );
	if ( !hOpen )
	{
		DownStatus = -1;
		return;
	}
	DownProgress = 10;
	hFile = InternetOpenUrl( hOpen, szUrl, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, 0 );

	if ( !hFile )
	{
		InternetCloseHandle( hOpen );
		DownStatus = -1;
		return;
	}
	DownProgress = 20;
	int code = 0;
	DWORD codeLen = 4;
	HttpQueryInfo( hFile, HTTP_QUERY_STATUS_CODE |
				   HTTP_QUERY_FLAG_NUMBER, &code, &codeLen, 0 );

	if ( code != HTTP_STATUS_OK )// 200 OK
	{
		InternetCloseHandle( hFile );
		InternetCloseHandle( hOpen );
		DownStatus = -1;
		return;
	}

	DWORD sizeBuffer;
	DWORD length = sizeof( sizeBuffer );
	HttpQueryInfo( hFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &sizeBuffer, &length, NULL );



	DownProgress = 30;

	do
	{
		dataSize += dwBytesRead;
		if ( sizeBuffer != 0 )
			DownProgress = ( dataSize * 100 ) / sizeBuffer;

		dwBytesRead = 0;
		unsigned char buffer[ 2000 ];
		BOOL isRead = InternetReadFile( hFile, ( LPVOID ) buffer, _countof( buffer ), &dwBytesRead );
		if ( dwBytesRead && isRead )
		{
			AllOkay = TRUE;
			for ( unsigned int i = 0; i < dwBytesRead; i++ )
				OutData.push_back( buffer[ i ] );
		}
		else
			break;
	}
	while ( dwBytesRead );

	if ( DownProgress == 30 )
	{
		DownProgress = 70;
	}

	if ( OutData.size( ) > 0 && AllOkay )
	{
		fopen_s( &outfile, filepath, "wb" );
		if ( outfile != NULL )
		{
			fwrite( &OutData[ 0 ], OutData.size( ), 1, outfile );
			OutData.clear( );
			fclose( outfile );
			DownStatus = 1;
		}
		else DownStatus = -1;
	}
	else DownStatus = -1;


	if ( DownProgress == 70 )
	{
		DownProgress = 100;
	}

	InternetCloseHandle( hFile );
	InternetCloseHandle( hOpen );

	return;
}

char * _addr = 0;
char * _request = 0;
char * _filepath = 0;

DWORD WINAPI SENDGETREQUEST( LPVOID )
{
	DownloadBytesGet( _addr, _request ).clear( );
	return 0;
}


DWORD WINAPI SENDSAVEFILEREQUEST( LPVOID )
{
	DownloadNewMapToFile( _addr, _filepath );
	return 0;
}

__declspec( dllexport ) int __cdecl SendGetRequest( char * addr, char * request )
{
	DownProgress = 0;
	_addr = addr; _request = request;
	DownStatus = 0;
	CreateThread( 0, 0, SENDGETREQUEST, 0, 0, 0 );
	return 0;
}

__declspec( dllexport ) int __cdecl SaveNewDotaVersionFromUrl( char * addr, char * filepath )
{
	DownProgress = 0;
	_addr = addr; _filepath = filepath;
	DownStatus = 0;
	CreateThread( 0, 0, SENDSAVEFILEREQUEST, 0, 0, 0 );
	return 0;
}

__declspec( dllexport ) int __cdecl GetDownloadStatus( int )
{
	return DownStatus;
}

__declspec( dllexport ) int __cdecl GetDownloadProgress( int )
{
	return DownProgress;
}

__declspec( dllexport ) const char * __cdecl GetLatestDownloadedString( int )
{
	return LatestDownloadedString.c_str( );
}


__declspec( dllexport ) const char * __cdecl GetCurrentMapPath( int )
{
	BuildFilePath( NULL );
	return CurrentMapPath;
}


int pOnChatMessage_offset;
vector<char *> mutedplayers;
//sub_6F2FB480
typedef void( __fastcall * pOnChatMessage )( int a1, int unused, int PlayerID, char * message, int a4, float a5 );
pOnChatMessage pOnChatMessage_org;
pOnChatMessage pOnChatMessage_ptr;
void __fastcall pOnChatMessage_my( int a1, int unused, int PlayerID, char * message, int a4, float a5 )
{
	char * playername = GetPlayerName( PlayerID, 1 );

	for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
	{
		if ( _stricmp( playername, mutedplayers[ i ] ) == 0 )
		{
			return;
		}
	}

	pOnChatMessage_ptr( a1, unused, PlayerID, message, a4, a5 );
}


__declspec( dllexport ) int __stdcall MutePlayer( const char * str )
{
	if ( !str || *str == 0 )
		return 1;
	for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
	{
		if ( _stricmp( str, mutedplayers[ i ] ) == 0 )
		{
			return 1;
		}
	}
	mutedplayers.push_back( _strdup( str ) );
	return 1;
}

__declspec( dllexport ) int __stdcall UnMutePlayer( const char * str )
{
	if ( !str || *str == 0 )
		return 1;
	for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
	{
		if ( _stricmp( str, mutedplayers[ i ] ) == 0 )
		{
			free( mutedplayers[ i ] );
			mutedplayers[ i ] = NULL;
			mutedplayers.erase( mutedplayers.begin( ) + i );
			return 1;
		}
	}
	return 1;
}



struct FloatStruct1
{
	float flt1;//0
	float flt2;//4
	float flt3;//8
	float flt4;//12
	float flt5;//16
	float flt6;//20
	float flt7;//24
	float flt8;//28
	float flt9;//32
	float flt10;//36
	float flt11;//40
	float flt12;//44
	float flt13;//48
	float flt14;//52
	float flt15;//56
	float flt16;//60
};


typedef void( __fastcall * SetGameAreaFOV )( FloatStruct1 * a1, int a2, float a3, float a4, float a5, float a6 );
SetGameAreaFOV SetGameAreaFOV_org;
SetGameAreaFOV SetGameAreaFOV_ptr;


float CustomFovFix = 1.0f;

int GetWindowXoffset = 0;
int GetWindowYoffset = 0;

int __fastcall SetGameAreaFOV_new( FloatStruct1 * a1, int _unused, float a3, float a4, float a5, float a6 )
{

	float ScreenX = *( float* ) ( GameDll + GetWindowXoffset );
	float ScreenY = *( float* ) ( GameDll + GetWindowYoffset );

	float v1 = 1.0f / sqrt( a4 * a4 + 1.0f );
	float v2 = tan( v1 * a3 * 0.5f );

	float v3 = v2 * a5;
	float v4 = v3 * a4;

	a1->flt1 = ( ( a5 * ( 4.0f / 3.0f ) ) / ( ScreenX / ScreenY ) * CustomFovFix ) / v4; // Fix 4:3 to WindowX/WindowY
	a1->flt2 = 0.0f;
	a1->flt3 = 0.0f;
	a1->flt4 = 0.0f;
	a1->flt5 = 0.0f;


	a1->flt6 = a5 / v3;
	a1->flt7 = 0.0f;
	a1->flt8 = 0.0f;
	a1->flt9 = 0.0f;
	a1->flt10 = 0.0f;


	a1->flt11 = ( a5 + a6 ) / ( a6 - a5 );
	a1->flt12 = 1.0f;
	a1->flt13 = 0.0f;
	a1->flt14 = 0.0f;


	a1->flt15 = a5 * ( a6 * -2.0f ) / ( a6 - a5 );
	a1->flt16 = 0.0f;


	return 0;
}


BOOL EnableFixFOV = FALSE;


__declspec( dllexport ) int __stdcall SetWidescreenFixState( BOOL widefixenable )
{
	EnableFixFOV = widefixenable;
	return 0;
}


__declspec( dllexport ) int __stdcall SetCustomFovFix( float _CustomFovFix )
{
	CustomFovFix = _CustomFovFix;
	return 0;
}


void __fastcall SetGameAreaFOV_my( FloatStruct1 * a1, int a2, float a3, float a4, float a5, float a6 )
{
	if ( EnableFixFOV )
	{
		SetGameAreaFOV_new( a1, a2, a3, a4, a5, a6 );
	}
	else
	{
		SetGameAreaFOV_ptr( a1, a2, a3, a4, a5, a6 );
	}
}




int SetGameAreaFOVoffset = 0;




void InitHook( )
{

		SetGameAreaFOV_org = ( SetGameAreaFOV ) ( SetGameAreaFOVoffset + GameDll );

		MH_CreateHook( SetGameAreaFOV_org, &SetGameAreaFOV_my, reinterpret_cast< void** >( &SetGameAreaFOV_ptr ) );

		MH_EnableHook( SetGameAreaFOV_org );

	
	// Установить адрес для IsDrawSkillPanel_org
	IsDrawSkillPanel_org = ( IsDrawSkillPanel ) ( IsDrawSkillPanelOffset + GameDll );
	// Создать хук (перехват) для IsDrawSkillPanel_org и сохранить его в памяти
	MH_CreateHook( IsDrawSkillPanel_org, &IsDrawSkillPanel_my, reinterpret_cast< void** >( &IsDrawSkillPanel_ptr ) );
	// Активировать хук для IsDrawSkillPanel_org
	MH_EnableHook( IsDrawSkillPanel_org );


	// Установить адрес для IsDrawSkillPanelOverlay_org
	IsDrawSkillPanelOverlay_org = ( IsDrawSkillPanelOverlay ) ( IsDrawSkillPanelOverlayOffset + GameDll );
	// Создать хук (перехват) для IsDrawSkillPanel_org и сохранить его в памяти
	MH_CreateHook( IsDrawSkillPanelOverlay_org, &IsDrawSkillPanelOverlay_my, reinterpret_cast< void** >( &IsDrawSkillPanelOverlay_ptr ) );
	// Активировать хук для IsDrawSkillPanel_org
	MH_EnableHook( IsDrawSkillPanelOverlay_org );


	// Установить адрес для IsNeedDrawUnit2org
	IsNeedDrawUnit2org = ( IsNeedDrawUnit2 ) ( IsNeedDrawUnit2offset + GameDll );
	// Создать хук (перехват) для IsNeedDrawUnit2org и сохранить его в памяти
	MH_CreateHook( IsNeedDrawUnit2org, &IsNeedDrawUnit2_my, reinterpret_cast< void** >( &IsNeedDrawUnit2ptr ) );
	// Активировать хук для IsNeedDrawUnit2org
	MH_EnableHook( IsNeedDrawUnit2org );

	/*	StormDll = GetModuleHandle( "Storm.dll" );

		Storm_279_org = ( pStorm_279 ) ( GetProcAddress( StormDll, ( LPCSTR ) 279 ) );
		MH_CreateHook( Storm_279_org, &Storm_279my, reinterpret_cast< void** >( &Storm_279_ptr ) );
		MH_EnableHook( Storm_279_org );

		*/


	pOnChatMessage_org = ( pOnChatMessage ) ( GameDll + pOnChatMessage_offset );
	MH_CreateHook( pOnChatMessage_org, &pOnChatMessage_my, reinterpret_cast< void** >( &pOnChatMessage_ptr ) );
	MH_EnableHook( pOnChatMessage_org );

}

void UninitializeHook( )
{
	// Отключить хук для IsDrawSkillPanel_org
	if ( IsDrawSkillPanel_org )
	{
		MH_DisableHook( IsDrawSkillPanel_org );
		IsDrawSkillPanel_org = 0;
	}
	// Отключить хук для IsDrawSkillPanelOverlay_org
	if ( IsDrawSkillPanelOverlay_org )
	{
		MH_DisableHook( IsDrawSkillPanelOverlay_org );
		IsDrawSkillPanelOverlay_org = 0;
	}
	// Отключить хук для IsNeedDrawUnit2org
	if ( IsNeedDrawUnit2org )
	{
		MH_DisableHook( IsNeedDrawUnit2org );
		IsNeedDrawUnit2org = 0;
	}

	/*	if ( Storm_279_org )
		{
		MH_DisableHook( Storm_279_org );
		Storm_279_org = 0;
		}*/

	if ( pOnChatMessage_org )
	{
		MH_DisableHook( pOnChatMessage_org );
		pOnChatMessage_org = 0;
	}

	if ( SetGameAreaFOV_org )
	{
		MH_DisableHook( SetGameAreaFOV_org );
		SetGameAreaFOV_org = 0;
	}
}



char buffer[ 4096 ];

typedef int( __stdcall * pStorm_503 )( int a1, int a2, int a3 );
pStorm_503 Storm_503;

// Создает "прыжок" с одного участка кода в другой.
BOOL PlantDetourJMP( BYTE* source, const BYTE* destination, const int length )
{

	DWORD oldProtection;
	BOOL bRet = VirtualProtect( source, length, PAGE_EXECUTE_READWRITE, &oldProtection );

	if ( bRet == FALSE )
		return FALSE;

	source[ 0 ] = 0xE9;
	*( DWORD* ) ( source + 1 ) = ( DWORD ) ( destination - source ) - 5;

	for ( int i = 5; i < length; i++ )
		source[ i ] = 0x90;

	VirtualProtect( source, length, oldProtection, &oldProtection );

	return TRUE;
}


int pAttackSpeedLimit = 0;

char * bufferaddr = 0;

typedef int( __fastcall * pGetHeroInt )( int unitaddr, int unused, BOOL withbonus );
pGetHeroInt GetHeroInt;

int GetItemInSlotAddr = 0;


int __declspec( naked ) __cdecl GetUnitItemInSlot126a( int unitaddr, int slotid )
{
	__asm
	{
		mov ecx, [ esp + 4 ];
		push esi;
		mov eax, ecx;
		jmp GetItemInSlotAddr;
	}
}


int __declspec( naked ) __cdecl GetUnitItemInSlot127a( int unitaddr, int slotid )
{
	__asm
	{
		push ebp;
		mov ebp, esp;
		mov ecx, [ ebp + 8 ];
		push esi;
		mov eax, ecx;
		jmp GetItemInSlotAddr;
	}
}


typedef int( __cdecl * pGetItemTypeId )( int itemhandle );
pGetItemTypeId GetItemTypeId;

int __cdecl GetItemTypeInSlot( int unitaddr, int slotid )
{
	int itemhandle = 0;

	if ( GameVersion == 0x26a )
	{
		itemhandle = GetUnitItemInSlot126a( unitaddr, slotid );
	}
	else if ( GameVersion == 0x27a )
	{
		itemhandle = GetUnitItemInSlot127a( unitaddr, slotid );
	}

	if ( itemhandle )
	{
		return GetItemTypeId( itemhandle );
	}

	return 0;
}


BOOL IsClassEqual( int ClassID1, int ClassID2 )
{
	if ( ClassID1 > 0 && ClassID2 > 0 )
	{


		char unitclass1[ 5 ];
		char unitclass2[ 5 ];
		memset( unitclass1, 0, 5 );
		memset( unitclass2, 0, 5 );

		int nClassID1 = ( int ) &ClassID1;
		int nClassID2 = ( int ) &ClassID2;


		*( BYTE* ) &unitclass1[ 0 ] = *( BYTE* ) ( nClassID1 + 3 );
		*( BYTE* ) &unitclass1[ 1 ] = *( BYTE* ) ( nClassID1 + 2 );
		*( BYTE* ) &unitclass1[ 2 ] = *( BYTE* ) ( nClassID1 + 1 );
		*( BYTE* ) &unitclass1[ 3 ] = *( BYTE* ) ( nClassID1 + 0 );

		*( BYTE* ) &unitclass2[ 0 ] = *( BYTE* ) ( nClassID2 + 3 );
		*( BYTE* ) &unitclass2[ 1 ] = *( BYTE* ) ( nClassID2 + 2 );
		*( BYTE* ) &unitclass2[ 2 ] = *( BYTE* ) ( nClassID2 + 1 );
		*( BYTE* ) &unitclass2[ 3 ] = *( BYTE* ) ( nClassID2 + 0 );


		//MessageBox( 0, unitclass1, unitclass2, 0 );

		return unitclass1[ 0 ] == unitclass2[ 0 ]
			&& unitclass1[ 1 ] == unitclass2[ 1 ]
			&& unitclass1[ 2 ] == unitclass2[ 2 ]
			&& unitclass1[ 3 ] == unitclass2[ 3 ];
	}

	return FALSE;
}

// Функция принимает данные о скорости атаки (и о увеличении урона от способностей) и сохраняет в буфер который будет использоваться при отрисовке
int __stdcall PrintAttackSpeedAndOtherInfo( int addr, float * attackspeed, float * BAT, int * unitaddr )
{
	if ( unitaddr != 0 )
	{
		if ( *unitaddr > 0 )
		{
			if ( IsNotBadUnit( *unitaddr ) && IsHero( *unitaddr ) )
			{
				bufferaddr = buffer;
				float realBAT = *( float* ) BAT;
				float realattackspeed = *( float* ) attackspeed;
				if ( realattackspeed > *( float* ) ( GameDll + pAttackSpeedLimit ) )
					realattackspeed = *( float* ) ( GameDll + pAttackSpeedLimit );

				if ( realattackspeed == 0 )
				{
					realattackspeed = 0.0001f;
				}

				if ( realBAT == 0 )
				{
					realBAT = 0.0001f;
				}

				int magicamp = GetHeroInt( *unitaddr, 0, TRUE ) / 16;
				int magicampbonus = 0;

				for ( int i = 0; i < 6; i++ )
				{
					if ( IsClassEqual( GetItemTypeInSlot( *unitaddr, i ), 'I0UF' ) )
					{
						magicampbonus += 5;
					}
				}


				if ( magicampbonus > 0 )
				{
					sprintf_s( buffer, sizeof( buffer ), "%.3f (Reload: %.2f sec)|nAttack speed bonus: %.1f|nMagic amplification: %i%% (|cFF20FF20+%i%%|r)|n", ( realattackspeed / *( float* ) BAT ), 1.0f / ( realattackspeed / *( float* ) BAT ), realattackspeed * 100.0 - 100.0, magicamp, magicampbonus );
				}
				else
				{
					sprintf_s( buffer, sizeof( buffer ), "%.3f (Reload: %.2f sec)|nAttack speed bonus: %.1f|nMagic amplification: %i%%|n", ( realattackspeed / *( float* ) BAT ), 1.0f / ( realattackspeed / *( float* ) BAT ), realattackspeed * 100.0 - 100.0, magicamp );
				}

				__asm
				{
					PUSH 0x200;
					PUSH bufferaddr;
					PUSH addr;
					CALL Storm_503;
				}
			}
			else if ( IsNotBadUnit( *unitaddr ) )
			{
				bufferaddr = buffer;
				float oldaddtackspeed = *( float* ) attackspeed;
				float realattackspeed = oldaddtackspeed;
				if ( realattackspeed > *( float* ) ( GameDll + pAttackSpeedLimit ) )
					realattackspeed = *( float* ) ( GameDll + pAttackSpeedLimit );

				sprintf_s( buffer, sizeof( buffer ), "%.3f (Reload: %.2f sec)", ( realattackspeed / *( float* ) BAT ), 1.0f / ( realattackspeed / *( float* ) BAT ) );

				__asm
				{
					PUSH 0x200;
					PUSH bufferaddr;
					PUSH addr;
					CALL Storm_503;
				}
			}
		}
	}

	return 0;
}

int saveeax = 0;
int saveebx = 0;
int saveecx = 0;
int saveedx = 0;
int saveesi = 0;
int saveedi = 0;
int saveebp = 0;
int saveesp = 0;

#pragma optimize("",off)

void __declspec( naked )  PrintAttackSpeedAndOtherInfoHook126a( )
{
	__asm
	{
		mov saveeax, eax;
		mov eax, [ esp + 0x14 ];
		cmp eax, 0;
		JE JUSTEND;
		add eax, 0x30;
		push eax;
		add eax, 0x128;
		push eax;
		add eax, 0x58;
		push eax;
		push esi;
		call PrintAttackSpeedAndOtherInfo;
		JUSTEND:;
		mov eax, saveeax;
		ret 8;
	}
}


void __declspec( naked )  PrintAttackSpeedAndOtherInfoHook127a( )
{
	__asm
	{
		mov saveeax, eax;
		mov eax, [ esp + 0x10 ];
		cmp eax, 0;
		JE JUSTEND;
		add eax, 0x30;
		push eax;
		add eax, 0x128;
		push eax;
		add eax, 0x58;
		push eax;
		push ecx;
		call PrintAttackSpeedAndOtherInfo;
		JUSTEND:;
		mov eax, saveeax;
		ret 8;
	}
}

int __stdcall PrintMoveSpeed( int addr, float * movespeed )
{
	bufferaddr = buffer;
	sprintf_s( buffer, sizeof( buffer ), "%.1f", ( *( float* ) movespeed ) );

	__asm
	{
		PUSH 0x200;
		PUSH bufferaddr;
		PUSH addr;
		CALL Storm_503;
	}

}


void __declspec( naked )  PrintMoveSpeedHook126a( )
{
	__asm
	{
		mov saveeax, eax;
		mov eax, esp;
		add eax, 4;
		push eax;
		push esi;
		call PrintMoveSpeed;
		mov eax, saveeax;
		ret 8;
	}
}

void __declspec( naked )  PrintMoveSpeedHook127a( )
{
	__asm
	{
		mov saveeax, eax;
		mov eax, esp;
		add eax, 4;
		push eax;
		push ecx;
		call PrintMoveSpeed;
		mov eax, saveeax;
		ret 8;
	}
}


#pragma optimize("",on)


char itemstr1[ 128 ];
char itemstr2[ 128 ];

char unitstr1[ 128 ];
char unitstr2[ 128 ];



typedef unsigned int( __cdecl * pGetPlayerColor )( int whichPlayer );
pGetPlayerColor GetPlayerColor;

typedef int( __cdecl * pPlayer )( int number );
pPlayer Player;



int         PLAYER_COLOR_RED = 0;
int         PLAYER_COLOR_BLUE = 1;
int         PLAYER_COLOR_CYAN = 2;
int         PLAYER_COLOR_PURPLE = 3;
int         PLAYER_COLOR_YELLOW = 4;
int         PLAYER_COLOR_ORANGE = 5;
int         PLAYER_COLOR_GREEN = 6;
int         PLAYER_COLOR_PINK = 7;
int         PLAYER_COLOR_LIGHT_GRAY = 8;
int         PLAYER_COLOR_LIGHT_BLUE = 9;
int         PLAYER_COLOR_AQUA = 10;
int         PLAYER_COLOR_BROWN = 11;


const char * GetPlayerColorString( int player )
{
	int c = GetPlayerColor( player );
	if ( c == PLAYER_COLOR_RED )
		return "|cffFF0202";
	else if ( c == PLAYER_COLOR_BLUE )
		return "|cff0031FF";
	else if ( c == PLAYER_COLOR_CYAN )
		return "|cff1BE5B8";
	else if ( c == PLAYER_COLOR_PURPLE )
		return "|cff530080";
	else if ( c == PLAYER_COLOR_YELLOW )
		return "|cffFFFC00";
	else if ( c == PLAYER_COLOR_ORANGE )
		return "|cffFE890D";
	else if ( c == PLAYER_COLOR_GREEN )
		return "|cff1FBF00";
	else if ( c == PLAYER_COLOR_PINK )
		return "|cffE45AAF";
	else if ( c == PLAYER_COLOR_LIGHT_GRAY )
		return "|cff949596";
	else if ( c == PLAYER_COLOR_LIGHT_BLUE )
		return "|cff7DBEF1";
	else if ( c == PLAYER_COLOR_AQUA )
		return "|cff0F6145";
	else if ( c == PLAYER_COLOR_BROWN )
		return "|cff4D2903";
	else
		return "|cffFFFFFF";

}



int __stdcall SaveStringsForPrintItem( int itemaddr )
{
	if ( itemaddr > 0 )
	{
		if ( IsNotBadItem( itemaddr ) )
		{
			int itemowner = *( int* ) ( itemaddr + 0x74 );
			if ( itemowner > 11 || itemowner < 0 )
			{
				sprintf_s( itemstr1, 128, "%%s%%s%%s%%s%%s" );
				sprintf_s( itemstr2, 128, "%%s%%s%%s" );
			}
			else
			{
				sprintf_s( itemstr1, 128, "Owned by %s%s|r|n%%s%%s%%s%%s%%s", GetPlayerColorString( Player( itemowner ) ), GetPlayerName( itemowner, 0 ) );
				sprintf_s( itemstr2, 128, "Owned by %s%s|r|n%%s%%s%%s", GetPlayerColorString( Player( itemowner ) ), GetPlayerName( itemowner, 0 ) );
			}
		}
	}
	return itemaddr;
}

int pGameClass1 = 0;

int GetUnitAddressFloatsRelated( int unitaddr, int step )
{
	int offset1 = unitaddr + step;
	int offset2 = *( int* ) pGameClass1;

	if ( *( int* ) offset1 &&  offset2 )
	{
		offset1 = *( int* ) offset1;
		offset2 = *( int* ) ( offset2 + 0xC );
		if ( offset2 )
		{
			return *( int* ) ( ( offset1 * 8 ) + offset2 + 4 );
		}
	}
	return 0;
}


float GetUnitHPregen( int unitaddr )
{
	int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xA0 );
	if ( offset1 )
	{
		return *( float* ) ( offset1 + 0x7C );
	}
	return 0.0f;
}

float GetUnitMPregen( int unitaddr )
{
	int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xC0 );
	if ( offset1 )
	{
		return *( float* ) ( offset1 + 0x7C );
	}
	return 0.0f;
}


int __stdcall SaveStringForHP_MP( int unitaddr )
{
	if ( IsKeyPressed( VK_LMENU ) )
	{
		if ( unitaddr )
		{
			if ( IsNotBadUnit( unitaddr ) )
			{
				float unitreghp = GetUnitHPregen( unitaddr );
				float unitregmp = GetUnitMPregen( unitaddr );



				if ( unitreghp == 0.0f )
				{
					sprintf_s( unitstr1, 128, "%%u|r |cFF00FF00+0.00|r" );
				}
				else if ( unitreghp < 9999.0f )
				{
					sprintf_s( unitstr1, 128, "%%u|r |cFF00FF00+%.1f|r", unitreghp );
				}
				else
				{
					sprintf_s( unitstr1, 128, "%%u |cFF00FF00+BIG|r" );
				}



				if ( unitregmp == 0.0f )
				{
					sprintf_s( unitstr2, 128, "%%u|r |cFF00FFFF+0.00|r" );
				}
				else if ( unitregmp < 9999.0f )
				{
					sprintf_s( unitstr2, 128, "%%u|r |cFF00FFFF+%.1f|r", unitregmp );
				}
				else
				{
					sprintf_s( unitstr2, 128, "%%u |cFF00FFFF+BIG|r" );
				}




			}
		}
		return unitaddr;
	}
	sprintf_s( unitstr1, 128, "%%u / %%u" );
	sprintf_s( unitstr2, 128, "%%u / %%u" );
	return unitaddr;
}


unsigned int hpbarcolorsHero[ 20 ];
unsigned int hpbarcolorsUnit[ 20 ];
unsigned int hpbarcolorsTower[ 20 ];

float hpbarscaleHeroX[ 20 ];
float hpbarscaleUnitX[ 20 ];
float hpbarscaleTowerX[ 20 ];

float hpbarscaleHeroY[ 20 ];
float hpbarscaleUnitY[ 20 ];
float hpbarscaleTowerY[ 20 ];

void __cdecl SetHPBarColorForPlayer( int playerid, unsigned int herocolor,
									 unsigned int unitcolor, unsigned int towercolor )
{
	if ( playerid >= 0 && playerid < 20 )
	{
		hpbarcolorsHero[ playerid ] = herocolor;
		hpbarcolorsUnit[ playerid ] = unitcolor;
		hpbarcolorsTower[ playerid ] = towercolor;
	}
}


void __cdecl SetHPBarXScaleForPlayer( int playerid, float heroscale,
									  float unitscale, float towerscale )
{
	if ( playerid >= 0 && playerid < 20 )
	{
		hpbarscaleHeroX[ playerid ] = heroscale;
		hpbarscaleUnitX[ playerid ] = unitscale;
		hpbarscaleTowerX[ playerid ] = towerscale;
	}
}

void __cdecl SetHPBarYScaleForPlayer( int playerid, float heroscale,
									  float unitscale, float towerscale )
{
	if ( playerid >= 0 && playerid < 20 )
	{
		hpbarscaleHeroY[ playerid ] = heroscale;
		hpbarscaleUnitY[ playerid ] = unitscale;
		hpbarscaleTowerY[ playerid ] = towerscale;
	}
}



struct BarStruct
{
	int _BarClass;
	int _unk1_flag;
	int _unk2_flag;
	int _unk3_pointer;
	int _unk4;
	int _unk5;
	int _unk6;
	int _unk7;
	int _unk8;
	int _unk9;
	int _unk10;
	int _unk11;
	int _unk12;
	int _unk13;
	int _unk14;
	int _unk15_pointer;
	int _unk16_pointer;
	float offset1;
	float offset2;
	float offset3;
	float offset4;
	int _unk17_flag;
	float ScaleX;
	float ScaleY;
	float Scale;
	int _unk18;
	int _unk19_pointer;
	int _unk20;
	int _unk21;
	int _unk22;
	int _unk23;
	int _unk24;
	int _unk25;
	int bartype;
	int _unk26;
	int _unk27;
	int _unk28;
	float offset5;
	float offset6;
	float offset7;
	float offset8;
	float offset9;
	int _unk29;
	int _unk30;
	int _unk31;
	int _unk32;
	int _unk33;
	int _unk34;
	int _unk35;
	int _unk36;
	int _unk37;
	int _unk38;
	int _unk39;
	int _unk40_pointer;
	int _unk41_pointer;
	int _unk42;
	int _unk43_pointer;
	int _unk44_pointer;
	int _unk45;
	int _unk46_pointer;
	int _unk47_pointer;
	int _unk48;
	int _unk49_pointer;
	int _unk50_pointer;
	int _unk51;
	int _unk52_pointer;
	int _unk53_pointer;
	int _unk54;
	int _unk55_pointer;
	int _unk56_pointer;
	int _unk57;
	int _unk58_pointer;
	int _unk59_pointer;
	int _unk60;
	float offset10;
	float offset11;
	float offset12;
	int _unk61_pointer;
	int _unk62;
	int _unk63;
	int _unk64_pointer;
	int _unk65_pointer;
	int _unk66;
	int _unk67;
	float offset13;
	int unitaddr;
	int _unk68;
	int _unk69;
};

int __stdcall SetColorForUnit( unsigned int  * coloraddr, BarStruct * BarStruct )
{


	if ( !BarStruct )
		return 0;
	if ( !coloraddr )
		return 0;

	int unitaddr = BarStruct->unitaddr;
	if ( !unitaddr )
		return 0;

	int unitslot = GetUnitOwnerSlot( unitaddr );

	if ( IsHero( unitaddr ) )
	{
		if ( BarStruct->bartype == 1 )
		{
			if ( hpbarscaleHeroX[ unitslot ] != 0.0 )
			{
				BarStruct->ScaleX = hpbarscaleHeroX[ unitslot ];
			}

			if ( hpbarscaleHeroY[ unitslot ] != 0.0 )
			{
				BarStruct->ScaleY = hpbarscaleHeroY[ unitslot ];
			}
		}

		if ( hpbarcolorsHero[ unitslot ] != 0 )
		{
			*coloraddr = hpbarcolorsHero[ unitslot ];
		}

	}
	else if ( IsTower( unitaddr ) )
	{
		if ( BarStruct->bartype == 1 )
		{
			if ( hpbarscaleTowerX[ unitslot ] != 0.0 )
			{
				BarStruct->ScaleX = hpbarscaleTowerX[ unitslot ];
			}

			if ( hpbarscaleTowerY[ unitslot ] != 0.0 )
			{
				BarStruct->ScaleY = hpbarscaleTowerY[ unitslot ];
			}
		}
		if ( hpbarcolorsTower[ unitslot ] != 0 )
		{
			*coloraddr = hpbarcolorsTower[ unitslot ];
		}
	}
	else
	{
		if ( BarStruct->bartype == 1 )
		{
			if ( hpbarscaleUnitX[ unitslot ] != 0.0 )
			{
				BarStruct->ScaleX = hpbarscaleUnitX[ unitslot ];
			}

			if ( hpbarscaleUnitY[ unitslot ] != 0.0 )
			{
				BarStruct->ScaleY = hpbarscaleUnitY[ unitslot ];
			}
		}

		if ( hpbarcolorsUnit[ unitslot ] != 0 )
		{
			*coloraddr = hpbarcolorsUnit[ unitslot ];
		}
	}


	return unitaddr;
}



const char * DISPATH_OK = "%sDIS%s";
const char * DISPATH_NO = "%s%s";

const char * DISPATH_SEARCH = "Disabled\\DIS";


BOOL __stdcall IsFoundDISPATH( const char * path )
{
	return strstr( path, DISPATH_SEARCH ) > 0;
}


#pragma region HookFunctions
#pragma optimize("",off)
int JumpBackAddr1( )
{
	MessageBox( 0, 0, 0, 1 );
	return 0;
}

int JumpBackAddr2( )
{
	MessageBox( 0, 0, 0, 2 );
	return 0;
}

int JumpBackAddr3( )
{
	MessageBox( 0, 0, 0, 3 );
	return 0;
}


int JumpBackAddr4( )
{
	MessageBox( 0, 0, 0, 4 );
	return 0;
}

int JumpBackAddr5( )
{
	MessageBox( 0, 0, 0, 5 );
	return 0;
}

int JumpBackAddr6( )
{
	MessageBox( 0, 0, 0, 6 );
	return 0;
}


int JumpBackAddr7( )
{
	MessageBox( 0, 0, 0, 7 );
	return 0;
}


int JumpBackAddr8( )
{
	MessageBox( 0, 0, 0, 8 );
	return 0;
}


int JumpBackAddr9( )
{
	MessageBox( 0, 0, 0, 9 );
	return 0;
}


void __declspec( naked ) HookIconHelper126a( )
{
	__asm
	{
		push esi;
		call IsFoundDISPATH;
		cmp eax, 0;
		je NODISPATH;
		push DISPATH_NO;
		jmp JumpBackAddr8;
		NODISPATH:
		push DISPATH_OK;
		jmp JumpBackAddr8;
	}
}



void __declspec( naked ) HookIconHelper127a( )
{
	__asm
	{
		push esi;
		call IsFoundDISPATH;
		cmp eax, 0;
		je NODISPATH;
		push DISPATH_NO;
		jmp JumpBackAddr8;
		NODISPATH:
		push DISPATH_OK;
		jmp JumpBackAddr8;
	}
}



void __declspec( naked ) HookHPBarColorHelper126a( )
{
	__asm
	{
		lea edx, [ esp + 0x10 ];
		fstp st( 0 );
		pushad;
		push edi;
		push edx;
		call SetColorForUnit;
		popad;
		jmp JumpBackAddr9;
	}
}


int calladdr1 = 0;



void __declspec( naked ) HookHPBarColorHelper127a( )
{
	__asm
	{
		lea eax, [ ebp + 0x08 ];
		pushad;
		push esi;
		push eax;
		call SetColorForUnit;
		popad;
		push eax;
		call calladdr1;
		jmp JumpBackAddr9;
	}
}



void __declspec( naked ) HookItemAddr126a( )
{
	__asm
	{
		push eax;
		call SaveStringsForPrintItem;
		mov ebx, eax;
		test ebx, ebx;
		mov[ esp + 0x1C ], ebx;
		jmp JumpBackAddr1;
	}
}


void __declspec( naked ) HookItemAddr127a( )
{
	__asm
	{
		push eax;
		call SaveStringsForPrintItem;
		mov edi, eax;
		mov[ ebp - 0x4F8 ], edi;
		jmp JumpBackAddr1;
	}
}



void __declspec( naked ) HookUnitAddr126a( )
{
	__asm
	{
		push edx;
		mov eax, ecx;
		push eax;
		call SaveStringForHP_MP;
		mov ecx, eax;
		pop edx;
		push edx;
		lea eax, [ esp + 0x40 ];
		jmp JumpBackAddr4;
	}
}


void __declspec( naked ) HookUnitAddr127a( )
{
	__asm
	{
		mov ecx, [ edi + 0x00000238 ];
		mov eax, ecx;
		push eax;
		call SaveStringForHP_MP;
		mov ecx, eax;
		test ecx, ecx;
		jmp JumpBackAddr4;
	}
}



void __declspec( naked ) HookPrint1_126a( )
{
	bufferaddr = itemstr1;
	__asm
	{
		push bufferaddr;
		lea eax, [ esp + 0xD8 ];
		push 0x300;
		jmp JumpBackAddr2;
	}
}


void __declspec( naked ) HookPrint2_126a( )
{
	bufferaddr = itemstr2;
	__asm
	{
		push bufferaddr;
		lea edx, [ esp + 0xD0 ];
		push 0x300;
		jmp JumpBackAddr3;
	}
}



void __declspec( naked ) HookPrint3_126a( )
{
	bufferaddr = unitstr1;
	__asm
	{
		push bufferaddr;
		lea ecx, [ esp + 0x0000100 ];
		push 0x28;
		jmp JumpBackAddr5;
	}
}

void __declspec( naked ) HookPrint4_126a( )
{
	bufferaddr = unitstr2;
	__asm
	{
		push bufferaddr;
		lea eax, [ esp + 0x0000128 ];
		push 0x28;
		jmp JumpBackAddr6;
	}
}


void __declspec( naked ) HookPrint1_127a( )
{
	bufferaddr = itemstr1;
	__asm
	{
		push bufferaddr;
		lea eax, [ ebp - 0x2F0 ];
		push 0x300;
		jmp JumpBackAddr2;
	}
}


void __declspec( naked ) HookPrint2_127a( )
{
	bufferaddr = itemstr2;
	__asm
	{
		push bufferaddr;
		lea eax, [ ebp - 0x2F0 ];
		push 0x300;
		jmp JumpBackAddr3;
	}
}




void __declspec( naked ) HookPrint3_127a( )
{
	bufferaddr = unitstr1;
	__asm
	{
		push bufferaddr;
		lea eax, [ ebp - 0x60 ];
		push 0x28;
		jmp JumpBackAddr5;
	}
}

void __declspec( naked ) HookPrint4_127a( )
{
	bufferaddr = unitstr2;
	__asm
	{
		push bufferaddr;
		lea eax, [ ebp - 0x38 ];
		push 0x28;
		jmp JumpBackAddr6;
	}
}


void __declspec( naked ) HookSetCD_1000s_126a( )
{
	int cd_addr;
	__asm
	{
		mov cd_addr, eax;
	}

	*( float* ) ( cd_addr + 4 ) = 1000.0;

	__asm
	{
		push esi;
		push eax;
		mov eax, [ ecx ];
		mov eax, [ eax + 0x18 ];
		lea edx, [ esp + 0x08 ];
		push edx;
		jmp JumpBackAddr7;
	}
}

//37ed3
void __declspec( naked ) HookSetCD_1000s_127a( )
{
	int cd_addr;
	__asm
	{
		mov cd_addr, eax;
	}
	*( float* ) ( cd_addr + 4 ) = 1000.0;

	__asm
	{
		lea ecx, [ edx + 0xD0 ];
		jmp JumpBackAddr7;
	}
}







#pragma optimize("",on)



#pragma endregion






#pragma region BackupOffsets
struct offsetdata
{
	int offaddr;
	int offdata;
};
std::vector<offsetdata> offsetslist;
__declspec( dllexport ) int __stdcall AddNewOffset( int address, int data )
{
	offsetdata temp;
	temp.offaddr = address;
	temp.offdata = data;
	offsetslist.push_back( temp );

	return 1;
}
#pragma endregion


int InGame = 0;


void * threadid = 0;
unsigned long __stdcall RefreshTimer( void * )
{
	while ( TRUE )
	{
		// Ждать установки InGame адреса
		while ( !InGame )
		{
			Sleep( 1000 );
		}

		// Ждать входа в игру
		while ( !*( BOOL* ) InGame )
		{
			Sleep( 1000 );
		}

		// Ждать пока игра не закончится
		while ( *( BOOL* ) InGame )
		{
			Sleep( 1000 );
		}

		// Выгрузить перехватчики функций
		UninitializeHook( );
		// Отключить мут
		UnMutePlayer( 0 );

		// Убрать все патчи и вернуть стандартные данные
		// Т.к функция вызывается после завершения игры проблем быть не должно.
		for ( UINT i = 0; i < offsetslist.size( ); i++ )
		{
			offsetdata temp = offsetslist[ i ];
			DWORD oldprotect, oldprotect2;
			VirtualProtect( ( void* ) temp.offaddr, 4, PAGE_EXECUTE_READWRITE, &oldprotect );
			*( int* ) temp.offaddr = temp.offdata;
			VirtualProtect( ( void* ) temp.offaddr, 4, oldprotect, &oldprotect2 );
		}
		offsetslist.clear( );

		while ( !*( BOOL* ) InGame )
		{
			Sleep( 1000 );
		}

		Sleep( 1000 );
	}

	return 0;
}

void PatchOffset( void * addr, void * buffer, unsigned int size )
{
	DWORD OldProtect1, OldProtect2;
	VirtualProtect( addr, size, PAGE_EXECUTE_READWRITE, &OldProtect1 );
	for ( unsigned int i = 0; i < size; i++ )
	{
		*( unsigned char* ) ( ( int ) addr + i ) = *( unsigned char* ) ( ( int ) buffer + i );
	}

	VirtualProtect( addr, size, OldProtect1, &OldProtect2 );
}






__declspec( dllexport ) int __stdcall InitDotaHelper( int gameversion )
{
	if ( threadid )
	{
		TerminateThread( threadid, 0 );
		threadid = 0;
	}
	//RemoveMapSizeLimit( );
	GameVersion = gameversion;

	while ( mutedplayers.size( ) )
	{
		free( mutedplayers.back( ) );
		mutedplayers.pop_back( );
	}

	memset( hpbarcolorsHero, 0, sizeof( hpbarcolorsHero ) );
	memset( hpbarcolorsUnit, 0, sizeof( hpbarcolorsUnit ) );
	memset( hpbarcolorsTower, 0, sizeof( hpbarcolorsTower ) );
	memset( hpbarscaleHeroX, 0, sizeof( hpbarscaleHeroX ) );
	memset( hpbarscaleUnitX, 0, sizeof( hpbarscaleUnitX ) );
	memset( hpbarscaleTowerX, 0, sizeof( hpbarscaleTowerX ) );
	memset( hpbarscaleHeroY, 0, sizeof( hpbarscaleHeroY ) );
	memset( hpbarscaleUnitY, 0, sizeof( hpbarscaleUnitY ) );
	memset( hpbarscaleTowerY, 0, sizeof( hpbarscaleTowerY ) );


	SetWidescreenFixState( FALSE );
	SetCustomFovFix( 1.0f );

	if ( gameversion == 0x26a )
	{
		UninitializeHook( );


		pOnChatMessage_offset = 0x2FB480;
		IsNeedDrawUnit2offset = 0x28E1D0;
		IsNeedDrawUnit2offsetRetAddress = 0x2F9B60;
		IsPlayerEnemyOffset = 0x3C9580;
		GetPlayerByIDOffset = 0x3BBB30;
		IsDrawSkillPanelOffset = 0x34F280;
		DrawSkillPanelOffset = 0x2774C0;
		IsDrawSkillPanelOverlayOffset = 0x34F2C0;
		DrawSkillPanelOverlayOffset = 0x277570;
		IsNeedDrawUnitOriginOffset = 0x285DC0;
		GlobalPlayerOffset = 0xAB65F4;
		pAttackSpeedLimit = 0xAB0074;
		pW3XGlobalClass = GameDll + 0xAB4F80;
		pPrintText2 = GameDll + 0x2F69A0;
		pGameClass1 = GameDll + 0xAB7788;
		UnitVtable = GameDll + 0x931934;
		ItemVtable = GameDll + 0x9320B4;
		GetHeroInt = ( pGetHeroInt ) ( GameDll + 0x277850 );
		Storm_503 = ( pStorm_503 ) ( *( int* ) ( GameDll + 0x86D584 ) );
		InGame = GameDll + 0xAB62A4;
		GetItemInSlotAddr = GameDll + 0x3C7730 + 0xA;
		GetItemTypeId = ( pGetItemTypeId ) ( GameDll + 0x3C4C60 );
		GetPlayerColor = ( pGetPlayerColor ) ( GameDll + 0x3C1240 );
		Player = ( pPlayer ) ( GameDll + 0x3BBB30 );
		GetPlayerName = ( p_GetPlayerName ) ( GameDll + 0x2F8F90 );


		int pDrawAttackSpeed = GameDll + 0x339150;
		AddNewOffset( pDrawAttackSpeed, *( int* ) pDrawAttackSpeed );
		AddNewOffset( pDrawAttackSpeed + 3, *( int* ) ( pDrawAttackSpeed + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawAttackSpeed ), ( BYTE* ) PrintAttackSpeedAndOtherInfoHook126a, 5 );


		int pDrawMoveSpeed = GameDll + 0x338FB0;
		AddNewOffset( pDrawMoveSpeed, *( int* ) pDrawMoveSpeed );
		AddNewOffset( pDrawMoveSpeed + 3, *( int* ) ( pDrawMoveSpeed + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawMoveSpeed ), ( BYTE* ) PrintMoveSpeedHook126a, 5 );


		int pDrawItemText1 = GameDll + 0x369e72;
		AddNewOffset( pDrawItemText1, *( int* ) pDrawItemText1 );
		AddNewOffset( pDrawItemText1 + 3, *( int* ) ( pDrawItemText1 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawItemText1 ), ( BYTE* ) HookPrint1_126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr2 ), ( BYTE* ) ( GameDll + 0x369e83 ), 5 );


		int pDrawItemText2 = GameDll + 0x369ee6;
		AddNewOffset( pDrawItemText2, *( int* ) pDrawItemText2 );
		AddNewOffset( pDrawItemText2 + 3, *( int* ) ( pDrawItemText2 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawItemText2 ), ( BYTE* ) HookPrint2_126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr3 ), ( BYTE* ) ( GameDll + 0x369ef7 ), 5 );


		int pSaveLatestItem = GameDll + 0x369b3d;
		AddNewOffset( pSaveLatestItem, *( int* ) pSaveLatestItem );
		AddNewOffset( pSaveLatestItem + 3, *( int* ) ( pSaveLatestItem + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pSaveLatestItem ), ( BYTE* ) HookItemAddr126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr1 ), ( BYTE* ) ( GameDll + 0x369b45 ), 5 );


		int pSaveLatestUnit = GameDll + 0x3580ad;
		AddNewOffset( pSaveLatestUnit, *( int* ) pSaveLatestUnit );
		AddNewOffset( pSaveLatestUnit + 3, *( int* ) ( pSaveLatestUnit + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pSaveLatestUnit ), ( BYTE* ) HookUnitAddr126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr4 ), ( BYTE* ) ( GameDll + 0x3580b2 ), 5 );


		int pDrawUnitText1 = GameDll + 0x358198;
		AddNewOffset( pDrawUnitText1, *( int* ) pDrawUnitText1 );
		AddNewOffset( pDrawUnitText1 + 3, *( int* ) ( pDrawUnitText1 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawUnitText1 ), ( BYTE* ) HookPrint3_126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr5 ), ( BYTE* ) ( GameDll + 0x3581a6 ), 5 );


		int pDrawUnitText2 = GameDll + 0x3583c2;
		AddNewOffset( pDrawUnitText2, *( int* ) pDrawUnitText2 );
		AddNewOffset( pDrawUnitText2 + 3, *( int* ) ( pDrawUnitText2 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawUnitText2 ), ( BYTE* ) HookPrint4_126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr6 ), ( BYTE* ) ( GameDll + 0x3583d0 ), 5 );


		int pAlwaysRefresh1 = GameDll + 0x35818c;
		int pAlwaysRefresh2 = GameDll + 0x3583ba;
		unsigned char JMPBYTE = 0xEB;

		AddNewOffset( pAlwaysRefresh1, *( int* ) pAlwaysRefresh1 );
		AddNewOffset( pAlwaysRefresh2, *( int* ) pAlwaysRefresh2 );
		PatchOffset( ( void* ) pAlwaysRefresh1, &JMPBYTE, 1 );
		PatchOffset( ( void* ) pAlwaysRefresh2, &JMPBYTE, 1 );


		int pSetCooldown = GameDll + 0x37ed3;
		AddNewOffset( pSetCooldown, *( int* ) pSetCooldown );
		AddNewOffset( pSetCooldown + 3, *( int* ) ( pSetCooldown + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pSetCooldown ), ( BYTE* ) HookSetCD_1000s_126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr7 ), ( BYTE* ) ( GameDll + 0x37edf ), 5 );


		int pICONHELPER = GameDll + 0x32093B;
		AddNewOffset( pICONHELPER, *( int* ) pICONHELPER );
		AddNewOffset( pICONHELPER + 3, *( int* ) ( pICONHELPER + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pICONHELPER ), ( BYTE* ) HookIconHelper126a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr8 ), ( BYTE* ) ( GameDll + 0x320940 ), 5 );

		int pHPBARHELPER = GameDll + 0x364beb;
		AddNewOffset( pHPBARHELPER, *( int* ) pHPBARHELPER );
		AddNewOffset( pHPBARHELPER + 3, *( int* ) ( pHPBARHELPER + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pHPBARHELPER ), ( BYTE* ) HookHPBarColorHelper126a, 6 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr9 ), ( BYTE* ) ( GameDll + 0x364bf1 ), 5 );





		MapNameOffset1 = GameDll + 0xAAE788;
		MapNameOffset2 = 8;


		SetGameAreaFOVoffset = 0x7B66F0;

		GetWindowXoffset = 0xADE91C;
		GetWindowYoffset = 0xADE918;


		InitHook( );
		threadid = CreateThread( 0, 0, RefreshTimer, 0, 0, 0 );


		/* crc32 simple protection */
		char outfilename[ MAX_PATH ];
		GetModuleFileName( GetCurrentModule, outfilename, MAX_PATH );
		DWORD dwCrc32;
		CCrc32Dynamic *pobCrc32Dynamic = new CCrc32Dynamic;
		pobCrc32Dynamic->Init( );
		pobCrc32Dynamic->FileCrc32Assembly( outfilename, dwCrc32 );
		pobCrc32Dynamic->Free( );
		delete pobCrc32Dynamic;
		return dwCrc32;
	}
	else if ( gameversion == 0x27a )
	{
		UninitializeHook( );


		pOnChatMessage_offset = 0x355CF0;
		IsNeedDrawUnit2offset = 0x66E710;
		IsNeedDrawUnit2offsetRetAddress = 0x359D60;
		IsPlayerEnemyOffset = 0x1E8090;
		GetPlayerByIDOffset = 0x1F1E70;
		IsDrawSkillPanelOffset = 0x3927F0;
		DrawSkillPanelOffset = 0x660D90;
		IsDrawSkillPanelOverlayOffset = 0x392830;
		DrawSkillPanelOverlayOffset = 0x660E40;
		IsNeedDrawUnitOriginOffset = 0x651530;
		GlobalPlayerOffset = 0xBE4238;
		pAttackSpeedLimit = 0xBE7A04;
		pW3XGlobalClass = GameDll + 0xBE6350;
		pPrintText2 = GameDll + 0x3577B0;
		pGameClass1 = GameDll + 0xBE40A8;
		UnitVtable = GameDll + 0xA4A704;
		ItemVtable = GameDll + 0xA4A2EC;
		GetHeroInt = ( pGetHeroInt ) ( GameDll + 0x6677F0 );
		Storm_503 = ( pStorm_503 ) ( *( int* ) ( GameDll + 0x94e684 ) );
		InGame = GameDll + 0xBE6530;
		GetItemInSlotAddr = GameDll + 0x1FAF50 + 0xC;
		GetItemTypeId = ( pGetItemTypeId ) ( GameDll + 0x1E2CC0 );
		GetPlayerColor = ( pGetPlayerColor ) ( GameDll + 0x1E3CA0 );
		Player = ( pPlayer ) ( GameDll + 0x1F1E70 );
		GetPlayerName = ( p_GetPlayerName ) ( GameDll + 0x34F730 );


		int pDrawAttackSpeed = GameDll + 0x38C6E0;
		AddNewOffset( pDrawAttackSpeed, *( int* ) pDrawAttackSpeed );
		AddNewOffset( pDrawAttackSpeed + 3, *( int* ) ( pDrawAttackSpeed + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawAttackSpeed ), ( BYTE* ) PrintAttackSpeedAndOtherInfoHook127a, 5 );


		int pDrawMoveSpeed = GameDll + 0x38D440;
		AddNewOffset( pDrawMoveSpeed, *( int* ) pDrawMoveSpeed );
		AddNewOffset( pDrawMoveSpeed + 3, *( int* ) ( pDrawMoveSpeed + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawMoveSpeed ), ( BYTE* ) PrintMoveSpeedHook127a, 5 );


		int pDrawItemText1 = GameDll + 0x3ab720;
		AddNewOffset( pDrawItemText1, *( int* ) pDrawItemText1 );
		AddNewOffset( pDrawItemText1 + 3, *( int* ) ( pDrawItemText1 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawItemText1 ), ( BYTE* ) HookPrint1_127a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr2 ), ( BYTE* ) ( GameDll + 0x3ab730 ), 5 );


		int pDrawItemText2 = GameDll + 0x3ab791;
		AddNewOffset( pDrawItemText2, *( int* ) pDrawItemText2 );
		AddNewOffset( pDrawItemText2 + 3, *( int* ) ( pDrawItemText2 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawItemText2 ), ( BYTE* ) HookPrint2_127a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr3 ), ( BYTE* ) ( GameDll + 0x3ab7a1 ), 5 );


		int pSaveLatestItem = GameDll + 0x3ab39e;
		AddNewOffset( pSaveLatestItem, *( int* ) pSaveLatestItem );
		AddNewOffset( pSaveLatestItem + 3, *( int* ) ( pSaveLatestItem + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pSaveLatestItem ), ( BYTE* ) HookItemAddr127a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr1 ), ( BYTE* ) ( GameDll + 0x3ab3a6 ), 5 );


		int pSaveLatestUnit = GameDll + 0x3bb8fd;
		AddNewOffset( pSaveLatestUnit, *( int* ) pSaveLatestUnit );
		AddNewOffset( pSaveLatestUnit + 3, *( int* ) ( pSaveLatestUnit + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pSaveLatestUnit ), ( BYTE* ) HookUnitAddr127a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr4 ), ( BYTE* ) ( GameDll + 0x3bb905 ), 5 );


		int pDrawUnitText1 = GameDll + 0x3bbd5e;
		AddNewOffset( pDrawUnitText1, *( int* ) pDrawUnitText1 );
		AddNewOffset( pDrawUnitText1 + 3, *( int* ) ( pDrawUnitText1 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawUnitText1 ), ( BYTE* ) HookPrint3_127a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr5 ), ( BYTE* ) ( GameDll + 0x3bbd68 ), 5 );


		int pDrawUnitText2 = GameDll + 0x3bbf4a;
		AddNewOffset( pDrawUnitText2, *( int* ) pDrawUnitText2 );
		AddNewOffset( pDrawUnitText2 + 3, *( int* ) ( pDrawUnitText2 + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pDrawUnitText2 ), ( BYTE* ) HookPrint4_127a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr6 ), ( BYTE* ) ( GameDll + 0x3bbf54 ), 5 );


		int pAlwaysRefresh1 = GameDll + 0x3bbd52;
		int pAlwaysRefresh2 = GameDll + 0x3bbf42;
		unsigned char JMPBYTE = 0xEB;

		AddNewOffset( pAlwaysRefresh1, *( int* ) pAlwaysRefresh1 );
		AddNewOffset( pAlwaysRefresh2, *( int* ) pAlwaysRefresh2 );
		PatchOffset( ( void* ) pAlwaysRefresh1, &JMPBYTE, 1 );
		PatchOffset( ( void* ) pAlwaysRefresh2, &JMPBYTE, 1 );


		int pSetCooldown = GameDll + 0x3F717F;
		AddNewOffset( pSetCooldown, *( int* ) pSetCooldown );
		AddNewOffset( pSetCooldown + 3, *( int* ) ( pSetCooldown + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pSetCooldown ), ( BYTE* ) HookSetCD_1000s_127a, 6 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr7 ), ( BYTE* ) ( GameDll + 0x3F7185 ), 5 );



		int pICONHELPER = GameDll + 0x336f68;
		AddNewOffset( pICONHELPER, *( int* ) pICONHELPER );
		AddNewOffset( pICONHELPER + 3, *( int* ) ( pICONHELPER + 3 ) );
		PlantDetourJMP( ( BYTE* ) ( pICONHELPER ), ( BYTE* ) HookIconHelper127a, 5 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr8 ), ( BYTE* ) ( GameDll + 0x336f6d ), 5 );


		calladdr1 = GameDll + 0xBFA30;
		int pHPBARHELPER = GameDll + 0x3bd5b0;
		AddNewOffset( pHPBARHELPER, *( int* ) pHPBARHELPER );
		AddNewOffset( pHPBARHELPER + 3, *( int* ) ( pHPBARHELPER + 3 ) );
		AddNewOffset( pHPBARHELPER + 6, *( int* ) ( pHPBARHELPER + 6 ) );
		PlantDetourJMP( ( BYTE* ) ( pHPBARHELPER ), ( BYTE* ) HookHPBarColorHelper127a, 9 );
		PlantDetourJMP( ( BYTE* ) ( JumpBackAddr9 ), ( BYTE* ) ( GameDll + 0x3bd5b9 ), 5 );





		MapNameOffset1 = GameDll + 0xBEE150;
		MapNameOffset2 = 8;



		GetWindowXoffset = 0xBBA22C;
		GetWindowYoffset = 0xBBA228;

		SetGameAreaFOVoffset = 0xD31D0;

		InitHook( );
		threadid = CreateThread( 0, 0, RefreshTimer, 0, 0, 0 );


		/* crc32 simple protection */
		char outfilename[ MAX_PATH ];
		GetModuleFileNameA( GetCurrentModule, outfilename, MAX_PATH );
		DWORD dwCrc32;
		CCrc32Dynamic *pobCrc32Dynamic = new CCrc32Dynamic;
		pobCrc32Dynamic->Init( );
		pobCrc32Dynamic->FileCrc32Assembly( outfilename, dwCrc32 );
		pobCrc32Dynamic->Free( );
		delete pobCrc32Dynamic;
		return dwCrc32;
	}

	return 0;
}



#pragma region Main
BOOL __stdcall DllMain( HINSTANCE Module, UINT reason, LPVOID )
{
	GetCurrentModule = Module;
	if ( reason == DLL_PROCESS_ATTACH )
	{
		GameDllModule = GetModuleHandle( "Game.dll" );
		GameDll = ( int ) GameDllModule;
		if ( !GameDll )
		{
			MessageBox( 0, "Need inject after Game.dll.", "ERROR! No game.dll found!", 0 );
		}
		// Инициализация "перехватчика" функций
		MH_Initialize( );

		///InitDotaHelper( 0x26a );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		// Отключить все "перехватчики" функций
		UninitializeHook( );
		// Выгрузить "перехватчик" функций
		MH_Uninitialize( );


		if ( threadid )
			TerminateThread( threadid, 0 );

		// Восстановить все изменения перед тем как выгрузить DLL
		for ( UINT i = 0; i < offsetslist.size( ); i++ )
		{
			offsetdata temp = offsetslist[ i ];
			DWORD oldprotect, oldprotect2;
			VirtualProtect( ( void* ) temp.offaddr, 4, PAGE_EXECUTE_READWRITE, &oldprotect );
			*( int* ) temp.offaddr = temp.offdata;
			VirtualProtect( ( void* ) temp.offaddr, 4, oldprotect, &oldprotect2 );
		}
		offsetslist.clear( );

	}
	return TRUE;
}
#pragma endregion










































#pragma region Developer
//"Abso!(d3scene.ru)"
#pragma endregion