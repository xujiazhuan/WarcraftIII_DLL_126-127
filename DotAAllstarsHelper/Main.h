#pragma once
//#define DOTA_HELPER_LOG

#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
//#define PSAPI_VERSION 1



#pragma region Includes

#pragma comment(lib,"legacy_stdio_definitions.lib")
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <csetjmp>
#include <ctime>
#include <Windows.h>
#include <string>
#include <vector>

#include <concurrent_vector.h>
#define safevector concurrency::concurrent_vector

#include <fstream> 
#include <iostream>
#include <sstream>
#include <algorithm>
#include <MinHook.h>
#include <thread>
#include <chrono>
#include <map>


#include <filesystem>
namespace fs = std::experimental::filesystem;


using namespace std;
#pragma comment(lib, "libMinHook.x86.lib")
#include <winsock2.h>
#pragma comment (lib, "Ws2_32.lib")
#include "buffer.h"
#ifdef DOTA_HELPER_LOG

#define PSAPI_VERSION 1
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

#endif
#define MASK_56 (((u_int64_t)1<<56)-1) /* i.e., (u_int64_t)0xffffffffffffff */

#include "fnv.h"


#include "Structures.h"

#include "Storm.h"

#include "WarcraftFrameHelper.h"

#include "MegaPacketHandler.h"

#pragma endregion




#define IsKeyPressed(CODE) ((GetAsyncKeyState(CODE) & 0x8000) > 0)





extern BOOL InitFunctionCalled;

extern BOOL TerminateStarted;
extern BOOL IsVEHex;
extern BOOL TestModeActivated;

void FrameDefHelperUninitialize( );
void FrameDefHelperInitialize( );


extern int RenderStage;

int GetGlobalClassAddr( );
BOOL FileExist( const char * name );
DWORD GetDllCrc32( );
typedef void *( __cdecl * _TriggerExecute )( int TriggerHandle );
extern _TriggerExecute TriggerExecute;



typedef void( __cdecl * pExecuteFunc )( RCString * funcname );
extern pExecuteFunc ExecuteFunc;
typedef void( __thiscall * pConvertStrToJassStr )( RCString * jStr, const char * cStr );
extern pConvertStrToJassStr str2jstr;


BOOL IsClassEqual( int ClassID1, int ClassID2 );
int GetTypeId( int unit_item_abil_etc_addr );
int __stdcall Wc3MessageBox( const char * message, int type );
typedef void( __fastcall * pGame_Wc3MessageBox ) ( int type, const char * text, BOOL IsUsedCallBack, int callbackaddr, int unk2, int unk3, int unk4 );
extern pGame_Wc3MessageBox Game_Wc3MessageBox;
typedef void( __fastcall * pLoadFrameDefList )( const char * filepath, int env );
extern pLoadFrameDefList LoadFrameDefList;

inline string ToLower( string str )
{
	std::transform( str.begin( ), str.end( ), str.begin( ), tolower );
	return str;
}

extern BOOL MainFuncWork;

char *  GetWar3Preferense( int ID );
extern char MyFpsString[ 512 ];

#pragma region DotaPlayerHelper.cpp

BOOL IsPlayerEnemy( int hPlayer1, int hPlayer2 );
extern std::map<std::pair<int, int>, BOOL> PlayerEnemyCache;

extern int playercache[ 16 ];
extern int player_real_cache[ 16 ];
extern BOOL player_observers[ 16 ];
extern int player_local_id;

typedef BOOL( __cdecl * pGetPlayerAlliance )( unsigned int hPlayer1, unsigned int hPlayer2, int hAlliancetype );
extern pGetPlayerAlliance GetPlayerAlliance;
typedef unsigned int( __cdecl * pGetPlayerColor )( unsigned int whichPlayer );
extern pGetPlayerColor GetPlayerColor2;
typedef int( __cdecl * pPlayer )( int number );
extern pPlayer _Player;
int __stdcall Player( int number );
typedef int( __cdecl * pIsPlayerObs )( unsigned int hPlayer );
extern pIsPlayerObs IsPlayerObs;


