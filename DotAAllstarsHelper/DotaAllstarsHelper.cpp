
#include "Main.h"
#include "ViewAllySkill.h"


#include <stdint.h>
#include "Crc32Dynamic.h"

bool FileExist( const char * name )
{
	ifstream f( name );
	return f.good( );
}

//  Game.dll
int GameDll = 0;
//	Storm.dll
int StormDll = 0;
HMODULE GameDllModule = 0;
HMODULE StormDllModule = 0;
int GameVersion = 0;

#pragma region All Offsets Here

int GlobalPlayerOffset = 0;
int IsPlayerEnemyOffset = 0;
int GetPlayerByIDOffset = 0;


int DrawSkillPanelOffset = 0;
int DrawSkillPanelOverlayOffset = 0;

int IsDrawSkillPanelOffset = 0;
int IsDrawSkillPanelOverlayOffset = 0;
int IsNeedDrawUnitOriginOffset = 0;
int IsNeedDrawUnit2offset = 0;
int IsNeedDrawUnit2offsetRetAddress = 0;

BOOL * InGame = 0;
int IsWindowActive = 0;
int ChatFound = 0;

int pW3XGlobalClass = 0;
int pGameClass1 = 0;
int pWar3Data1 = 0;

int UnitVtable = 0;
int ItemVtable = 0;

int pPrintText2 = 0;


int MapNameOffset1 = 0;
int MapNameOffset2 = 0;

int pOnChatMessage_offset;

int _BarVTable = 0;

int pAttackSpeedLimit = 0;

int GetItemInSlotAddr = 0;

int GetWindowXoffset = 0;
int GetWindowYoffset = 0;

#pragma endregion


HMODULE GetCurrentModule;
#pragma region Game.dll JassNatives
// Проверка являются ли игроки врагами
typedef int( __cdecl * IsPlayerEnemy )( UINT Player1, UINT Player2 ); /*Game+3C9580*/

// Получить хэндл игрока по его номеру слота
typedef UINT( __cdecl * GetPlayerByID )( int PlayerId ); /*Game+3BBB30*/


#pragma endregion





void * GetGlobalPlayerData( )
{
	return ( void * ) *( int * ) ( GlobalPlayerOffset + GameDll );
}

int GetPlayerByNumber( int number )
{
	void * arg1 = GetGlobalPlayerData( );
	int result = -1;
	if ( arg1 != nullptr )
	{
		result = ( int ) arg1 + ( number * 4 ) + 0x58;

		if ( result )
		{
			result = *( int* ) result;
		}
		else
		{
			return 0;
		}
	}
	return result;
}

// Получить слот игрока
int GetLocalPlayerId( )
{
	void * gldata = GetGlobalPlayerData( );

	int playerslotaddr = ( int ) gldata + 0x28;

	return ( int ) *( short * ) ( playerslotaddr );
}

int GetSelectedUnitCountBigger( int slot )
{
	int plr = GetPlayerByNumber( slot );
	if ( plr )
	{
		int PlayerData1 = *( int* ) ( plr + 0x34 );
		if ( PlayerData1 )
		{
			int unitcount = *( int * ) ( PlayerData1 + 0x10 );
			int unitcount2 = *( int * ) ( PlayerData1 + 0x1D4 );

			if ( unitcount > unitcount2 )
				return unitcount;
			else
				return unitcount2;
		}
	}

	return NULL;
}




void DisplayText( char *szText, float fDuration )
{
	DWORD dwDuration = *( ( DWORD * ) &fDuration );
	__asm
	{
		PUSH 0xFFFFFFFF;
		PUSH dwDuration;
		PUSH szText;
		MOV		ECX, [ pW3XGlobalClass ];
		MOV		ECX, [ ECX ];
		MOV		EAX, pPrintText2;
		CALL	EAX;
	}
}





// Получить имя игрока по его слоту
typedef char *( __fastcall * p_GetPlayerName )( int a1, int a2 );
p_GetPlayerName GetPlayerName = NULL;


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





