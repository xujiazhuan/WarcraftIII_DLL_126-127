#pragma once
#pragma warning(disable:4668)
#pragma warning(disable:4820)
#pragma warning(disable:4710)
#pragma warning(disable:4100)

#define MY_HEADER_FILE_
#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#pragma region Includes
// Все WINAPI и прочие функции

#include <intrin.h>
#include <vector>
#include <tchar.h>
#include <fstream> 
#include <iostream>
#pragma intrinsic(_ReturnAddress)
// Перехват функций
#include <MinHook.h>
#include <string>
using namespace std;
#pragma comment(lib,"libMinHook.x86.lib")

#pragma endregion
#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0



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
} ;

bool FileExist( const char * name );

BOOL __stdcall IsNotBadUnit( int unitaddr );
BOOL __stdcall IsEnemy( int UnitAddr );
BOOL __stdcall IsHero( int UnitAddr );
BOOL __stdcall IsTower( int unitaddr );

#pragma region UnitAndItem.cpp
int __stdcall GetUnitOwnerSlot( int unitaddr );
BOOL __stdcall IsHero( int unitaddr );
BOOL __stdcall IsTower( int unitaddr );
BOOL __stdcall IsNotBadUnit( int unitaddr );
BOOL __stdcall IsNotBadItem( int itemaddr );
typedef int( __fastcall * pGetHeroInt )( int unitaddr, int unused, BOOL withbonus );
extern pGetHeroInt GetHeroInt;
#pragma endregion

#pragma region ManaBar.cpp
extern BYTE BarVtableClone[];
void ManaBarSwitch( int GameDLL, HMODULE StormDLL, BOOL b );
void PatchOffset( void * addr, void * buffer, unsigned int size );
int __stdcall SetColorForUnit( unsigned int  * coloraddr, BarStruct * BarStruct );
#pragma endregion



//  Game.dll
extern int GameDll;
//	Storm.dll
extern int StormDll;
extern HMODULE GameDllModule;
extern HMODULE StormDllModule;
extern int GameVersion;

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
extern int MapNameOffset1;
extern int MapNameOffset2;
extern int pOnChatMessage_offset;
extern int _BarVTable ;
extern int pAttackSpeedLimit;
extern int GetItemInSlotAddr;
extern int GetWindowXoffset;
extern int GetWindowYoffset ;

#pragma endregion