int GetLocalPlayerId( );
int _GetLocalPlayerId( );
int GetPlayerByNumber( int number );
int _GetPlayerByNumber( int number );
// Проверить являются ли игроки врагами
typedef int( __cdecl * IsPlayerEnemy_org )( unsigned int Player1, unsigned int Player2 );
// Получить игрока по ID
typedef unsigned int( __cdecl * GetPlayerByID )( int PlayerId );
typedef char *( __fastcall * p_GetPlayerName )( int a1, int a2 );
extern p_GetPlayerName GetPlayerName;
int __stdcall MutePlayer( const char * str );
int __stdcall UnMutePlayer( const char * str );
typedef void( __fastcall * pOnChatMessage )( int a1, int unused, int PlayerID, char * message, int a4, float a5 );
void __fastcall pOnChatMessage_my( int a1, int unused, int PlayerID, char * message, int a4, float a5 );
extern vector<char *> mutedplayers;
//sub_6F2FB480
extern pOnChatMessage pOnChatMessage_org, pOnChatMessage_ptr;
BOOL IsPlayerObserver( int pid );
BOOL _IsPlayerObserver( int pid );
BOOL IsLocalPlayerObserver( );
extern BOOL ShowSkillPanelForObservers;
extern BOOL ShowSkillPanelOnlyForHeroes;
#pragma endregion

#pragma region UnitAndItem.cpp
int * FindUnitAbils( int unitaddr, unsigned int * count, int abilcode = 0, int abilbasecode = 0 );
int __stdcall GetUnitOwnerSlot( int unitaddr );
BOOL __stdcall IsEnemy( int UnitAddr );
BOOL __stdcall IsHero( int unitaddr );
BOOL __stdcall IsTower( int unitaddr );
BOOL __stdcall IsNotBadUnit( int unitaddr, BOOL onlymem = FALSE );
BOOL __stdcall IsUnitInvulnerable( int unitaddr );
BOOL __stdcall IsUnitIllusion( int unitaddr );
BOOL __stdcall IsNotBadItem( int itemaddr, BOOL extracheck = FALSE );
typedef int( __fastcall * pGetHeroInt )( int unitaddr, int unused, BOOL withbonus );
extern pGetHeroInt GetHeroInt;
int GetSelectedUnitCountBigger( int slot );
int GetSelectedUnit( int slot );
void GetItemLocation2D( int itemaddr, float * x, float * y );
void GetUnitLocation2D( int unitaddr, float * x, float * y );
int * GetUnitCountAndUnitArray( int ** unitarray );
int * GetItemCountAndItemArray( int ** itemarray );
float GetUnitMPregen( int unitaddr );
float GetUnitHPregen( int unitaddr );
int GetUnitAddressFloatsRelated( int unitaddr, int step );
float GetUnitX_real( int unitaddr );
float GetUnitY_real( int unitaddr );
#pragma endregion

#pragma region DotaMPBarHelper.cpp

extern BYTE BarVtableClone[ 0x80 ];
void ManaBarSwitch( BOOL b );
void PatchOffset( void * addr, void * buffer, unsigned int size );
PBYTE HookVTableFunction( PDWORD* dwVTable, PBYTE dwHook, INT Index );
PBYTE GetVTableFunction( PDWORD* dwVTable, INT Index );

int __stdcall SetColorForUnit( unsigned int  * coloraddr, BarStruct * BarStruct );
#pragma endregion


#pragma region DotaHPBarHelper.cpp
extern unsigned int hpbarcolorsHero[ 20 ];
extern unsigned int hpbarcolorsUnit[ 20 ];
extern unsigned int hpbarcolorsTower[ 20 ];
extern float hpbarscaleHeroX[ 20 ];
extern float hpbarscaleUnitX[ 20 ];
extern float hpbarscaleTowerX[ 20 ];
extern float hpbarscaleHeroY[ 20 ];
extern float hpbarscaleUnitY[ 20 ];
extern float hpbarscaleTowerY[ 20 ];
extern vector<CustomHPBar> CustomHPBarList[ 20 ];
#pragma endregion

#pragma region ErrorHandler.cpp
#ifdef DOTA_HELPER_LOG
void  __stdcall AddNewLineToJassLog( const char * s );
void __stdcall  AddNewLineToDotaChatLog( const char * s );
void __stdcall  AddNewLineToDotaHelperLog( const char * s, int line );
void __stdcall  AddNewLineToJassNativesLog( const char * s );
void __stdcall EnableErrorHandler( int );
void __stdcall DisableErrorHandler( int );
void DumpExceptionInfoToFile( _EXCEPTION_POINTERS *ExceptionInfo );

