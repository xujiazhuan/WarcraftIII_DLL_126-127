#pragma once
//#define DOTA_HELPER_LOG


#define MY_HEADER_FILE_
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

#define MASK_56 (((u_int64_t)1<<56)-1) /* i.e., (u_int64_t)0xffffffffffffff */

#include "fnv.h"
#pragma endregion
#define IsKeyPressed(CODE) ((GetAsyncKeyState(CODE) & 0x8000) > 0)

extern BOOL TerminateStarted;
extern BOOL IsVEHex;

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
	int _unk18;			// 64
	int _unk19_pointer; // 68
	int _unk20;			// 6C
	int _unk21;			// 70
	int _unk22;			// 78
	int _unk23;			// 7C
	int _unk24;			// 80
	int _unk25;			// 84
	int bartype;		// 88
	int _unk26;			// 8C
	int _unk27;			// 90
	int _unk28;			// 94
	float offset5;		// 98
	float offset6;		// 9C
	float offset7;		// 100
	float offset8;		// 104
	float offset9;		// 108
	int _unk29;			// 10C
	int _unk30;			// 110
	int _unk31;			// 114
	int _unk32;			// 118
	int _unk33;			// 11C
	int _unk34;			// 120
	int _unk35;			// 124
	int _unk36;			// 128
	int _unk37;			// 12C
	int _unk38;			// 130
	int _unk39;			// 134
	int _unk40_pointer;	// 138
	int _unk41_pointer;	// 13C
	int _unk42;			// 140
	int _unk43_pointer;	// 144
	int _unk44_pointer;	// 148
	int _unk45;			// 14C
	int _unk46_pointer;	// 150
	int _unk47_pointer;	// 154
	int _unk48;			// 158
	int _unk49_pointer;	// 15C
	int _unk50_pointer;	// 160
	int _unk51;			// 164
	int _unk52_pointer;	// 168
	int _unk53_pointer;	// 16C
	int _unk54;			// 170
	int _unk55_pointer;	// 174
	int _unk56_pointer;	// 178
	int _unk57;			// 17C
	int _unk58_pointer;	// 180
	int _unk59_pointer;	// 184
	int _unk60;			// 188
	float offset10;		// 18C
	float offset11;		// 190
	float offset12;		// 194
	int _unk61_pointer;	// 198
	int _unk62;			// 19C
	int _unk63;			// 200
	int _unk64_pointer;	// 204
	int _unk65_pointer;	// 208
	int _unk66;			// 20C
	int _unk67;			// 210
	float offset13;		// 214
	int unitaddr;		// 218
	int _unk68;			// 21C
	int _unk69;			// 220
};
int GetGlobalClassAddr( );
bool FileExist( const char * name );
DWORD GetDllCrc32( );
//typedef void *( __cdecl * _TriggerExecute )( int TriggerHandle );
//extern _TriggerExecute TriggerExecute;
// Total Size: 32 bytes
struct RCString
{
	int	VTable;
	int	refCount;
	int	Field_0008;
	int	Field_000C;
	int	Field_0010;
	int	Field_0014;
	int	Field_0018;
	LPSTR	String;
	RCString( )
	{
		VTable = refCount = Field_0008
			= Field_000C = Field_0010
			= Field_0014 = Field_0018
			= 0;
		String = NULL;
	}
};

// Total Size: 16 bytes
struct JassString
{
	int		VTable;
	int		Field_0004;
	RCString*	Value;
	int		Field_000C;
	JassString( )
	{
		VTable = Field_0004 = Field_000C = 0;
		Value = NULL;
	}
};
typedef void( __cdecl * pExecuteFunc )( JassString * funcname );
extern pExecuteFunc ExecuteFunc;
typedef void( __thiscall * pConvertStrToJassStr )( JassString * jStr, const char * cStr );
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

typedef BOOL( __cdecl * pGetPlayerAlliance )( unsigned int hPlayer1, unsigned int hPlayer2, int hAlliancetype );
extern pGetPlayerAlliance GetPlayerAlliance;
typedef unsigned int( __cdecl * pGetPlayerColor )( unsigned int whichPlayer );
extern pGetPlayerColor GetPlayerColor;
typedef int( __cdecl * pPlayer )( int number );
extern pPlayer Player;
typedef int( __cdecl * pIsPlayerObs )( unsigned int hPlayer );
extern pIsPlayerObs IsPlayerObs;


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
extern pOnChatMessage pOnChatMessage_org, pOnChatMessage_ptr;
BOOL IsPlayerObserver( int pid );
BOOL IsLocalPlayerObserver( );
extern BOOL ShowSkillPanelForObservers;
#pragma endregion