typedef int( __stdcall * pStorm_279 )( const char*  a1, int a2, int a3, size_t Size, int a5 );
pStorm_279 Storm_279_org;
pStorm_279 Storm_279_ptr;


char MPQFilePath[ 4000 ];

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
	size_t cpylen, orglen, retlen, tolen = 0, fromlen = strlen( from );

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
		memcpy( pret, str, (size_t)pos_cache[ 0 ] );
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
	char * tmpstr = 0;
	if ( strstr( MPQFilePath, "Disabled\\DIS" ) )
	{
		tmpstr = repl_string( MPQFilePath, "Disabled\\DIS", "\\" );
		sprintf_s( MPQFilePath, 4000, "%s\0", tmpstr );
		free( tmpstr );
	}
}

// Функция открытия файла и получения его размера.
int __stdcall Storm_279my( const char* a1, int a2, int a3, size_t Size, int a5 )
{
	int retval = Storm_279_ptr( a1, a2, a3, Size, a5 );
	if ( !retval )
	{
		sprintf_s( MPQFilePath, 4000, "%s\0", a1 );
		ReplaceIconPathIfNeed( );
		retval = Storm_279_ptr( MPQFilePath, a2, a3, Size, a5 );
	}
	return retval;
}




char CurrentMapPath[ MAX_PATH ];
char NewMapPath[ MAX_PATH ];


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




__declspec( dllexport ) const char * __stdcall GetCurrentMapPath( int )
{
	BuildFilePath( NULL );
	return CurrentMapPath;
}



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
			mutedplayers.erase(  mutedplayers.begin( ) + (int)i );
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


int __fastcall SetGameAreaFOV_new( FloatStruct1 * a1, int _unused, float a3, float a4, float a5, float a6 )
{

	float ScreenX = *( float* ) ( GameDll + GetWindowXoffset );
	float ScreenY = *( float* ) ( GameDll + GetWindowYoffset );

	float v1 = 1.0f / sqrt( a4 * a4 + 1.0f );
	float v2 = tan( v1 * a3 * 0.5f );

	float v3 = v2 * a5;
	float v4 = v3 * a4;

	//0,0
	//1,0
	//2,0

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
/*
typedef int *( __fastcall * sub_6F379A30 )( void * a1, int unused, int a2, int a3 );
sub_6F379A30 sub_6F379A30_org;
sub_6F379A30 sub_6F379A30_ptr;
int * __fastcall sub_6F379A30my ( void * a1, int unused, int a2, int a3 )
{
	char msg[ 100 ];
	sprintf_s( msg, 100, "%X,%X,%X,%X->%X", ( int ) a1, unused, a2, a3, 0  );
	MessageBox( 0, msg, msg, 0 );
	int * retval = sub_6F379A30_ptr( a1, unused, a2, a3 );
	sprintf_s( msg, 100, "%X,%X,%X,%X->%X", ( int ) a1, unused, a2, a3, ( int ) retval );
	MessageBox( 0, msg, msg, 0 );
	return retval;
}
*/


HWND Warcraft3Window = 0;

typedef LRESULT( __stdcall *  WarcraftRealWNDProc )( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );
WarcraftRealWNDProc WarcraftRealWNDProc_org;
WarcraftRealWNDProc WarcraftRealWNDProc_ptr;

BOOL SkippAllMessages = TRUE;

LPARAM lpF1ScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM ) ( MapVirtualKey( VK_F1, 0 ) << 16 ));
LPARAM lpF1ScanKeyDOWN = ( LPARAM ) ( 0x00000001 | ( LPARAM ) ( MapVirtualKey( VK_F1, 0 ) << 16 ));


BOOL NeedPressKeyForHWND = FALSE;

WPARAM NeedPressMsg = 0;
WPARAM NeedPresswParam;
LPARAM NeedPresslParam;

HANDLE hPressKeyWithDelay = NULL;

BOOL PressKeyWithDelayEND = FALSE;