extern void ResetTopLevelExceptionFilter( );
extern LPTOP_LEVEL_EXCEPTION_FILTER OriginFilter;
extern BOOL bDllLogEnable;
typedef LONG( __fastcall * StormErrorHandler )( int a1, void( *a2 )( int, const char *, ... ), int a3, BYTE *a4, LPSYSTEMTIME a5 );
extern StormErrorHandler StormErrorHandler_org;
typedef int( __fastcall *LookupNative )( int global, int unused, const char * name );
extern LookupNative LookupNative_org;
typedef signed int( __fastcall * LookupJassFunc )( int global, int unused, const char * funcname );
extern LookupJassFunc LookupJassFunc_org;
typedef void( __fastcall * ProcessNetEvents )( void * data, int unused_, int Event );
extern ProcessNetEvents ProcessNetEvents_org;
typedef void( __fastcall * BlizzardDebug1 ) ( const char*str );
extern BlizzardDebug1 BlizzardDebug1_org;
typedef void( __cdecl * BlizzardDebug2 )( const char * src, int lineid, const char * classname );
extern BlizzardDebug2 BlizzardDebug2_org;
typedef void( __cdecl * BlizzardDebug3 )( const char *format, ... );
extern BlizzardDebug3 BlizzardDebug3_org;
typedef void( __cdecl * BlizzardDebug4 )( BOOL type1, const char *format, ... );
extern BlizzardDebug4 BlizzardDebug4_org;
typedef void( __cdecl * BlizzardDebug5 )( const char *format, ... );
extern BlizzardDebug5 BlizzardDebug5_org;
typedef void( __cdecl * BlizzardDebug6 )( const char *format, ... );
extern BlizzardDebug6 BlizzardDebug6_org;
#endif
#pragma endregion


//  Game.dll
extern int GameDll;
//	Storm.dll
extern int StormDll;
extern HMODULE GameDllModule;
extern HMODULE StormDllModule;
extern int GameVersion;
extern HWND Warcraft3Window;

#pragma region All Offsets Here
extern int GlobalPlayerOffset;
extern int IsPlayerEnemyOffset;
extern int DrawSkillPanelOffset;
extern int DrawSkillPanelOverlayOffset;
extern int IsDrawSkillPanelOffset;
extern int IsDrawSkillPanelOverlayOffset;
extern int IsNeedDrawUnitOriginOffset;
extern int IsNeedDrawUnit2offset;
extern int IsNeedDrawUnit2offsetRetAddress;
extern BOOL * InGame;
extern BOOL * IsWindowActive;
extern int ChatFound;
extern int pW3XGlobalClass;
extern int pGameClass1;
extern int pWar3GlobalData1;
extern int UnitVtable;
extern int ItemVtable;
extern int pPrintText2;
extern void PrintText( const char * text, float staytime = 10.0f );
extern int MapNameOffset1;
extern int MapNameOffset2;
extern int pOnChatMessage_offset;
extern int _BarVTable;
extern int pAttackSpeedLimit;
extern int GetItemInSlotAddr;
extern float * GetWindowXoffset;
extern float * GetWindowYoffset;
extern int GameFrameAtMouseStructOffset;
extern int pTriggerExecute;
#pragma endregion