#pragma region UnitAndItem.cpp
int * FindUnitAbils( int unitaddr, unsigned int * count, int abilcode = 0, int abilbasecode = 0 );
int __stdcall GetUnitOwnerSlot( int unitaddr );
BOOL __stdcall IsEnemy( int UnitAddr );
BOOL __stdcall IsHero( int unitaddr );
BOOL __stdcall IsTower( int unitaddr );
BOOL __stdcall IsNotBadUnit( int unitaddr );
BOOL __stdcall IsUnitInvulnerable( int unitaddr );
BOOL __stdcall IsNotBadItem( int itemaddr );
typedef int( __fastcall * pGetHeroInt )( int unitaddr, int unused, BOOL withbonus );
extern pGetHeroInt GetHeroInt;
int GetSelectedUnitCountBigger( int slot );
int GetSelectedUnit( int slot );
#pragma endregion

#pragma region DotaMPBarHelper.cpp

extern BYTE BarVtableClone[ 0x80 ];
void ManaBarSwitch( BOOL b );
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
#ifdef DOTA_HELPER_LOG
void  __stdcall AddNewLineToJassLog( string s );
void __stdcall  AddNewLineToDotaChatLog( string s );
void __stdcall  AddNewLineToDotaHelperLog( string s );
#endif
void __stdcall EnableErrorHandler( int );
void __stdcall DisableErrorHandler( int );

extern void ResetTopLevelExceptionFilter( );
extern LPTOP_LEVEL_EXCEPTION_FILTER OriginFilter;
extern BOOL bDllLogEnable;
typedef LONG( __fastcall * StormErrorHandler )( int a1, void( *a2 )( int, const char *, ... ), int a3, BYTE *a4, LPSYSTEMTIME a5 );
extern StormErrorHandler StormErrorHandler_org;
typedef int( __fastcall *LookupNative )(int global, int unused, LPSTR name );
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
//extern int pTriggerExecute;
#pragma endregion



#pragma region DotaClickHelper.cpp
extern vector<int> doubleclickSkillIDs;
extern vector<int> WhiteListForTeleport;
extern BOOL ShopHelperEnabled;
extern BOOL TeleportShiftPress;
extern BOOL BlockKeyAndMouseEmulation;
extern BOOL EnableSelectHelper;
extern BOOL ClickHelper;
extern BOOL LOCK_MOUSE_IN_WINDOW;
extern BOOL BlockKeyboardAndMouseWhenTeleport;
extern BOOL rawimage_skipmouseevent;
typedef LRESULT( __fastcall *  WarcraftRealWNDProc )( HWND hWnd, unsigned int Msg, WPARAM wParam, LPARAM lParam );
extern WarcraftRealWNDProc WarcraftRealWNDProc_org, WarcraftRealWNDProc_ptr;
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
	string FilePath;
	float X, Y, Z, Radius;
	ModelCollisionFixStruct( )
	{
		FilePath = string( );
		X = Y = Z = Radius = 0.0f;
	}
};
struct ModelTextureFixStruct
{
	string FilePath;
	int TextureID;
	string NewTexturePath;
	ModelTextureFixStruct( )
	{
		FilePath = string( );
		NewTexturePath = string( );
		TextureID = 0;
	}
};
struct ModelPatchStruct
{
	string FilePath;
	string patchPath;
	ModelPatchStruct( )
	{
		FilePath = string( );
		patchPath = string( );
	}
};
struct ModelRemoveTagStruct
{
	string FilePath;
	string TagName;
	ModelRemoveTagStruct( )
	{
		FilePath = string( );
		TagName = string( );
	}
};
struct ModelSequenceReSpeedStruct
{
	string FilePath;
	string AnimationName;
	float SpeedUp;
	ModelSequenceReSpeedStruct( )
	{
		FilePath = string( );
		AnimationName = string( );
		SpeedUp = 0.0f;
	}
};
struct ModelScaleStruct
{
	string FilePath;
	float Scale;
	ModelScaleStruct( )
	{
		Scale = 0.0f;
		FilePath = string( );
	}
};
struct ModelSequenceValueStruct
{
	string FilePath;
	string AnimationName;
	int Indx;
	float Value;
	ModelSequenceValueStruct( )
	{
		FilePath = string( );
		AnimationName = string( );
		Indx = 0;
		Value = 0.0f;
	}
};
extern vector<ModelCollisionFixStruct> ModelCollisionFixList;
extern vector<ModelTextureFixStruct> ModelTextureFixList;
extern vector<ModelPatchStruct> ModelPatchList;
extern vector<ModelRemoveTagStruct> ModelRemoveTagList;
extern vector<ModelSequenceReSpeedStruct> ModelSequenceReSpeedList;
extern vector<ModelSequenceValueStruct> ModelSequenceValueList;
extern vector<ModelScaleStruct> ModelScaleList;

