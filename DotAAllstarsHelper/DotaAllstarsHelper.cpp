
#include "Main.h"
#include "ViewAllySkill.h"


#include <stdint.h>
#include "Crc32Dynamic.h"

bool FileExist( const char * name )
{
	ifstream f( name );
	return f.good( );
}

//DWORD GameDLLsz = 0;
//DWORD StormDLLsz = 0;

//  Game.dll
int GameDll = 0;
//	Storm.dll
int StormDll = 0;
HMODULE GameDllModule = 0;
HMODULE StormDllModule = 0;
int GameVersion = 0;

#pragma region All Offsets Here



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

float * GetWindowXoffset = 0;
float * GetWindowYoffset = 0;


int StormErrorHandlerOffset = 0;
int JassNativeLookupOffset = 0;
int JassFuncLookupOffset = 0;
int ProcessNetEventsOffset = 0;
int BlizzardDebug1Offset = 0;
int BlizzardDebug2Offset = 0;
int BlizzardDebug3Offset = 0;
int BlizzardDebug4Offset = 0;
int BlizzardDebug5Offset = 0;
int BlizzardDebug6Offset = 0;
int GameFrameAtMouseStructOffset = 0;
int pTriggerExecute = 0;
int SetGameAreaFOVoffset = 0;
int GameGetFileOffset = 0;

#pragma endregion

BOOL MainFuncWork = FALSE;

void TestLog( const char * s )
{
	/*FILE *f;
	fopen_s( &f, ".\\log.txt", "a+" );
	fprintf( f, "%s\n", s );
	fclose( f );*/
}


__declspec( dllexport ) int __stdcall SetMainFuncWork( BOOL state )
{
	MainFuncWork = state;
	return 0;
}


BOOL BlockKeyAndMouseEmulation = FALSE;
BOOL EnableSelectHelper = FALSE;
BOOL ClickHelper = FALSE;


HMODULE GetCurrentModule;


_TriggerExecute TriggerExecute;