#pragma region DotaClickHelper.cpp
extern vector<int> doubleclickSkillIDs;
extern vector<int> WhiteListForTeleport;
extern BOOL ShopHelperEnabled;
extern BOOL TeleportShiftPress;
extern BOOL BlockKeyAndMouseEmulation;
extern BOOL EnableSelectHelper;
extern BOOL ClickHelper;
extern BOOL AutoSelectHero;
extern BOOL LOCK_MOUSE_IN_WINDOW;
extern BOOL BlockKeyboardAndMouseWhenTeleport;
extern BOOL rawimage_skipmouseevent;
typedef LRESULT( __fastcall *  WarcraftRealWNDProc )( HWND hWnd, unsigned int Msg, WPARAM wParam, LPARAM lParam );
extern WarcraftRealWNDProc WarcraftRealWNDProc_org, WarcraftRealWNDProc_ptr;
LRESULT __fastcall BeforeWarcraftWNDProc( HWND hWnd, unsigned int Msg, WPARAM _wParam, LPARAM lParam );
extern int ShiftPressed;
extern BOOL SkipAllMessages;
extern int IssueWithoutTargetOrderOffset;
extern int IssueTargetOrPointOrder2Offset;
extern int sub_6F339D50Offset;
extern int IssueTargetOrPointOrderOffset;
extern int sub_6F339E60Offset;
extern int sub_6F339F00Offset;
extern int sub_6F339F80Offset;
extern int sub_6F33A010Offset;
void IssueFixerInit( );
void IssueFixerDisable( );
typedef int( __fastcall * c_SimpleButtonClickEvent )( int pButton, int unused, int ClickEventType );
int __fastcall SimpleButtonClickEvent_my( int pButton, int unused, int ClickEventType );
extern c_SimpleButtonClickEvent SimpleButtonClickEvent_org;
extern c_SimpleButtonClickEvent SimpleButtonClickEvent_ptr;
extern int CommandButtonVtable;
extern std::vector<ClickPortrainForId> ClickPortrainForIdList;
#pragma endregion


#pragma region DotaFilesHelper.cpp
extern vector<ModelCollisionFixStruct> ModelCollisionFixList;
extern vector<ModelTextureFixStruct> ModelTextureFixList;
extern vector<ModelPatchStruct> ModelPatchList;
extern vector<ModelRemoveTagStruct> ModelRemoveTagList;
extern vector<ModelSequenceReSpeedStruct> ModelSequenceReSpeedList;
extern vector<ModelSequenceValueStruct> ModelSequenceValueList;
extern vector<ModelScaleStruct> ModelScaleList;

extern vector<ICONMDLCACHE> ICONMDLCACHELIST;

extern vector<FileRedirectStruct> FileRedirectList;
typedef BOOL( __fastcall * GameGetFile )( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown );
BOOL __fastcall GameGetFile_my( const char * filename, int * OutDataPointer, unsigned int * OutSize, BOOL unknown );
extern GameGetFile GameGetFile_org, GameGetFile_ptr;

void FreeAllIHelpers( );




extern RawImageCallbackData * GlobalRawImageCallbackData;
BOOL RawImageGlobalCallbackFunc( RawImageEventType callbacktype, float mousex, float mousey );

extern vector<RawImageStruct> ListOfRawImages;

extern vector<FakeFileStruct> FakeFileList;
void ClearAllRawImages( );
extern BOOL NeedReleaseUnusedMemory;
#pragma endregion


#pragma region DotaFovFix.cpp
extern BOOL EnableFixFOV;
int __stdcall SetWidescreenFixState( BOOL widefixenable );
int __stdcall SetCustomFovFix( float _CustomFovFix );
extern Matrix1 globalmatrix;
void __fastcall SetGameAreaFOV_my( Matrix1 * a1, int a2, float a3, float a4, float a5, float a6 );
typedef int( __fastcall * SetGameAreaFOV )( Matrix1 * a1, int a2, float a3, float a4, float a5, float a6 );
extern SetGameAreaFOV SetGameAreaFOV_org, SetGameAreaFOV_ptr;
#pragma endregion


#pragma region DotaWebHelper.cpp
string SendHttpPostRequest( const char * url, const char * data );
string SendHttpGetRequest( const char * host, const char * path );
#pragma endregion 


#pragma region DotaAutoFPSlimit.cpp
extern BOOL FPS_LIMIT_ENABLED;
extern HANDLE Warcraft3_Process;
extern unsigned int CPU_cores;
void InitThreadCpuUsage( );
double GetWar3CpuUsage( );
void UpdateFPS( );
typedef int( __cdecl * p_SetMaxFps )( int maxfps );
extern p_SetMaxFps _SetMaxFps;
typedef void( __fastcall * pDrawBarForUnit )( int unitaddr );
void __fastcall DrawBarForUnit_my( int unitaddr );
extern pDrawBarForUnit DrawBarForUnit_org, DrawBarForUnit_ptr;
//extern map<int, BOOL> NeedDrawBarForUnit;
extern BOOL FPSfix1Enabled;
#pragma endregion


