#pragma once

#define MY_HEADER_FILE_
#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
//#define PSAPI_VERSION 1

#pragma region Includes
#include <cstdio>
#include <cstring>
#include <csetjmp>

#include <Windows.h>
#include <string>
#include <vector>
#include <fstream> 
#include <iostream>
#include <sstream>
#include <algorithm>
#include <MinHook.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;


using namespace std;
#pragma comment(lib, "libMinHook.x86.lib")
#include <winsock2.h>
#pragma comment (lib, "Ws2_32.lib")
#include "buffer.h"
#pragma endregion
#define IsKeyPressed(CODE) ((GetAsyncKeyState(CODE) & 0x8000) > 0)

extern BOOL TerminateStarted;

struct CustomHPBar
{
	int unittypeid;
	unsigned int color;
	float scalex;
	float scaley;
};

struct Matrix1//Matrix 4x4
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


struct BarStruct
{
	int _BarClass;		// 0
	int _unk1_flag;		// 4
	int _unk2_flag;		// 8
	int _unk3_pointer;	// C
	int _unk4;			// 10
	int _unk5;			// 14
	int _unk6;			// 18
	int _unk7;			// 1c
	int _unk8;			// 20
	int _unk9;			// 24
	int _unk10;			// 28
	int _unk11;			// 2c
	int _unk12;			// 30
	int _unk13;			// 34
	int _unk14;			// 38
	int _unk15_pointer;	// 3c
	int _unk16_pointer;	// 40
	float offset1;		// 44
	float offset2;		// 48
	float offset3;		// 4c
	float offset4;		// 50
	int _unk17_flag;	// 54
	float ScaleX;		// 58
	float ScaleY;		// 5c
	float Scale;		// 60
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

bool FileExist( const char * name );
DWORD GetDllCrc32( );
//typedef void *( __cdecl * _TriggerExecute )( int TriggerHandle );
//extern _TriggerExecute TriggerExecute;
BOOL __stdcall IsNotBadUnit( int unitaddr );
BOOL __stdcall IsEnemy( int UnitAddr );
BOOL __stdcall IsHero( int UnitAddr );
BOOL __stdcall IsTower( int unitaddr );
BOOL IsClassEqual( int ClassID1, int ClassID2 );
int GetTypeId( int unit_item_abil_etc_addr );

string ToLower( string s );

extern BOOL MainFuncWork;

#pragma region DotaPlayerHelper.cpp

int GetLocalPlayerId( );
int GetPlayerByNumber( int number );

// Проверить являются ли игроки врагами
typedef int( __cdecl * IsPlayerEnemy )( unsigned int Player1, unsigned int Player2 );
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
extern pOnChatMessage pOnChatMessage_org;
extern pOnChatMessage pOnChatMessage_ptr;

#pragma endregion

#pragma region UnitAndItem.cpp

int __stdcall GetUnitOwnerSlot( int unitaddr );
BOOL __stdcall IsHero( int unitaddr );
BOOL __stdcall IsTower( int unitaddr );
BOOL __stdcall IsNotBadUnit( int unitaddr );
BOOL __stdcall IsNotBadItem( int itemaddr );
typedef int( __fastcall * pGetHeroInt )( int unitaddr, int unused, BOOL withbonus );
extern pGetHeroInt GetHeroInt;
int GetSelectedUnitCountBigger( int slot );
int GetSelectedUnit(int slot);
#pragma endregion

#pragma region DotaMPBarHelper.cpp

extern BYTE BarVtableClone[ 0x80 ];
void ManaBarSwitch( int GameDLL, BOOL b );
void PatchOffset( void * addr, void * buffer, unsigned int size );
int __stdcall SetColorForUnit( unsigned int  * coloraddr, BarStruct * BarStruct );
typedef void *( __stdcall * Storm_401 )( size_t Size, const char * srcfile, int line, int val );
extern Storm_401 Storm_401_org;
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
void AddNewLineToDotaChatLog( string s );
void EnableErrorHandler( );
void DisableErrorHandler( );
void AddNewLineToDotaHelperLog( string s );
extern void ResetTopLevelExceptionFilter( );
extern LPTOP_LEVEL_EXCEPTION_FILTER OriginFilter;
extern BOOL bDllLogEnable;
typedef LONG( __fastcall * StormErrorHandler )( int a1, void( *a2 )( int, const char *, ... ), int a3, BYTE *a4, LPSYSTEMTIME a5 );
extern StormErrorHandler StormErrorHandler_org;
typedef int( __fastcall *LookupNative )( LPSTR );
extern LookupNative LookupNative_org;
typedef signed int( __fastcall * LookupJassFunc )( int a1, int unused, char * funcname );
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
extern int GetPlayerByIDOffset;
extern int DrawSkillPanelOffset;
extern int DrawSkillPanelOverlayOffset;
extern int IsDrawSkillPanelOffset;
extern int IsDrawSkillPanelOverlayOffset;
extern int IsNeedDrawUnitOriginOffset;
extern int IsNeedDrawUnit2offset;
extern int IsNeedDrawUnit2offsetRetAddress;
extern BOOL * InGame;
extern int IsWindowActive;
extern int ChatFound;
extern int pW3XGlobalClass;
extern int pGameClass1;
extern int pWar3Data1;
extern int UnitVtable;
extern int ItemVtable;
extern int pPrintText2;
extern void PrintText( const char * text, float staytime = 10.0f);
extern int MapNameOffset1;
extern int MapNameOffset2;
extern int pOnChatMessage_offset;
extern int _BarVTable;
extern int pAttackSpeedLimit;
extern int GetItemInSlotAddr;
extern float * GetWindowXoffset;
extern float * GetWindowYoffset;
extern int GameFrameAtMouseStructOffset;
//extern int pTriggerExecute;
#pragma endregion



#pragma region DotaClickHelper.cpp

extern BOOL BlockKeyAndMouseEmulation;
extern BOOL EnableSelectHelper;
extern BOOL ClickHelper;


typedef LRESULT( __fastcall *  WarcraftRealWNDProc )( HWND hWnd, unsigned int Msg, WPARAM wParam, LPARAM lParam );
extern WarcraftRealWNDProc WarcraftRealWNDProc_org;
extern WarcraftRealWNDProc WarcraftRealWNDProc_ptr;

extern HANDLE hPressKeyWithDelay;
LRESULT __fastcall BeforeWarcraftWNDProc( HWND hWnd, unsigned int Msg, WPARAM _wParam, LPARAM lParam );
extern BOOL PressKeyWithDelayEND;
DWORD WINAPI PressKeyWithDelay( LPVOID );

extern unsigned char ShiftPressed;
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
extern c_SimpleButtonClickEvent SimpleButtonClickEvent;

#pragma endregion


#pragma region DotaFilesHelper.cpp

struct ModelCollisionFixStruct
{
	char FilePath[ 512 ];
	float X, Y, Z, Radius;
};

struct ModelTextureFixStruct
{
	char FilePath[ 512 ];
	int TextureID;
	char NewTexturePath[ 260 ];
};


struct ModelPatchStruct
{
	char FilePath[ 512 ];
	char patchPath[ 512 ];
};


struct ModelRemoveTagStruct
{
	char FilePath[ 512 ];
	char TagName[ 5 ];
};

struct ModelSequenceReSpeedStruct
{
	char FilePath[ 512 ];
	char AnimationName[ 100 ];
	float SpeedUp;
};


struct ModelScaleStruct
{
	char FilePath[ 512 ];
	float Scale;
};



struct ModelSequenceValueStruct
{
	char FilePath[ 512 ];
	char AnimationName[ 100 ];
	int Indx;
	float Value;
};


extern vector<ModelCollisionFixStruct> ModelCollisionFixList;
extern vector<ModelTextureFixStruct> ModelTextureFixList;
extern vector<ModelPatchStruct> ModelPatchList;
extern vector<ModelRemoveTagStruct> ModelRemoveTagList;
extern vector<ModelSequenceReSpeedStruct> ModelSequenceReSpeedList;
extern vector<ModelSequenceValueStruct> ModelSequenceValueList;
extern vector<ModelScaleStruct> ModelScaleList;

struct FileRedirectStruct
{
	char NewFilePath[ 512 ];
	char RealFilePath[ 512 ];
};

extern vector<FileRedirectStruct> FileRedirectList;

typedef signed int( __stdcall * Storm_403 )( void *a1, const char * str, int line, int id );
extern Storm_403 Storm_403_org;

typedef signed int( __fastcall * GameGetFile )( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown );
signed int __fastcall GameGetFile_my( const  char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown );
extern GameGetFile GameGetFile_org;
extern GameGetFile GameGetFile_ptr;

void FreeAllIHelpers( );

struct FakeFileStruct
{
	char * filename;
	BYTE * buffer;
	size_t size;
};
extern vector<FakeFileStruct> FakeFileList;

#pragma endregion


#pragma region DotaFovFix.cpp

int __stdcall SetWidescreenFixState( BOOL widefixenable );
int __stdcall SetCustomFovFix( float _CustomFovFix );
void __fastcall SetGameAreaFOV_my( Matrix1 * a1, int a2, float a3, float a4, float a5, float a6 );
typedef int( __fastcall * SetGameAreaFOV )( Matrix1 * a1, int a2, float a3, float a4, float a5, float a6 );
extern SetGameAreaFOV SetGameAreaFOV_org;
extern SetGameAreaFOV SetGameAreaFOV_ptr;


#pragma endregion


#pragma region DotaWebHelper.cpp

string SendHttpPostRequest( const char * host, const char * path, const char * data );

string SendHttpGetRequest( const char * host, const char * path );

#pragma endregion 


#pragma region SendGamePacket.cpp


extern int PacketClassPtr;
extern int pGAME_SendPacket;
void SendPacket( BYTE* packetData, DWORD size );

#pragma endregion