void DisplayText( char *szText, float fDuration )
{
	AddNewLineToDotaHelperLog( "DisplayText" );
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








char CurrentMapPath[ MAX_PATH ];
char NewMapPath[ MAX_PATH ];


void SaveCurrentMapPath( )
{
	AddNewLineToDotaHelperLog( "SaveCurrentMapPath" );
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
	AddNewLineToDotaHelperLog( "BuildFilePath" );
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






void InitHook( )
{


	TestLog( "InitHook" );
	AddNewLineToDotaHelperLog( "InitHook" );
	StormErrorHandler_org = ( StormErrorHandler ) StormErrorHandlerOffset;
	LookupNative_org = ( LookupNative ) JassNativeLookupOffset;
	LookupJassFunc_org = ( LookupJassFunc ) JassFuncLookupOffset;
	ProcessNetEvents_org = ( ProcessNetEvents ) ProcessNetEventsOffset;
	BlizzardDebug1_org = ( BlizzardDebug1 ) BlizzardDebug1Offset;
	BlizzardDebug2_org = ( BlizzardDebug2 ) BlizzardDebug2Offset;
	BlizzardDebug3_org = ( BlizzardDebug3 ) BlizzardDebug3Offset;
	BlizzardDebug4_org = ( BlizzardDebug4 ) BlizzardDebug4Offset;
	BlizzardDebug5_org = ( BlizzardDebug5 ) BlizzardDebug5Offset;
	BlizzardDebug6_org = ( BlizzardDebug6 ) BlizzardDebug6Offset;

	EnableErrorHandler( );


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



	GameGetFile_org = ( GameGetFile ) ( GameDll + GameGetFileOffset );
	MH_CreateHook( GameGetFile_org, &GameGetFile_my, reinterpret_cast< void** >( &GameGetFile_ptr ) );
	MH_EnableHook( GameGetFile_org );


	pOnChatMessage_org = ( pOnChatMessage ) ( GameDll + pOnChatMessage_offset );
	MH_CreateHook( pOnChatMessage_org, &pOnChatMessage_my, reinterpret_cast< void** >( &pOnChatMessage_ptr ) );
	MH_EnableHook( pOnChatMessage_org );
	IssueFixerInit( );

	TestLog( "InitHook end" );
}

void UninitializeHook( )
{
	AddNewLineToDotaHelperLog( "UninitializeHook" );
	DisableErrorHandler( );


#pragma region Game.dll hook


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

	if ( GameGetFile_org )
	{
		MH_DisableHook( GameGetFile_org );
		GameGetFile_org = 0;
	}


#pragma endregion


#pragma region Storm.dll hook

	if ( GetModuleHandle( "Storm.dll" ) != 0 )
	{

	}

#pragma endregion

	IssueFixerDisable( );
}



char buffer[ 4096 ];

typedef int( __stdcall * pStorm_503 )( int a1, int a2, int a3 );
pStorm_503 Storm_503;

BOOL PlantDetourJMP( BYTE* source, const BYTE* destination, size_t length )
{
	AddNewLineToDotaHelperLog( "PlantDetourJMP" );
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
	AddNewLineToDotaHelperLog( "GetItemTypeInSlot" );
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

int GetTypeId( int unit_item_abil_etc_addr )
{
	if ( unit_item_abil_etc_addr )
		return *( int* ) ( unit_item_abil_etc_addr + 0x30 );
	else
		return 0;
}

BOOL IsClassEqual( int ClassID1, int ClassID2 )
{
	return ClassID1 == ClassID2;
}

// Функция принимает данные о скорости атаки (и о увеличении урона от способностей) и сохраняет в буфер который будет использоваться при отрисовке
int __stdcall PrintAttackSpeedAndOtherInfo( int addr, float * attackspeed, float * BAT, int * unitaddr )
{
	int retval = 0;
	__asm mov retval, eax;
	if ( unitaddr > 0 )
	{
		AddNewLineToDotaHelperLog( "PrintAttackSpeedAndOtherInfo" );
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
		else
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

	return retval;
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

	if ( !mataddr || *( int * ) ( mataddr + 32 ) )
		result = 0;
	else
		result = *( int * ) ( mataddr + 84 );
	return result;
}


vector<int> ReturnAbils;

int * FindUnitAbils( int unitaddr, unsigned int * count, int abilcode = 0, int abilbasecode = 0 )
{
	if ( ReturnAbils.size( ) != 0 )
		ReturnAbils.clear( );
	*count = 0;
	if ( unitaddr > 0 )
	{
		AddNewLineToDotaHelperLog( "FindUnitAbils" );
		int pAddr1 = unitaddr + 0x1DC;
		int pAddr2 = unitaddr + 0x1E0;

		if ( ( int ) ( *( unsigned int * ) ( pAddr1 ) & *( unsigned int * ) ( pAddr2 ) ) != -1 )
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
		}
	}

	return &ReturnAbils[ 0 ];
}


float __stdcall GetMagicProtectionForHero( int AmovAddr )
{
	int addr = *( int* ) ( AmovAddr + 0x30 );

	if ( addr != 0 && addr != -1 )
	{
		AddNewLineToDotaHelperLog( "GetMagicProtectionForHero" );
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
	if ( AmovAddr > 0 )
	{
		AddNewLineToDotaHelperLog( "PrintMoveSpeed" );
		float MagicProtection = GetMagicProtectionForHero( AmovAddr );
		bufferaddr = buffer;

		if ( MagicProtection == 0.0f )
			sprintf_s( buffer, sizeof( buffer ), "%.1f", ( *( float* ) movespeed ) );
		else if ( MagicProtection > 30.0f )
			sprintf_s( buffer, sizeof( buffer ), "%.1f\nMagic Protection: |cFF00C800%.1f|r%%", ( *( float* ) movespeed ), MagicProtection );
		else if ( MagicProtection <= 30.0f && MagicProtection > 0.0f )
			sprintf_s( buffer, sizeof( buffer ), "%.1f\nMagic Protection: %.1f%%", ( *( float* ) movespeed ), MagicProtection );
		else
			sprintf_s( buffer, sizeof( buffer ), "%.1f\nMagic Protection: |cFFD82005%.1f|r%%", ( *( float* ) movespeed ), MagicProtection );
		__asm
		{
			PUSH 0x200;
			PUSH bufferaddr;
			PUSH addr;
			CALL Storm_503;
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
		AddNewLineToDotaHelperLog( "SaveStringsForPrintItem" );
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
	if ( unitaddr > 0 )
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
	}
	return 0;
}


float GetUnitHPregen( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xA0 );
		if ( offset1 )
		{

			result = *( float* ) ( offset1 + 0x7C );
		}
	}
	return result;
}

float GetUnitMPregen( int unitaddr )
{
	float result = 0.0f;
	if ( unitaddr > 0 )
	{
		int offset1 = GetUnitAddressFloatsRelated( unitaddr, 0xC0 );
		if ( offset1 )
		{
			result = *( float* ) ( offset1 + 0x7C );
		}
	}
	return result;
}


int __stdcall SaveStringForHP_MP( int unitaddr )
{
	if ( *( BOOL* ) IsWindowActive &&  IsKeyPressed( VK_LMENU ) )
	{
		AddNewLineToDotaHelperLog( "SaveStringForHP_MP" );
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
			return unitaddr;
		}
	}
	sprintf_s( unitstr1, 128, "%%u / %%u" );
	sprintf_s( unitstr2, 128, "%%u / %%u" );
	return unitaddr;
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


vector<LPVOID> FreeExecutableMemoryList;

__declspec( dllexport ) int __stdcall FreeExecutableMemory( void * addr )
{
	FreeExecutableMemoryList.push_back( addr );
	return 0;
}

void __stdcall RestoreAllOffsets( )
{
	TestLog( "RestoreAllOffsets" );
	for ( UINT i = 0; i < offsetslist.size( ); i++ )
	{
		offsetdata temp = offsetslist[ i ];
		DWORD oldprotect, oldprotect2;
		if ( VirtualProtect( ( void* ) temp.offaddr, 4, PAGE_EXECUTE_READWRITE, &oldprotect ) )
		{
			*( int* ) temp.offaddr = temp.offdata;
			VirtualProtect( ( void* ) temp.offaddr, 4, oldprotect, &oldprotect2 );
		}
	}
	offsetslist.clear( );
}

void __stdcall ClearCustomsBars( )
{
	TestLog( "ClearCustomsBars" );
	for ( int i = 0; i < 20; i++ )
	{
		if ( !CustomHPBarList[ i ].empty( ) )
			CustomHPBarList[ i ].clear( );
	}
}

void __stdcall FreeAllVectors( )
{
	TestLog( "FreeAllVectors" );
	if ( !ModelCollisionFixList.empty( ) )
		ModelCollisionFixList.clear( );
	if ( !ModelTextureFixList.empty( ) )
		ModelTextureFixList.clear( );
	if ( !ModelPatchList.empty( ) )
		ModelPatchList.clear( );
	if ( !ModelRemoveTagList.empty( ) )
		ModelRemoveTagList.clear( );
	if ( !ModelSequenceReSpeedList.empty( ) )
		ModelSequenceReSpeedList.clear( );
	if ( !ModelSequenceValueList.empty( ) )
		ModelSequenceValueList.clear( );
}

void __stdcall UnloadHWNDHandler( BOOL Force = FALSE )
{
	if ( WarcraftRealWNDProc_org )
	{
		SkipAllMessages = TRUE;
		PressKeyWithDelayEND = TRUE;
		WaitForSingleObject( hPressKeyWithDelay, 2000 );
		PressKeyWithDelayEND = FALSE;
		if ( !Force )
			MH_DisableHook( WarcraftRealWNDProc_org );
		SkipAllMessages = FALSE;
	}
}

void __stdcall DisableAllHooks( )
{
	TestLog( "DisableAllHooks" );
	// Выгрузить перехватчики функций
	UnloadHWNDHandler( );
	UninitializeHook( );
	// Отключить мут
	UnMutePlayer( 0 );
	// Убрать все патчи и вернуть стандартные данные
	RestoreAllOffsets( );
	// Очистить список кастом баров
	ClearCustomsBars( );
	// Отключить ManaBar 
	ManaBarSwitch( GameDll, FALSE );

	for ( LPVOID lpAddr : FreeExecutableMemoryList )
		VirtualFree( lpAddr, 0, MEM_RELEASE );
	FreeExecutableMemoryList.clear( );

	FreeAllIHelpers( );
	FreeAllVectors( );

	KeyboardHaveTriggerEvent = FALSE;
}

void * hRefreshTimer = 0;
BOOL RefreshTimerEND = FALSE;
unsigned long __stdcall RefreshTimer( void * )
{
	while ( TRUE && !RefreshTimerEND )
	{
		// Ждать установки InGame адреса
		if ( InGame != 0 )
		{
			TestLog( "RefreshTimer" );
			AddNewLineToDotaHelperLog( "RefreshTimer" );
			// Ждать входа в игру
			while ( !( *InGame ) )
			{
				Sleep( 200 );

				if ( RefreshTimerEND )
				{
					return 0;
				}
			}

			// Ждать пока игра не закончится
			while ( *InGame )
			{
				Sleep( 200 );

				if ( RefreshTimerEND )
				{
					return 0;
				}
			}

			DisableAllHooks( );
			TestLog( "RefreshTimerEnd" );
			AddNewLineToDotaHelperLog( "RefreshTimerEnd" );

		}
		Sleep( 200 );
	}
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
	TestLog( "InitDotaHelper" );
	AddNewLineToDotaHelperLog( "InitDotaHelper" );
	if ( hRefreshTimer )
	{
		RefreshTimerEND = TRUE;
		WaitForSingleObject( hRefreshTimer, 2000 );
		RefreshTimerEND = FALSE;
		hRefreshTimer = 0;
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
	EnableSelectHelper = FALSE;
	BlockKeyAndMouseEmulation = FALSE;
	SetWidescreenFixState( FALSE );
	MainFuncWork = FALSE;
	SetCustomFovFix( 1.0f );

	if ( gameversion == 0x26a )
	{
		UninitializeHook( );

		FreeAllIHelpers( );
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
		InGame = ( BOOL * ) ( GameDll + 0xAB62A4 );
		GetItemInSlotAddr = GameDll + 0x3C7730 + 0xA;
		GetItemTypeId = ( pGetItemTypeId ) ( GameDll + 0x3C4C60 );
		GetPlayerColor = ( pGetPlayerColor ) ( GameDll + 0x3C1240 );
		Player = ( pPlayer ) ( GameDll + 0x3BBB30 );
		GetPlayerName = ( p_GetPlayerName ) ( GameDll + 0x2F8F90 );
		_BarVTable = GameDll + 0x93E604;
		IsWindowActive = GameDll + 0xA9E7A4;
		ChatFound = GameDll + 0xAD15F0;
		TriggerExecute = ( _TriggerExecute ) ( GameDll + 0x3C3F40 );
		StormErrorHandlerOffset = StormDll + 0x28F0;
		JassNativeLookupOffset = GameDll + 0x44EA00;
		JassFuncLookupOffset = GameDll + 0x45AE80;
		ProcessNetEventsOffset = GameDll + 0x551D80;
		BlizzardDebug1Offset = GameDll + 0x7AEC10;
		BlizzardDebug2Offset = GameDll + 0x537e80;
		BlizzardDebug3Offset = GameDll + 0x20BC60;
		BlizzardDebug4Offset = GameDll + 0x41F5E0;
		BlizzardDebug5Offset = GameDll + 0x39E970;
		BlizzardDebug6Offset = GameDll + 0x579C10;

		OriginFilter = ( LPTOP_LEVEL_EXCEPTION_FILTER ) ( StormDll + 0x16880 );

		IssueWithoutTargetOrderOffset = 0x339C60;
		IssueTargetOrPointOrder2Offset = 0x339CC0;
		sub_6F339D50Offset = 0x339D50;
		IssueTargetOrPointOrderOffset = 0x339DD0;
		sub_6F339E60Offset = 0x339E60;
		sub_6F339F00Offset = 0x339F00;
		sub_6F339F80Offset = 0x339F80;
		sub_6F33A010Offset = 0x33A010;

		GameGetFileOffset = 0x4C1550;

		GameFrameAtMouseStructOffset = GameDll + 0xA9A444;

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



		GetWindowXoffset = ( float * ) ( GameDll + 0xADE91C );
		GetWindowYoffset = ( float * ) ( GameDll + 0xADE918 );

		SetGameAreaFOVoffset = 0x7B66F0;


		hRefreshTimer = CreateThread( 0, 0, RefreshTimer, 0, 0, 0 );


		pWar3Data1 = GameDll + 0xACBD40;
		pWar3Data1 = *( int* ) pWar3Data1;
		Warcraft3Window = *( HWND* ) ( GameDll + 0xAD1398 );


		ManaBarSwitch( GameDll, TRUE );

		InitHook( );

		/* crc32 simple protection */
		char outfilename[ MAX_PATH ];
		GetModuleFileName( GetCurrentModule, outfilename, MAX_PATH );
		DWORD dwCrc32;
		CCrc32Dynamic *pobCrc32Dynamic = new CCrc32Dynamic;
		pobCrc32Dynamic->Init( );
		pobCrc32Dynamic->FileCrc32Assembly( outfilename, dwCrc32 );
		pobCrc32Dynamic->Free( );
		delete pobCrc32Dynamic;
		AddNewLineToDotaHelperLog( "InitEnd" );
		TestLog( "InitEnd" );
		return dwCrc32;
	}
	else if ( gameversion == 0x27a )
	{
		UninitializeHook( );

		FreeAllIHelpers( );
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
		InGame = ( BOOL * ) ( GameDll + 0xBE6530 );
		GetItemInSlotAddr = GameDll + 0x1FAF50 + 0xC;
		GetItemTypeId = ( pGetItemTypeId ) ( GameDll + 0x1E2CC0 );
		GetPlayerColor = ( pGetPlayerColor ) ( GameDll + 0x1E3CA0 );
		Player = ( pPlayer ) ( GameDll + 0x1F1E70 );
		GetPlayerName = ( p_GetPlayerName ) ( GameDll + 0x34F730 );
		_BarVTable = GameDll + 0x98F52C;
		IsWindowActive = GameDll + 0xB673EC;
		ChatFound = GameDll + 0xBDAA14;
		TriggerExecute = ( _TriggerExecute ) ( GameDll + 0x1F9100 );
		StormErrorHandlerOffset = StormDll + 0x8230;
		JassNativeLookupOffset = GameDll + 0x7E2FE0;
		JassFuncLookupOffset = GameDll + 0x7EFBB0;
		ProcessNetEventsOffset = GameDll + 0x3098D0;
		BlizzardDebug1Offset = GameDll + 0x179f20;
		BlizzardDebug2Offset = GameDll + 0x30F990;
		BlizzardDebug3Offset = GameDll + 0x520f0;
		BlizzardDebug4Offset = GameDll + 0x273290;
		BlizzardDebug5Offset = GameDll + 0x1c14f0;
		BlizzardDebug6Offset = GameDll + 0x2eeb70;

		OriginFilter = ( LPTOP_LEVEL_EXCEPTION_FILTER ) ( StormDll + 0x1BB60 );

		IssueWithoutTargetOrderOffset = 0x3AE4E0;
		IssueTargetOrPointOrder2Offset = 0x3AE540;
		sub_6F339D50Offset = 0x3AE810;
		IssueTargetOrPointOrderOffset = 0x3AE660;
		sub_6F339E60Offset = 0x3AE5D0;
		sub_6F339F00Offset = 0x3AE880;
		sub_6F339F80Offset = 0x3AE6F0;
		sub_6F33A010Offset = 0x3AE780;



		GameGetFileOffset = 0x048C10;

		GameFrameAtMouseStructOffset = GameDll + 0xB66318;



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



		GetWindowXoffset = ( float * ) ( GameDll + 0xBBA22C );
		GetWindowYoffset = ( float * ) ( GameDll + 0xBBA228 );

		SetGameAreaFOVoffset = 0xD31D0;


		hRefreshTimer = CreateThread( 0, 0, RefreshTimer, 0, 0, 0 );


		pWar3Data1 = GameDll + 0xBC5420;
		pWar3Data1 = *( int* ) pWar3Data1;
		Warcraft3Window = *( HWND* ) ( GameDll + 0xBDA9CC );


		ManaBarSwitch( GameDll, TRUE );

		InitHook( );

		/* crc32 simple protection */
		char outfilename[ MAX_PATH ];
		GetModuleFileName( GetCurrentModule, outfilename, MAX_PATH );
		DWORD dwCrc32;
		CCrc32Dynamic *pobCrc32Dynamic = new CCrc32Dynamic;
		pobCrc32Dynamic->Init( );
		pobCrc32Dynamic->FileCrc32Assembly( outfilename, dwCrc32 );
		pobCrc32Dynamic->Free( );
		delete pobCrc32Dynamic;

		TestLog( "InitEnd" );
		AddNewLineToDotaHelperLog( "InitEnd" );
		return dwCrc32;
	}



	AddNewLineToDotaHelperLog( "InitBadVersion" );

	return 0;
}

Ordinal403 Storm_403_org = NULL;

const char * GameDllName = "Game.dll";
const char * StormDllName = "Storm.dll";

__declspec( dllexport ) int __stdcall SetCustomGameDLLandStormDLL( const char * _GameDllName, const char * _StormDllName )
{
	GameDllModule = GetModuleHandle( _GameDllName );
	GameDll = ( int ) GameDllModule;

	StormDllModule = GetModuleHandle( _StormDllName );
	StormDll = ( int ) StormDllModule;

	if ( StormDllModule && GameDllModule )
	{
		//	MODULEINFO modinfo;

		//GetModuleInformation( GetCurrentProcess( ), GameDllModule, &modinfo, sizeof( MODULEINFO ) );
		//GameDLLsz = modinfo.SizeOfImage;

		//GetModuleInformation( GetCurrentProcess( ), StormDllModule, &modinfo, sizeof( MODULEINFO ) );
		//StormDLLsz = modinfo.SizeOfImage;

		Storm_401_org = ( Storm_401 ) ( int ) GetProcAddress( StormDllModule, ( LPCSTR ) 401 );
		Storm_403_org = ( Ordinal403 ) ( int ) GetProcAddress( StormDllModule, ( LPCSTR ) 403 );

		// Инициализация "перехватчика" функций
		MH_Initialize( );

	}
	return 0;
}



#pragma region Main
BOOL __stdcall DllMain( HINSTANCE Module, UINT reason, LPVOID )
{
	GetCurrentModule = Module;
	if ( reason == DLL_PROCESS_ATTACH )
	{

		GameDllModule = GetModuleHandle( GameDllName );
		GameDll = ( int ) GameDllModule;
		DisableThreadLibraryCalls( Module );
		StormDllModule = GetModuleHandle( StormDllName );
		StormDll = ( int ) StormDllModule;

		if ( StormDllModule && GameDllModule )
		{
			//	MODULEINFO modinfo;

				//GetModuleInformation( GetCurrentProcess( ), GameDllModule, &modinfo, sizeof( MODULEINFO ) );
				//GameDLLsz = modinfo.SizeOfImage;

				//GetModuleInformation( GetCurrentProcess( ), StormDllModule, &modinfo, sizeof( MODULEINFO ) );
				//StormDLLsz = modinfo.SizeOfImage;

			Storm_401_org = ( Storm_401 ) ( int ) GetProcAddress( StormDllModule, ( LPCSTR ) 401 );
			Storm_403_org = ( Ordinal403 ) ( int ) GetProcAddress( StormDllModule, ( LPCSTR ) 403 );

			// Инициализация "перехватчика" функций
			MH_Initialize( );

		}
		//MessageBox( 0, "Init for 1.26a", "ok", MB_OK );
		//InitDotaHelper( 0x26a );

	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		if ( !GetModuleHandle( GameDllName ) || !GetModuleHandle( StormDllName ) )
		{
			ExitProcess( 0 );
		}
		// Cleanup
		if ( hRefreshTimer )
		{
			RefreshTimerEND = TRUE;
			WaitForSingleObject( hRefreshTimer, 2000 );
			RefreshTimerEND = FALSE;
			hRefreshTimer = 0;
		}
		if ( !GetModuleHandle( GameDllName ) || !GetModuleHandle( StormDllName ) )
		{
			ExitProcess( 0 );
		}
		UnloadHWNDHandler( TRUE );
		if ( !GetModuleHandle( GameDllName ) || !GetModuleHandle( StormDllName ) )
		{
			ExitProcess( 0 );
		}
		ClearCustomsBars( );
		FreeAllVectors( );
		FreeAllIHelpers( );
		KeyboardHaveTriggerEvent = FALSE;
		RestoreAllOffsets( );
		UnMutePlayer( 0 );
		for( LPVOID lpAddr : FreeExecutableMemoryList )
			VirtualFree( lpAddr, 0, MEM_RELEASE );
		FreeExecutableMemoryList.clear( );
		ManaBarSwitch( GameDll, FALSE );
		MH_Uninitialize( );


	}
	return TRUE;
}
#pragma endregion










































#pragma region Developer
//"Abso!(d3scene.ru)"
#pragma endregion