#pragma region DotaChatHelper.cpp
typedef int( __fastcall * pGameChatSetState )( int chat, int unused, BOOL IsOpened );
extern pGameChatSetState GameChatSetState;
#pragma endregion


#pragma region DotaCustomFrames.cpp
extern BOOL usedcustomframes;
extern int pCurrentFrameFocusedAddr;
extern int DefaultCStatus;
extern int LoadFramesVar1;
extern int LoadFramesVar2;
typedef void( __fastcall * pLoadNewFrameDef )( const char * filename, int var1, int var2, int cstatus );
extern pLoadNewFrameDef LoadNewFrameDef_org;
typedef int( __fastcall * pCreateNewFrame ) ( const char * FrameName, int pGlobalGameClass, int unk1, int unk2, int unk3 );
extern pCreateNewFrame CreateNewFrame;
typedef void( __thiscall * pSetFramePos )( int FrameAddr180, int rowid, float left, float bottom, BOOL unk );
extern pSetFramePos SetFramePos;
//sub_6F606770
typedef void *( __thiscall * pShowFrameAlternative )( int FrameAddr180, int unk1_0, int pGlobalGameClass180, int unk1_1, float x, float y, BOOL unk );
extern pShowFrameAlternative ShowFrameAlternative;
typedef void *( __thiscall * pShowThisFrame )( int FrameAddr );
extern pShowThisFrame ShowThisFrame;
typedef void *( __thiscall * pDestructThisFrame )( int FrameAddr, BOOL unk );
extern pDestructThisFrame DestructThisFrame;//Without clean memory
typedef int( __fastcall * pGetFrameItemAddress )( const char * name, int id );
extern pGetFrameItemAddress GetFrameItemAddress;
typedef int( __thiscall * pUpdateFrameFlags )( int FrameAddr, char unk );
extern pUpdateFrameFlags UpdateFrameFlags;
typedef int( __thiscall *  pWc3ControlClickButton )( int btnaddr, int unk );
extern pWc3ControlClickButton Wc3ControlClickButton_org, Wc3ControlClickButton_ptr;
int __fastcall Wc3ControlClickButton_my( int btnaddr, int, int unk );
int __stdcall ShowConfigWindow( const char * );
extern BOOL NeedOpenConfigWindow;
void ProcessClickAtCustomFrames( );
extern BOOL GlyphButtonCreated;
#pragma endregion


#pragma region Overlays
// for all warcraft versions
void UninitOpenglHook( );
void InitOpenglHook( );

// 1.26a
void Uninitd3d8Hook( BOOL cleartextures );
void Initd3d8Hook( );

// 1.27a
void Uninitd3d9Hook( BOOL cleartextures );
void Initd3d9Hook( );

void DrawOverlayDx9( );
void DrawOverlayDx8( );
void DrawOverlayGl( );

void SetNewLightDx9( int id );
void SetOldLightDx9( int id );

void SetNewLightDx8( int id );
void SetOldLightDx8( int id );

extern BOOL OverlayDrawed;

#pragma endregion


#pragma region MemoryHack
//Get module from addr
HMODULE GetModuleFromAddress( int addr );
typedef int( __cdecl * GetTownUnitCount_p )( int *, int, BOOL );
int __cdecl Wc3MemoryRW( int * addr, int value, BOOL write );
extern GetTownUnitCount_p GetTownUnitCount_org;
extern GetTownUnitCount_p GetTownUnitCount_ptr;
int __stdcall GetJassStringCount( BOOL dump );
int __stdcall ScanJassStringForErrors( BOOL dump );
#pragma endregion

const float DesktopScreen_Width = ( float )GetSystemMetrics( SM_CXSCREEN );
const float DesktopScreen_Height = ( float )GetSystemMetrics( SM_CYSCREEN );

extern float DefaultSceenWidth;
extern float DefaultSceenHeight;

#pragma region DotaDesyncScan
extern int ScanId;


#pragma endregion



inline std::string WStringToString( LPCWSTR s )
{
	if ( !s )
		return "";
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes( s );
}

inline std::wstring StringToWString( LPCSTR s )
{
	if ( !s )
		return L"";
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes( s );
}