DWORD WINAPI PressKeyWithDelay( LPVOID )
{
	while ( true && !PressKeyWithDelayEND )
	{
		Sleep( 40 );
		if ( NeedPressKeyForHWND )
		{
			NeedPressKeyForHWND = FALSE;
			Sleep( 160 );
			if ( NeedPressMsg == 0 )
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, NeedPresswParam, NeedPresslParam );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, NeedPresswParam, ( LPARAM ) (0xC0000000 | NeedPresslParam) );
			}
			else
				WarcraftRealWNDProc_ptr( Warcraft3Window, NeedPressMsg, NeedPresswParam, NeedPresslParam );
		}

	}
	PressKeyWithDelayEND = FALSE;
	return 0;
}


LRESULT __stdcall BeforeWarcraftWNDProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
	if ( SkippAllMessages )
		return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );




	if ( *( BOOL* ) IsWindowActive && *( int* ) ChatFound == 0 )
	{
		if ( Msg == WM_KEYDOWN )
		{
			if ( wParam >= 0x41 && wParam <= 0x5A )
			{
				if ( GetSelectedUnitCountBigger( GetLocalPlayerId( ) ) == 0 )
				{
					SkippAllMessages = TRUE;
					WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
					WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );

					NeedPressMsg = 0;
					NeedPresswParam = wParam;
					NeedPresslParam = lParam;
					NeedPressKeyForHWND = TRUE;



					SkippAllMessages = FALSE;
				}
			}
		}
		if ( Msg == WM_RBUTTONDOWN )
		{
			if ( GetSelectedUnitCountBigger( GetLocalPlayerId( ) ) == 0 )
			{
				SkippAllMessages = TRUE;
				WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
				WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );

				/*	NeedPressMsg = Msg;
					NeedPresswParam = wParam;
					NeedPresslParam = lParam;
					NeedPressKeyForHWND = TRUE;
	*/


				SkippAllMessages = FALSE;
			}
		}
	}


	return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
}



__declspec( dllexport ) int __stdcall ToggleForcedSubSelection( BOOL enable )
{
	SkippAllMessages = !enable;
	return 0;
}




void InitHook( )
{
	/*sub_6F379A30_org = ( sub_6F379A30 ) ( 0x379A30 + GameDll );

	MH_CreateHook( sub_6F379A30_org, &sub_6F379A30my, reinterpret_cast< void** >( &sub_6F379A30_ptr ) );

	MH_EnableHook( sub_6F379A30_org );
	*/

	if ( Warcraft3Window )
	{
		WarcraftRealWNDProc_org = ( WarcraftRealWNDProc ) GetWindowLongA( Warcraft3Window, GWL_WNDPROC );
		if ( WarcraftRealWNDProc_org )
		{
			hPressKeyWithDelay = CreateThread( 0, 0, PressKeyWithDelay, 0, 0, 0 );
			MH_CreateHook( WarcraftRealWNDProc_org, &BeforeWarcraftWNDProc, reinterpret_cast< void** >( &WarcraftRealWNDProc_ptr ) );
			MH_EnableHook( WarcraftRealWNDProc_org );
		}
	}


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


	Storm_279_org = ( pStorm_279 ) ( (int) GetProcAddress( StormDllModule, ( LPCSTR ) 279 ) );
	MH_CreateHook( Storm_279_org, &Storm_279my, reinterpret_cast< void** >( &Storm_279_ptr ) );
	MH_EnableHook( Storm_279_org );




	pOnChatMessage_org = ( pOnChatMessage ) ( GameDll + pOnChatMessage_offset );
	MH_CreateHook( pOnChatMessage_org, &pOnChatMessage_my, reinterpret_cast< void** >( &pOnChatMessage_ptr ) );
	MH_EnableHook( pOnChatMessage_org );

}