struct ICONMDLCACHE
{
	uint64_t _hash;
	size_t hashlen;
	char * buf;
	size_t size;
};

extern vector<ICONMDLCACHE> ICONMDLCACHELIST;
struct FileRedirectStruct
{
	string NewFilePath;
	string RealFilePath;
	FileRedirectStruct( )
	{
		NewFilePath = string( );
		RealFilePath = string( );
	}
};
extern vector<FileRedirectStruct> FileRedirectList;
typedef signed int( __stdcall * Storm_403 )( void *a1, const char * str, int line, int id );
extern Storm_403 Storm_403_org;
typedef BOOL( __fastcall * GameGetFile )( const char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown );
BOOL __fastcall GameGetFile_my( const  char * filename, int * OutDataPointer, size_t * OutSize, BOOL unknown );
extern GameGetFile GameGetFile_org, GameGetFile_ptr;
//int __stdcall Storm_279_my( const char * filename, int arg1, int arg2, size_t arg3, int arg4 );
//typedef int( __stdcall * Storm_279 )( const char * filename, int, int, size_t, int );
//extern Storm_279 Storm_279_org;
//extern Storm_279 Storm_279_ptr;
void FreeAllIHelpers( );
struct FakeFileStruct
{
	char * filename;
	BYTE * buffer;
	size_t size;
};


enum class RawImageEventType : unsigned int
{
	MouseUp = 1U,
	MouseDown = 2U,
	MouseClick = 4U,
	MouseEnter = 8U,
	MouseLeave = 16U,
	MouseMove = 32U,
	ALL = 63U
};


struct RawImageCallbackData
{
	int RawImage;
	RawImageEventType EventType;
	float mousex;
	float mousey;
	BOOL IsAltPressed;
	BOOL IsCtrlPressed;
	BOOL IsLeftButton;
};



extern RawImageCallbackData * GlobalRawImageCallbackData;
BOOL RawImageGlobalCallbackFunc( RawImageEventType callbacktype, float mousex, float mousey );

struct RawImageStruct
{
	int RawImage;
	int width;
	int height;
	Buffer img;
	Buffer ingamebuffer;
	BOOL ingame;
	string filename;
	BOOL used_for_overlay;
	float overlay_x; // 0.0 1.0
	float overlay_y; // 0.0 1.0
	float size_x; // 0.0 1.0
	float size_y; // 0.0 1.0
	void * textureaddr;
	BOOL needResetTexture;
	BOOL MouseCallback;
	JassString MouseActionCallback;
	BOOL IsMouseDown;
	BOOL IsMouseEntered;
	unsigned int events;

	RawImageStruct( )
	{
		width = 0;
		height = 0;
		img = Buffer( );
		ingamebuffer = Buffer( );
		ingame = FALSE;
		filename = string( );
		used_for_overlay = FALSE;
		overlay_x = overlay_y = size_x = size_y = 0.0f;
		textureaddr = NULL;
		needResetTexture = FALSE;
		RawImage = 0;
		events = 0;
		IsMouseDown = FALSE;
		IsMouseEntered = FALSE;
		MouseCallback = FALSE;
		MouseActionCallback = JassString( );
	}
};

extern vector<RawImageStruct> ListOfRawImages;

extern vector<FakeFileStruct> FakeFileList;
void ClearAllRawImages( );
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
string SendHttpPostRequest( const char * host, const char * path, const char * data );
string SendHttpGetRequest( const char * host, const char * path );
#pragma endregion 


#pragma region SendGamePacket.cpp
extern int PacketClassPtr;
extern int pGAME_SendPacket;
void SendPacket( BYTE* packetData, DWORD size );
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
void Uninitd3d8Hook( );
void Initd3d8Hook( );

// 1.27a
void Uninitd3d9Hook( );
void Initd3d9Hook( );

void DrawOverlayDx9( );
void DrawOverlayDx8( );
void DrawOverlayGl( );

extern BOOL OverlayDrawed;

#pragma endregion

const float DesktopScreen_Width = ( float )GetSystemMetrics( SM_CXSCREEN );
const float DesktopScreen_Height = ( float )GetSystemMetrics( SM_CYSCREEN );