void UninitializeHook( )
{
	if ( WarcraftRealWNDProc_org )
	{
		PressKeyWithDelayEND = TRUE;
		WaitForSingleObject( PressKeyWithDelay, 1000 );
		MH_DisableHook( WarcraftRealWNDProc_org );
	}
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

	if ( Storm_279_org )
	{
		MH_DisableHook( Storm_279_org );
		Storm_279_org = 0;
	}

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
BOOL PlantDetourJMP( BYTE* source, const BYTE* destination, size_t length )
{

	DWORD oldProtection;
	BOOL bRet = VirtualProtect( source, length, PAGE_EXECUTE_READWRITE, &oldProtection );

	if ( bRet == FALSE )
		return FALSE;

	source[ 0 ] = 0xE9;
	*( DWORD* ) ( source + 1 ) = ( DWORD ) ( destination - source ) - 5;

	for ( unsigned int i = 5; i < length; i++ )
		source[ i ] = 0x90;

	VirtualProtect( source, length, oldProtection, &oldProtection );

	return TRUE;
}




char * bufferaddr = 0;






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
	return ClassID1 == ClassID2;
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



int ReadObjectAddrFromGlobalMat( unsigned int a1, unsigned int a2 )
{
	BOOL found1;
	int result;
	int AddrType1;
	int v5;

	if ( !( a1 >> 31 ) )
	{
		if ( a1 < *( unsigned int * ) ( *( int* ) pGameClass1 + 28 ) )
		{
			found1 = *( int * ) ( *( int * ) ( *( int* ) pGameClass1 + 12 ) + 8 * a1 ) == -2;
			if ( !found1 )
				return 0;
			if ( a1 >> 31 )
			{
				AddrType1 = *( int * ) ( *( int * ) ( *( int* ) pGameClass1 + 44 ) + 8 * a1 + 4 );
				result = *( unsigned int * ) ( AddrType1 + 24 ) != a2 ? 0 : AddrType1;
			}
			else
			{
				v5 = *( int * ) ( *( int * ) ( *( int* ) pGameClass1 + 12 ) + 8 * a1 + 4 );
				result = *( unsigned int * ) ( v5 + 24 ) != a2 ? 0 : v5;
			}
			return result;
		}
		return 0;
	}
	if ( ( a1 & 0x7FFFFFFF ) >= *( unsigned int * ) ( *( int* ) pGameClass1 + 60 ) )
		return 0;
	found1 = *( int * ) ( *( int * ) ( *( int* ) pGameClass1 + 44 ) + 8 * a1 ) == -2;
	if ( !found1 )
		return 0;
	if ( a1 >> 31 )
	{
		AddrType1 = *( int * ) ( *( int * ) ( *( int* ) pGameClass1 + 44 ) + 8 * a1 + 4 );
		result = *( unsigned int * ) ( AddrType1 + 24 ) != a2 ? 0 : AddrType1;
	}
	else
	{
		v5 = *( int * ) ( *( int * ) ( *( int* ) pGameClass1 + 12 ) + 8 * a1 + 4 );
		result = *( unsigned int * ) ( v5 + 24 ) != a2 ? 0 : v5;
	}
	return result;
}


int GetObjectDataAddr( int addr )
{
	int mataddr;
	int result; // eax@3

	mataddr = ReadObjectAddrFromGlobalMat( *( unsigned int * ) addr, *( unsigned int * ) ( addr + 4 ) );

	if ( !mataddr || *(  int * ) ( mataddr + 32 ) )
		result = 0;
	else
		result = *(  int * ) ( mataddr + 84 );
	return result;
}


vector<int> ReturnAbils;

int * FindUnitAbils( int unitaddr, unsigned int * count, int abilcode = 0, int abilbasecode = 0 )
{
	if ( ReturnAbils.size( ) != 0 )
		ReturnAbils.clear( );

	int pAddr1 = unitaddr + 0x1DC;
	int pAddr2 = unitaddr + 0x1E0;

	if ( (int)(*( unsigned int * ) ( pAddr1 ) & *( unsigned int * ) ( pAddr2 )) != -1 )
	{
		int pData = GetObjectDataAddr( pAddr1 );

		while ( pData != 0 )
		{
			int pData2 = *( int* ) ( pData + 0x54 );
			if ( pData2 != 0 )
			{
				if ( abilcode != 0 && *( int* ) ( pData2 + 0x34 ) == abilcode )
				{
					if ( abilbasecode != 0 && *( int* ) ( pData2 + 0x30 ) == abilbasecode )
					{
						ReturnAbils.push_back( pData );
					}
					else if ( abilbasecode == 0 )
					{
						ReturnAbils.push_back( pData );
					}
				}
				else if ( abilcode == 0 )
				{
					if ( abilbasecode != 0 && *( int* ) ( pData2 + 0x30 ) == abilbasecode )
					{
						ReturnAbils.push_back( pData );
					}
					else if ( abilbasecode == 0 )
					{
						ReturnAbils.push_back( pData );
					}
				}
			}
			pData = GetObjectDataAddr( pData + 0x24 );
		}

		*count = ReturnAbils.size( );
		if ( *count != 0 )
		{
			return &ReturnAbils[ 0 ];
		}
	}

	return 0;
}


float __stdcall GetMagicProtectionForHero( int AmovAddr )
{
	int addr = *( int* ) ( AmovAddr + 0x30 );

	if ( addr != 0 && addr != -1 )
	{
		double indmg = 100.0;
		unsigned int abilscount = 0;
		int * abils = FindUnitAbils( addr, &abilscount, 0, 'AIdd' );
		for ( unsigned int i = 0; i < abilscount; i++ )
		{
			int pData = *( int* ) ( abils[ i ] + 0x54 );
			if ( pData != 0 )
			{
				float DmgProt = *( float* ) ( pData + 0x20 + 0x68 * ( *( int* ) ( abils[ i ] + 0x50 ) + 1 ) );
				indmg = indmg * DmgProt;
			}
		}

		return ( float ) ( 100.0 - indmg );
	}

	return 0.0f;
}

int __stdcall PrintMoveSpeed( int addr, float * movespeed, int AmovAddr )
{
	int retval = 0;
	__asm mov retval, eax;
	if ( AmovAddr )
	{
		float MagicProtection = GetMagicProtectionForHero( AmovAddr );
		bufferaddr = buffer;

		if ( MagicProtection == 0.0f )
			sprintf_s( buffer, sizeof( buffer ), "%.1f", ( *( float* ) movespeed ) );
		else
			sprintf_s( buffer, sizeof( buffer ), "%.1f\nMagic Protection: %.1f%%", ( *( float* ) movespeed ), MagicProtection );
		__asm
		{
			PUSH 0x200;
			PUSH bufferaddr;
			PUSH addr;
			CALL Storm_503;
			mov retval, eax;
		}
	}
	return retval;
}


void __declspec( naked )  PrintMoveSpeedHook126a( )
{
	__asm
	{
		mov saveeax, eax;
		mov eax, esp;
		add eax, 4;
		push ebx;
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
		push edi;
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



unsigned int         PLAYER_COLOR_RED = 0;
unsigned int         PLAYER_COLOR_BLUE = 1;
unsigned int         PLAYER_COLOR_CYAN = 2;
unsigned int         PLAYER_COLOR_PURPLE = 3;
unsigned int         PLAYER_COLOR_YELLOW = 4;
unsigned int         PLAYER_COLOR_ORANGE = 5;
unsigned int         PLAYER_COLOR_GREEN = 6;
unsigned int         PLAYER_COLOR_PINK = 7;
unsigned int         PLAYER_COLOR_LIGHT_GRAY = 8;
unsigned int         PLAYER_COLOR_LIGHT_BLUE = 9;
unsigned int         PLAYER_COLOR_AQUA = 10;
unsigned int         PLAYER_COLOR_BROWN = 11;


const char * GetPlayerColorString( int player )
{
	unsigned int c = GetPlayerColor( player );
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
			if ( itemowner > 15 || itemowner < 0 )
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
	if ( *( BOOL* ) IsWindowActive &&  IsKeyPressed( VK_LMENU ) )
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

__declspec( dllexport ) void __stdcall SetHPBarColorForPlayer( int playerid, unsigned int herocolor,
															   unsigned int unitcolor, unsigned int towercolor )
{
	if ( playerid >= 0 && playerid < 20 )
	{
		hpbarcolorsHero[ playerid ] = herocolor;
		hpbarcolorsUnit[ playerid ] = unitcolor;
		hpbarcolorsTower[ playerid ] = towercolor;
	}
}


__declspec( dllexport ) void __stdcall SetHPBarXScaleForPlayer( int playerid, float heroscale,
																float unitscale, float towerscale )
{
	if ( playerid >= 0 && playerid < 20 )
	{
		hpbarscaleHeroX[ playerid ] = heroscale;
		hpbarscaleUnitX[ playerid ] = unitscale;
		hpbarscaleTowerX[ playerid ] = towerscale;
	}
}

__declspec( dllexport ) void __stdcall SetHPBarYScaleForPlayer( int playerid, float heroscale,
																float unitscale, float towerscale )
{
	if ( playerid >= 0 && playerid < 20 )
	{
		hpbarscaleHeroY[ playerid ] = heroscale;
		hpbarscaleUnitY[ playerid ] = unitscale;
		hpbarscaleTowerY[ playerid ] = towerscale;
	}
}






int __stdcall SetColorForUnit( unsigned int  * coloraddr, BarStruct * BarStruct )
{
	int retval = 0;
	__asm mov retval, eax;
	if ( !BarStruct )
	{
		return retval;
	}

	if ( BarStruct->_BarClass != _BarVTable && BarStruct->_BarClass != ( int ) BarVtableClone )
	{
		return retval;
	}

	int unitaddr = BarStruct->unitaddr;
	if ( !unitaddr || !IsNotBadUnit( unitaddr ) )
		return retval;

	int unitslot = GetUnitOwnerSlot( unitaddr );

	if ( unitslot > 15 )
		return retval;

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
		if ( !coloraddr )
			return retval;
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
		if ( !coloraddr )
			return retval;
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
		if ( !coloraddr )
			return retval;
		if ( hpbarcolorsUnit[ unitslot ] != 0 )
		{
			*coloraddr = hpbarcolorsUnit[ unitslot ];
		}
	}


	return retval;
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
		push 0x200;
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
		push 0x200;
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
		push 0x200;
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
		push 0x200;
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
	for ( unsigned int i = 0; i < offsetslist.size( ); i++ )
	{
		if ( offsetslist[ i ].offaddr == address )
		{
			return 0;
		}
	}

	offsetdata temp;
	temp.offaddr = address;
	temp.offdata = data;
	offsetslist.push_back( temp );

	return 1;
}
#pragma endregion



void * RefreshTimerID = 0;
BOOL RefreshTimerEND = FALSE;
unsigned long __stdcall RefreshTimer( void * )
{
	while ( TRUE && !RefreshTimerEND )
	{
		// Ждать установки InGame адреса
		if ( InGame != 0 )
		{
			// Ждать входа в игру
			while ( !( *InGame ) )
			{
				Sleep( 200 );

				if ( RefreshTimerEND )
				{
					RefreshTimerEND = FALSE;
					return 0;
				}

			}

			// Ждать пока игра не закончится
			while ( *InGame )
			{
				Sleep( 200 );

				if ( RefreshTimerEND )
				{
					RefreshTimerEND = FALSE;
					return 0;
				}

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

			// Отключить ManaBar 
			ManaBarSwitch( GameDll, StormDllModule, FALSE );
		}

		Sleep( 200 );
	}
	RefreshTimerEND = FALSE;
	return 0;
}

void PatchOffset( void * addr, void * lpbuffer, unsigned int size )
{
	DWORD OldProtect1, OldProtect2;
	VirtualProtect( addr, size, PAGE_EXECUTE_READWRITE, &OldProtect1 );
	for ( unsigned int i = 0; i < size; i++ )
	{
		*( unsigned char* ) ( ( int ) addr + i ) = *( unsigned char* ) ( ( int ) lpbuffer + i );
	}

	VirtualProtect( addr, size, OldProtect1, &OldProtect2 );
}



__declspec( dllexport ) unsigned int __stdcall InitDotaHelper( int gameversion )
{
	if ( RefreshTimerID )
	{
		RefreshTimerEND = TRUE;
		WaitForSingleObject( RefreshTimerID, 1000 );
		RefreshTimerID = 0;
	}
	//RemoveMapSizeLimit( );
	GameVersion = gameversion;

	while ( mutedplayers.size( ) )
	{
		void * fMemAddr = mutedplayers.back( );
		if ( fMemAddr )
			free( fMemAddr );
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

	Warcraft3Window = 0;
	SkippAllMessages = TRUE;

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
		InGame = ( BOOL * ) (GameDll + 0xAB62A4);
		GetItemInSlotAddr = GameDll + 0x3C7730 + 0xA;
		GetItemTypeId = ( pGetItemTypeId ) ( GameDll + 0x3C4C60 );
		GetPlayerColor = ( pGetPlayerColor ) ( GameDll + 0x3C1240 );
		Player = ( pPlayer ) ( GameDll + 0x3BBB30 );
		GetPlayerName = ( p_GetPlayerName ) ( GameDll + 0x2F8F90 );
		_BarVTable = GameDll + 0x93E604;
		IsWindowActive = GameDll + 0xA9E7A4;
		ChatFound = GameDll + 0xAD15F0;

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



		pWar3Data1 = GameDll + 0xACBD40;
		pWar3Data1 = *( int* ) pWar3Data1;
		if ( pWar3Data1 )
		{
			Warcraft3Window = *( HWND* ) ( pWar3Data1 + 0x574 );
		}

		ManaBarSwitch( GameDll, StormDllModule, TRUE );


		InitHook( );
		RefreshTimerID = CreateThread( 0, 0, RefreshTimer, 0, 0, 0 );


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
		InGame = ( BOOL * ) (GameDll + 0xBE6530);
		GetItemInSlotAddr = GameDll + 0x1FAF50 + 0xC;
		GetItemTypeId = ( pGetItemTypeId ) ( GameDll + 0x1E2CC0 );
		GetPlayerColor = ( pGetPlayerColor ) ( GameDll + 0x1E3CA0 );
		Player = ( pPlayer ) ( GameDll + 0x1F1E70 );
		GetPlayerName = ( p_GetPlayerName ) ( GameDll + 0x34F730 );
		_BarVTable = GameDll + 0x98F52C;
		IsWindowActive = GameDll + 0xB673EC;
		ChatFound = GameDll + 0xBDAA14;

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


		RefreshTimerID = CreateThread( 0, 0, RefreshTimer, 0, 0, 0 );


		pWar3Data1 = GameDll + 0xBC5420;
		pWar3Data1 = *( int* ) pWar3Data1;
		if ( pWar3Data1 )
		{
			Warcraft3Window = *( HWND* ) ( pWar3Data1 + 0x574 );
		}

		ManaBarSwitch( GameDll, StormDllModule, TRUE );

		InitHook( );

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
		StormDllModule = GetModuleHandle( "Storm.dll" );
		StormDll = ( int ) StormDllModule;


		// Инициализация "перехватчика" функций
		MH_Initialize( );

		//	InitDotaHelper( 0x26a );
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		// Отключить все "перехватчики" функций
		UninitializeHook( );
		// Выгрузить "перехватчик" функций
		MH_Uninitialize( );

		// Уничтожить поток
		if ( RefreshTimerID )
		{
			RefreshTimerEND = TRUE;
			WaitForSingleObject( RefreshTimerID, 1000 );
		}
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

		// Отключить ManaBar 
		ManaBarSwitch( GameDll, StormDllModule, FALSE );
	}
	return TRUE;
}
#pragma endregion










































#pragma region Developer
//"Abso!(d3scene.ru)"
#pragma endregion