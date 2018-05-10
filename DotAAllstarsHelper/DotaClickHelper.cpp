#include "Main.h"



BOOL SetInfoObjDebugVal = TRUE;




HWND Warcraft3Window = 0;


WarcraftRealWNDProc WarcraftRealWNDProc_org = NULL;
WarcraftRealWNDProc WarcraftRealWNDProc_ptr;


LPARAM lpF1ScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_F1, 0 ) << 16 ) );
LPARAM lpF1ScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_F1, 0 ) << 16 ) );

LPARAM lpAltScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_MENU, 0 ) << 16 ) );
LPARAM lpAltScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_MENU, 0 ) << 16 ) );

LPARAM lpCtrlScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_CONTROL, 0 ) << 16 ) );
LPARAM lpCtrlScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_CONTROL, 0 ) << 16 ) );

BOOL EmulateKeyInputForHWND = FALSE;


int ShiftPressed = 0;
BOOL SkipAllMessages = FALSE;


void PressKeyboard( int VK )
{
	BOOL PressedKey = FALSE;
	INPUT Input = { 0 };
	Input.type = INPUT_KEYBOARD;
	Input.ki.wScan = ( WORD )MapVirtualKey( ( unsigned int )VK, 0 );
	Input.ki.wVk = ( WORD )VK;
	if ( IsKeyPressed( VK ) )
	{
		PressedKey = TRUE;
		Input.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput( 1, &Input, sizeof( INPUT ) );
	}

	Input.ki.dwFlags = 0;
	SendInput( 1, &Input, sizeof( INPUT ) );
	if ( !PressedKey )
	{
		Input.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput( 1, &Input, sizeof( INPUT ) );
	}
}

DWORD LastKeyPressedTime = 0;
DWORD LastKeyPressedKey = 0;

void __fastcall PressCancel( int data )
{
	__asm
	{
		push data;
		mov ecx, GameDll;
		add ecx, 0xACE9EC;
		mov edi, GameDll;
		add edi, 0x3747E0;
		call edi;
	}
}

BOOL IsCursorSelectTarget( )
{
	int pOffset1 = GetGlobalClassAddr( );
	if ( pOffset1 > 0 && *( int* )( pOffset1 + 0x1BC ) == 1 )
	{
		/*char tmp[ 100 ];
		sprintf_s( tmp, 100, "%X", pOffset1 );
		MessageBoxA( 0, tmp, tmp, 0 );*/
		return TRUE;
	}
	return FALSE;
}

int GetCursorSkillID( )
{
	int pOffset1 = GetGlobalClassAddr( );
	if ( pOffset1 > 0 && ( pOffset1 = *( int* )( pOffset1 + 0x1B4 ) ) > 0 )
	{
		return *( int* )( pOffset1 + 0xC );
	}
	return 0;
}

int GetCursorOrder( )
{
	int pOffset1 = GetGlobalClassAddr( );
	if ( pOffset1 > 0 && ( pOffset1 = *( int* )( pOffset1 + 0x1B4 ) ) > 0 )
	{
		return *( int* )( pOffset1 + 0x10 );
	}
	return 0;
}

vector<int> doubleclickSkillIDs;

int __stdcall AddDoubleClickSkillID( int skillID )
{
	/*char addedid[ 100 ];
	sprintf_s( addedid, "Added new id:%i", skillID );
	MessageBoxA( 0, addedid, "", 0 );*/
	if ( skillID == 0 && !doubleclickSkillIDs.empty( ) )
	{
		//MessageBoxA( 0, "ERROR! IDS CLEARED", "", 0 );
		doubleclickSkillIDs.clear( );
	}
	else
		doubleclickSkillIDs.push_back( skillID );

	return skillID;
}

float HeroPortX = 0.318f;
float HeroPortY = 0.888f;

BOOL ScreenToClientReplace = FALSE;

POINT ScreenToClientReplacedPoint;


BOOL ClientToScreenReplace = FALSE;

POINT ClientToScreenReplacedPoint;

LPARAM oldlParam = 0;



BOOL BLOCKMOUSEMOVING = FALSE;

float HeroFrameX = 0.256f;
float HeroFrameY = 0.0666f;

float HeroFrameX_old = 0.256f;
float HeroFrameY_old = 0.0666f;



void SetHeroFrameXY( )
{
	HeroFrameX_old = *( float* )( GameFrameAtMouseStructOffset + 0x14 );
	*( float* )( GameFrameAtMouseStructOffset + 0x14 ) = HeroFrameX;
	HeroFrameY_old = *( float* )( GameFrameAtMouseStructOffset + 0x18 );
	*( float* )( GameFrameAtMouseStructOffset + 0x18 ) = HeroFrameY;
}

void SetHeroFrameXY_old( )
{
	if ( *( float* )( GameFrameAtMouseStructOffset + 0x14 ) == HeroFrameX )
	{
		*( float* )( GameFrameAtMouseStructOffset + 0x14 ) = HeroFrameX_old;
		*( float* )( GameFrameAtMouseStructOffset + 0x18 ) = HeroFrameY_old;
	}
}



void MouseClickX( int toX, int toY )
{
	Sleep( 5 );
	POINT point;
	GetCursorPos( &point );
	PostMessage( Warcraft3Window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG( point.x, point.y ) );
	PostMessage( Warcraft3Window, WM_LBUTTONUP, MK_LBUTTON, MAKELONG( point.x, point.y ) );

	/*
		POINT cursorPos;
		GetCursorPos( &cursorPos );

		double dx = toX * ( 65535.0f / DesktopScreen_Width );
		double dy = toY * ( 65535.0f / DesktopScreen_Height );
		INPUT Input = { 0 };

		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
		Input.mi.dx = LONG( dx );
		Input.mi.dy = LONG( dy );
		SendInput( 1, &Input, sizeof( INPUT ) );
		SetHeroFrameXY( );

		SendInput( 1, &Input, sizeof( INPUT ) );
		SetHeroFrameXY( );

		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput( 1, &Input, sizeof( INPUT ) );
		SetHeroFrameXY( );

		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput( 1, &Input, sizeof( INPUT ) );
		SetHeroFrameXY( );

		dx = cursorPos.x*( 65535.0f / DesktopScreen_Width );
		dy = cursorPos.y*( 65535.0f / DesktopScreen_Height );

		Input.type = INPUT_MOUSE;
		Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
		Input.mi.dx = LONG( dx );
		Input.mi.dy = LONG( dy );
		SendInput( 1, &Input, sizeof( INPUT ) );
		SetHeroFrameXY( );*/
}




unsigned long __stdcall ThreadTest( void * lpp )
{
	POINT * p = ( POINT * )lpp;
	SkipAllMessages = TRUE;
	MouseClickX( p->x, p->y );
	SkipAllMessages = FALSE;
	delete p;
	return 0;
}

void MouseClick( int toX, int toY )
{
	POINT * ClickPoint = new POINT( );
	ClickPoint->x = toX;
	ClickPoint->y = toY;
	HANDLE thr = CreateThread( 0, 0, ThreadTest, ClickPoint, 0, 0 );
	if ( thr != NULL )
		CloseHandle( thr );
}

void JustClickMouse( )
{
	BOOL ButtonDown = FALSE;
	if ( IsKeyPressed( VK_LBUTTON ) )
	{
		ButtonDown = TRUE;
		SendMessage( Warcraft3Window, WM_LBUTTONUP, 0, oldlParam );
	}

	INPUT Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput( 1, &Input, sizeof( INPUT ) );

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput( 1, &Input, sizeof( INPUT ) );

}
int PressMouseAtSelectedHero( BOOL IsItem )
{
	int errorvalue = 0;
	if ( !IsCursorSelectTarget( ) )
	{
		errorvalue = 1;
		//PrintText( "ERROR 1" );
	}
	if ( GetCursorOrder( ) == 0xD000F ||
		GetCursorOrder( ) == 0xD0012 ||
		GetCursorOrder( ) == 0xD0016 )
	{
		errorvalue = 2;
		//	PrintText( "ERROR 2" );
	}

	if ( IsCursorSelectTarget( ) &&
		GetCursorOrder( ) != 0xD000F &&
		GetCursorOrder( ) != 0xD0012 &&
		GetCursorOrder( ) != 0xD0016 )
	{
		if ( /*!IsItem || */doubleclickSkillIDs.empty( ) ||
			std::find( doubleclickSkillIDs.begin( ), doubleclickSkillIDs.end( ), GetCursorSkillID( ) ) != doubleclickSkillIDs.end( )
			)
		{
			int PortraitButtonAddr = GetGlobalClassAddr( );
			if ( PortraitButtonAddr > 0 )
			{
				PortraitButtonAddr = *( int* )( PortraitButtonAddr + 0x3F4 );
			}
			if ( PortraitButtonAddr > 0 && !IsItem )
			{
				//PrintText( "NEED CLICK PORTRAIT" );
				Wc3ControlClickButton_org( PortraitButtonAddr, 1 );
			}
			else
			{
				errorvalue = 4;
				//PrintText( "ERROR 4" );
			}

			/*BOOL ButtonDown = FALSE;
			if ( IsKeyPressed( VK_LBUTTON ) )
			{
				ButtonDown = TRUE;
				SendMessage( Warcraft3Window, WM_LBUTTONUP, 0, oldlParam );
			}

			int x = ( int )( *GetWindowXoffset * HeroPortX );
			int y = ( int )( *GetWindowYoffset * HeroPortY );

			POINT cursorhwnd;
			GetCursorPos( &cursorhwnd );
			ScreenToClient( Warcraft3Window, &cursorhwnd );
			POINT cursor;
			GetCursorPos( &cursor );

			x = x - cursorhwnd.x;
			y = y - cursorhwnd.y;

			cursor.x = cursor.x + x;
			cursor.y = cursor.y + y;
			//( toXX, toYY );

			MouseClick( cursor.x, cursor.y );*/
		}
		else
		{
			errorvalue = 3;
			//PrintText( ( "ERROR 3:" + to_string( GetCursorSkillID( ) ) ).c_str( ) );
		}

	}

	return errorvalue;
}




DWORD LastPressedKeysTime[ 1024 ];


vector<int> RegisteredKeyCodes;
vector<int> BlockedKeyCodes;

vector<KeyActionStruct> KeyActionList;




struct mMessage
{
	unsigned int Msg;
	unsigned int wParam;
};

vector<mMessage> SkipMessagesList;


WPARAM LastShift = 0;

LPARAM MakeLParamVK( unsigned int VK, BOOL up, BOOL Extended = FALSE )
{
	if ( up ) return ( LPARAM )( 0xC0000001 | ( ( unsigned int )Extended << 24 ) | ( LPARAM )( MapVirtualKey( VK, 0 ) << 16 ) );
	else return ( LPARAM )( 0x00000001 | ( ( unsigned int )Extended << 24 ) | ( LPARAM )( MapVirtualKey( VK, 0 ) << 16 ) );
}


int __stdcall TriggerRegisterPlayerKeyboardEvent( int KeyCode )
{
	if ( !KeyCode )
	{
		if ( !RegisteredKeyCodes.empty( ) )
			RegisteredKeyCodes.clear( );
		return 0;
	}


	RegisteredKeyCodes.push_back( KeyCode );
	return 0;
}

int __stdcall BlockKeyAction( int KeyCode )
{
	if ( !KeyCode )
	{
		if ( !BlockedKeyCodes.empty( ) )
			BlockedKeyCodes.clear( );
		return 0;
	}
	BlockedKeyCodes.push_back( KeyCode );

	return 0;
}

int GetAltBtnID( int btnID )
{
	switch ( btnID )
	{
	case 2:
		return 0;
	case 5:
		return 3;
	case 8:
		return 6;
	case 11:
		return 9;
	case 4:
		return 1;
	case 7:
		return 4;
	}

	return -1;
}

BOOL EnabledReplaceHotkeyFlag = TRUE;

void __stdcall EnableReplaceHotkeyFlag( BOOL enabled )
{
	EnabledReplaceHotkeyFlag = enabled;
}

std::vector<KeySelectActionStruct> KeySelectActionList;

int __stdcall AddKeySelectAction( int KeyCode, int GroupHandle )
{
	if ( !KeyCode )
	{
		if ( !KeySelectActionList.empty( ) )
			KeySelectActionList.clear( );
		return 0;
	}

	KeySelectActionStruct tmpstr;
	tmpstr.VK = KeyCode & 0xFF;
	tmpstr.IsAlt = KeyCode & 0x10000;
	tmpstr.IsCtrl = KeyCode & 0x20000;
	tmpstr.IsShift = KeyCode & 0x40000;
	tmpstr.GroupHandle = GroupHandle;
	tmpstr.units = GetUnitsFromGroup( GroupHandle );
	//reverse( tmpstr.units.begin( ), tmpstr.units.end( ) );


	if ( !EnabledReplaceHotkeyFlag || KeyCode & 0x100000 )
	{
		for ( KeySelectActionStruct & curstr : KeySelectActionList )
		{
			if ( curstr.VK == tmpstr.VK )
			{
				if ( curstr.GroupHandle == tmpstr.GroupHandle ||
					( ( ( !curstr.IsAlt && !curstr.IsCtrl && !curstr.IsShift )
						&&
						( !tmpstr.IsAlt && !tmpstr.IsCtrl && !tmpstr.IsShift ) )

						|| ( curstr.IsAlt && tmpstr.IsAlt )

						|| ( curstr.IsCtrl && tmpstr.IsCtrl )

						|| ( curstr.IsShift && tmpstr.IsShift ) ) )
				{
					if ( SetInfoObjDebugVal )
					{
						PrintText( "Replace old selection hotkey." );
					}

					curstr = tmpstr;
					return 0;
				}
			}
		}
	}
	if ( SetInfoObjDebugVal )
	{
		PrintText( "Add new selection hotkey." );
	}

	KeySelectActionList.push_back( tmpstr );
	return 0;
}


std::vector<KeyChatActionStruct> KeyChatActionList;

int __stdcall AddKeyChatAction( int KeyCode, const char * str, BOOL SendToAll )
{
	if ( !KeyCode )
	{
		if ( !KeyChatActionList.empty( ) )
			KeyChatActionList.clear( );
		return 0;
	}

	KeyChatActionStruct tmpstr;
	tmpstr.VK = KeyCode & 0xFF;
	tmpstr.IsAlt = KeyCode & 0x10000;
	tmpstr.IsCtrl = KeyCode & 0x20000;
	tmpstr.IsShift = KeyCode & 0x40000;
	tmpstr.SendToAll = SendToAll;
	tmpstr.Message = str && strlen( str ) < 127 ? str : "Bad message length";

	if ( !EnabledReplaceHotkeyFlag || KeyCode & 0x100000 )
	{
		for ( KeyChatActionStruct & curstr : KeyChatActionList )
		{
			if ( curstr.VK == tmpstr.VK )
			{
				if ( ( ( !curstr.IsAlt && !curstr.IsCtrl && !curstr.IsShift ) &&
					( !tmpstr.IsAlt && !tmpstr.IsCtrl && !tmpstr.IsShift ) )
					|| ( curstr.IsAlt && tmpstr.IsAlt )
					|| ( curstr.IsCtrl && tmpstr.IsCtrl )
					|| ( curstr.IsShift && tmpstr.IsShift )
					)
				{
					if ( SetInfoObjDebugVal )
					{
						PrintText( "Replace old chat hotkey." );
					}
					curstr = tmpstr;
					return 0;
				}
			}
		}
	}
	if ( SetInfoObjDebugVal )
	{
		PrintText( "Add new chat hotkey." );
	}
	KeyChatActionList.push_back( tmpstr );

	return 0;
}

std::vector<KeyCalbackActionStruct> KeyCalbackActionList;

int __stdcall AddKeyCalbackAction( int KeyCode, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8 )
{
	if ( !KeyCode )
	{
		if ( !KeyCalbackActionList.empty( ) )
			KeyCalbackActionList.clear( );
		return 0;
	}

	KeyCalbackActionStruct tmpstr;
	tmpstr.VK = KeyCode & 0xFF;
	tmpstr.IsAlt = KeyCode & 0x10000;
	tmpstr.IsCtrl = KeyCode & 0x20000;
	tmpstr.IsShift = KeyCode & 0x40000;
	// save args safe
	tmpstr.args[ 0 ] = arg2;
	tmpstr.args[ 1 ] = arg3;
	tmpstr.args[ 2 ] = arg4;
	tmpstr.args[ 3 ] = arg5;
	tmpstr.args[ 4 ] = arg6;
	tmpstr.args[ 5 ] = arg7;
	tmpstr.args[ 6 ] = arg8;

	if ( !EnabledReplaceHotkeyFlag || KeyCode & 0x100000 )
	{
		for ( KeyCalbackActionStruct & curstr : KeyCalbackActionList )
		{
			if ( curstr.VK == tmpstr.VK )
			{
				if (
					( ( !curstr.IsAlt && !curstr.IsCtrl && !curstr.IsShift ) &&
					( !tmpstr.IsAlt && !tmpstr.IsCtrl && !tmpstr.IsShift ) )


					|| ( curstr.IsAlt && tmpstr.IsAlt )
					|| ( curstr.IsCtrl && tmpstr.IsCtrl )
					|| ( curstr.IsShift && tmpstr.IsShift )
					)
				{
					if ( SetInfoObjDebugVal )
					{
						PrintText( "Replace old callback hotkey." );
					}
					curstr = tmpstr;
					return 0;
				}
			}
		}
	}

	if ( SetInfoObjDebugVal )
	{
		char bufka[ 200 ];
		sprintf_s( bufka, "Add new callback hotkey. VK %X/ALT %X/CTRL %X/SHIFT %X", tmpstr.VK, tmpstr.IsAlt, tmpstr.IsCtrl, tmpstr.IsShift );

		PrintText( bufka );
	}

	KeyCalbackActionList.push_back( tmpstr );

	return 0;
}


int __stdcall AddKeyButtonAction( int KeyCode, int btnID, BOOL IsSkill )
{
	if ( !KeyCode )
	{
		if ( !KeyActionList.empty( ) )
			KeyActionList.clear( );
		return 0;
	}

	KeyActionStruct tmpstr;
	tmpstr.VK = KeyCode & 0xFF;
	tmpstr.btnID = btnID;
	tmpstr.IsSkill = IsSkill;

	if ( IsSkill )
		tmpstr.altbtnID = ( GetAltBtnID( btnID ) );
	else
		tmpstr.altbtnID = 0;

	tmpstr.IsAlt = KeyCode & 0x10000;
	tmpstr.IsCtrl = KeyCode & 0x20000;
	tmpstr.IsShift = KeyCode & 0x40000;
	tmpstr.IsRightClick = KeyCode & 0x80000;
	tmpstr.IsQuickCast = KeyCode & 0x200000;

	if ( !EnabledReplaceHotkeyFlag || KeyCode & 0x100000 )
	{
		for ( KeyActionStruct & curstr : KeyActionList )
		{
			if ( curstr.btnID == tmpstr.btnID )
			{
				if ( ( ( !curstr.IsAlt && !curstr.IsCtrl && !curstr.IsShift ) &&
					( !tmpstr.IsAlt && !tmpstr.IsCtrl && !tmpstr.IsShift ) )
					|| ( curstr.IsAlt && tmpstr.IsAlt )
					|| ( curstr.IsCtrl && tmpstr.IsCtrl )
					|| ( curstr.IsShift && tmpstr.IsShift )
					)
				{
					if ( SetInfoObjDebugVal )
					{
						PrintText( "Replace hotkey" );
					}
					curstr = tmpstr;
					return 0;
				}
			}
		}
	}
	if ( SetInfoObjDebugVal )
	{
		char debugtext[ 512 ];
		sprintf_s( debugtext, "%s:%X %s:%s %s:%s %s:%s %s:%s %s:%s",
			"added new hotkey", KeyCode,
			"IsAlt", GetBoolStr( tmpstr.IsAlt ),
			"IsCtrl", GetBoolStr( tmpstr.IsCtrl ),
			"IsShift", GetBoolStr( tmpstr.IsShift ),
			"IsRightClick", GetBoolStr( tmpstr.IsRightClick ),
			"IsQuickCast", GetBoolStr( tmpstr.IsQuickCast ) );
		PrintText( debugtext );

	}
	KeyActionList.push_back( tmpstr );

	return 0;
}




BOOL IsNULLButtonFound( int pButton )
{
	if ( pButton > 0 && *( int* )( pButton ) > 0 )
	{
		if ( *( int* )( pButton + 0x190 ) != 0 && *( int* )( *( int* )( pButton + 0x190 ) + 4 ) == 0 )
			return TRUE;
	}
	return FALSE;
}

// | 0 | 3 | 6 | 9  |
// | 1 | 4 | 7 | 10 | 
// | 2 | 5 | 8 | 11 |

#define flagsOffset 0x138
#define sizeOfCommandButtonObj 0x1c0

int __stdcall GetSkillPanelButton( int idx )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	if ( GetSelectedUnit( GetLocalPlayerId( ) ) )
	{
		int pclass = GetGlobalClassAddr( );
		if ( pclass > 0 )
		{
			int pGamePlayerPanelSkills = *( int* )( pclass + 0x3c8 );
			if ( pGamePlayerPanelSkills > 0 )
			{
				int topLeftCommandButton = *( int* )( pGamePlayerPanelSkills + 0x154 );
				if ( topLeftCommandButton > 0 )
				{
					topLeftCommandButton = **( int** )( topLeftCommandButton + 0x8 );
					if ( topLeftCommandButton > 0 )
						return topLeftCommandButton + sizeOfCommandButtonObj * idx;
				}
			}
		}
	}
	return 0;
}

// | 0 | 1
// | 2 | 3
// | 4 | 5

int __stdcall GetItemPanelButton( int idx )
{
	if ( GetSelectedUnit( GetLocalPlayerId( ) ) )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		int pclass = GetGlobalClassAddr( );
		if ( pclass > 0 )
		{
			int pGamePlayerPanelItems = *( int* )( pclass + 0x3c4 );
			if ( pGamePlayerPanelItems > 0 )
			{
				int topLeftCommandButton = *( int* )( pGamePlayerPanelItems + 0x148 );
				if ( topLeftCommandButton > 0 )
				{
					topLeftCommandButton = *( int* )( topLeftCommandButton + 0x130 );
					if ( topLeftCommandButton > 0 )
					{
						topLeftCommandButton = *( int* )( topLeftCommandButton + 0x4 );
						if ( topLeftCommandButton > 0 )
						{
							return topLeftCommandButton + sizeOfCommandButtonObj * idx;
						}
					}
				}
			}
		}
	}
	return 0;
}


int GetHeroButton( int idx )
{
	int pclass = GetGlobalClassAddr( );
	if ( pclass > 0 )
	{
		int pGamePlayerHeroBtn = *( int* )( pclass + 0x3c8 );
		if ( pGamePlayerHeroBtn > 0 )
		{
			pGamePlayerHeroBtn = *( int* )( pGamePlayerHeroBtn + 0x40 );
			if ( pGamePlayerHeroBtn > 0 )
			{
				pGamePlayerHeroBtn = *( int* )( pGamePlayerHeroBtn + 0x20 );
				return pGamePlayerHeroBtn;
			}
		}
	}
	return 0;
}


c_SimpleButtonClickEvent SimpleButtonClickEvent_org;
c_SimpleButtonClickEvent SimpleButtonClickEvent_ptr;

int CommandButtonVtable = 0;

BOOL IsCommandButton( int addr )
{
	if ( addr > 0 )
	{
		if ( CommandButtonVtable )
		{
			return *( int* )addr == CommandButtonVtable;
		}

	}
	return FALSE;
}

std::vector<ClickPortrainForId> ClickPortrainForIdList;

BOOL __stdcall AddClickPortrainForId( int abilid, int keycode )
{
	if ( abilid == 0 || keycode == 0 )
	{
		ClickPortrainForIdList.clear( );
		return FALSE;
	}

	ClickPortrainForId tmpClickPortrainForId = ClickPortrainForId( );
	tmpClickPortrainForId.abilid = abilid;
	tmpClickPortrainForId.keycode = keycode;
	tmpClickPortrainForId.checkforcd = FALSE;

	ClickPortrainForIdList.push_back( tmpClickPortrainForId );

	return TRUE;
}

BOOL __stdcall AddClickPortrainForIdEx( int abilid, int keycode, BOOL checkforcd )
{
	if ( abilid == 0 || keycode == 0 )
	{
		ClickPortrainForIdList.clear( );
		return FALSE;
	}

	ClickPortrainForId tmpClickPortrainForId = ClickPortrainForId( );
	tmpClickPortrainForId.abilid = abilid;
	tmpClickPortrainForId.keycode = keycode;
	tmpClickPortrainForId.checkforcd = checkforcd;

	ClickPortrainForIdList.push_back( tmpClickPortrainForId );

	return TRUE;
}

int CheckBtnForClickPortrain( int pButton )
{
	if ( pButton && IsCommandButton( pButton ) )
	{
		//PrintText( "SimpleButton IsCommandButton" );
		int CommandButtonData = *( int* )( pButton + 0x190 );
		if ( CommandButtonData )
		{
			//	PrintText( "Click command button." );
			int pAbil = *( int* )( CommandButtonData + 0x6D4 );
			if ( pAbil )
			{
				//	PrintText( "Abil found." );
				int pAbilId = *( int* )( pAbil + 0x34 );
				if ( pAbilId )
				{
					//	PrintText( "Abil id found." );
					for ( auto tmpClick : ClickPortrainForIdList )
					{
						if ( pAbilId == tmpClick.abilid && !IsKeyPressed( tmpClick.keycode ) )
						{
							if ( tmpClick.checkforcd )
							{
								if ( *( unsigned int * )( pAbil + 0x20 ) & 0x200 )
									return 2;
							}
							//		PrintText( "OK! Need click!" );
							return 1;
						}
						else
						{
							//	PrintText( "NO Need click!" );
						}
					}
				}
			}
		}
	}
	return 0;
}

int __fastcall SimpleButtonClickEvent_my( int pButton, int unused, int ClickEventType )
{

	if ( ClickEventType == 1 && CheckBtnForClickPortrain( pButton ) )
	{
		//PrintText( "Abil id found in list." );
		int retval = SimpleButtonClickEvent_ptr( pButton, unused, ClickEventType );
		int PortraitButtonAddr = GetGlobalClassAddr( );
		if ( PortraitButtonAddr > 0 )
		{
			PortraitButtonAddr = *( int* )( PortraitButtonAddr + 0x3F4 );
		}
		if ( PortraitButtonAddr > 0 )
		{
			//PrintText( "Click to portrain." );
			Wc3ControlClickButton_org( PortraitButtonAddr, 1 );
		}
		return retval;
	}

	////PrintText( "Click SimpleButton." );
	//if ( IsCommandButton( pButton ) )
	//{
	//	//PrintText( "SimpleButton IsCommandButton" );
	//	int CommandButtonData = *( int* )( pButton + 0x190 );
	//	if ( CommandButtonData )
	//	{
	//		//PrintText( "Click command button." );
	//		int pAbil = *( int* )( CommandButtonData + 0x6D4 );
	//		if ( pAbil )
	//		{
	//			//PrintText( "Abil found." );
	//			int pAbilId = *( int* )( pAbil + 0x34 );
	//			if ( pAbilId )
	//			{
	//				//PrintText( "Abil id found." );
	//				for ( auto tmpClick : ClickPortrainForIdList )
	//				{
	//					if ( pAbilId == tmpClick.abilid && !IsKeyPressed( tmpClick.keycode ) )
	//					{
	//						
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	return SimpleButtonClickEvent_ptr( pButton, unused, ClickEventType );
}


BOOL __stdcall SimpleButtonClick( int simplebtnaddr, BOOL LeftMouse )
{
	return SimpleButtonClickEvent_org( simplebtnaddr, simplebtnaddr, LeftMouse ? 1 : 4 );
}

BOOL PressSkillPanelButton( int idx, BOOL RightClick )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	int button = GetSkillPanelButton( idx );
	if ( button > 0 && IsCommandButton( button ) )
	{
		UINT oldflag = *( UINT * )( button + flagsOffset );
		if ( !( oldflag & 2 ) )
			*( UINT * )( button + flagsOffset ) = oldflag | 2;
		int retval = SimpleButtonClickEvent_org( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
		return retval;
	}
	return 0;
}

BOOL PressItemPanelButton( int idx, BOOL RightClick )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	int button = GetItemPanelButton( idx );
	if ( button > 0 && IsCommandButton( button ) )
	{
		UINT oldflag = *( UINT * )( button + flagsOffset );
		if ( !( oldflag & 2 ) )
			*( UINT * )( button + flagsOffset ) = oldflag | 2;
		int retval = SimpleButtonClickEvent_org( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
		return retval;
	}
	return 0;
}

BOOL PressHeroPanelButton( int idx, BOOL RightClick )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	int button = GetHeroButton( idx );
	if ( button > 0 && IsCommandButton( button ) )
	{
		UINT oldflag = *( UINT * )( button + flagsOffset );
		if ( !( oldflag & 2 ) )
			*( UINT * )( button + flagsOffset ) = oldflag | 2;
		int retval = SimpleButtonClickEvent_org( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
		return retval;
	}
	return 0;
}

BOOL IsMouseOverWindow( RECT pwi, POINT cursorPos )
{
	return PtInRect( &pwi, cursorPos );
}

vector<unsigned char> SendKeyEvent;

auto t_start = std::chrono::high_resolution_clock::now( );

BOOL LOCK_MOUSE_IN_WINDOW = FALSE;

int __stdcall LockMouseInWindow( BOOL enable )
{
	LOCK_MOUSE_IN_WINDOW = enable;

	if ( !LOCK_MOUSE_IN_WINDOW )
		ClipCursor( 0 );

	return enable;
}


BOOL BlockKeyboardAndMouseWhenTeleport = FALSE;

int __stdcall TeleportHelper( BOOL enabled )
{
	BlockKeyboardAndMouseWhenTeleport = enabled;
	return enabled;
}

vector<int> WhiteListForTeleport;


BOOL TeleportShiftPress = FALSE;

int __stdcall TeleportShiftKey( BOOL enabled )
{
	TeleportShiftPress = enabled;
	return enabled;
}

int __stdcall TeleportWhiteListKey( int VK )
{
	if ( VK == 0 && !WhiteListForTeleport.empty( ) )
		WhiteListForTeleport.clear( );
	WhiteListForTeleport.push_back( VK );
	return VK;
}

BOOL SingleShift = FALSE;

BOOL ShopHelperEnabled = FALSE;

int __stdcall ShopHelper( BOOL enable )
{
	ShopHelperEnabled = enable;
	return enable;
}

int ChatEditBoxVtable = 0;


BOOL IsChatActive( )
{
	return  *( int* )pCurrentFrameFocusedAddr  && **( int** )pCurrentFrameFocusedAddr == GameDll + ChatEditBoxVtable;
}

BOOL IsGameFrameActive( )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif


	if ( IsChatActive( ) )
	{
		return FALSE;
	}

	if ( *( int* )pCurrentFrameFocusedAddr == 0 )
	{
		return TRUE;
	}


	int pGlAddr = GetGlobalClassAddr( );
	if ( pGlAddr > 0 )
	{
		pGlAddr = *( int* )( pGlAddr + 0x3D0 );
		if ( pGlAddr > 0 )
		{
			pGlAddr = *( int* )( pGlAddr + 0x164 );
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
			return pGlAddr > 0;
		}
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	return FALSE;
}


BOOL rawimage_skipmouseevent = TRUE;

int __stdcall RawImage_SkipMouseClick( BOOL enabled )
{
	rawimage_skipmouseevent = enabled;
	return rawimage_skipmouseevent;
}

BOOL AutoSelectHero = FALSE;

int __stdcall SetAutoSelectHero( BOOL enabled )
{
	AutoSelectHero = TRUE;
	return AutoSelectHero;
}



safevector<DelayedPress> DelayedPressList;


void DelayedPressList_pushback( DelayedPress & d )
{
	for ( unsigned int i = 0; i < DelayedPressList.size( ); i++ )
	{
		if ( DelayedPressList[ i ].IsNull( ) )
		{
			DelayedPressList[ i ] = d;
			DelayedPressList[ i ].ISNULL = FALSE;

			return;
		}
	}
	DelayedPressList.push_back( d );
}

int PRESSRIGHTMOUSEIFCURSORTARGETWORK = 0;

DWORD WINAPI PRESSRIGHTMOUSEIFCURSORTARGET( LPVOID )
{
	while ( PRESSRIGHTMOUSEIFCURSORTARGETWORK )
	{
		Sleep( 5 );
	}
	PRESSRIGHTMOUSEIFCURSORTARGETWORK++;

	int MaxWaitTime = 15;

	while ( !IsCursorSelectTarget( ) && ( --MaxWaitTime ) )
	{
		Sleep( 1 );
		MaxWaitTime--;
	}

	if ( MaxWaitTime )
	{
	//	SetTlsForMe( );

		POINT cursorhwnd;
		GetCursorPos( &cursorhwnd );
		ScreenToClient( Warcraft3Window, &cursorhwnd );

		WarcraftRealWNDProc_ptr( Warcraft3Window, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM( cursorhwnd.x, cursorhwnd.y ) );
		WarcraftRealWNDProc_ptr( Warcraft3Window, WM_RBUTTONUP, 0, MAKELPARAM( cursorhwnd.x, cursorhwnd.y ) );

		//__asm
		//{
		//	mov ecx, GameDll;
		//	add ecx, 0xAB4F80;
		//	mov ecx, [ ecx ];
		//	mov edi, GameDll;
		//	add edi, 0x2FB920; //  clear cursor
		//	call edi;
		//}
	}

	PRESSRIGHTMOUSEIFCURSORTARGETWORK--;
	return 0;
}

void PressKeyWithDelay_timed( )
{
	if ( IsGame( ) && *IsWindowActive )
	{
		for ( unsigned int i = 0; i < DelayedPressList.size( ); i++ )
		{
			if ( DelayedPressList[ i ].IsNull( ) )
				continue;

			if ( DelayedPressList[ i ].TimeOut >= 20 )
			{
				DelayedPressList[ i ].TimeOut -= 20;
			}
			else if ( DelayedPressList[ i ].TimeOut > 0 )
			{
				DelayedPressList[ i ].TimeOut = 0;
			}
			else
			{
				if ( !DelayedPressList[ i ].IsCustom )
				{
					if ( DelayedPressList[ i ].NeedPressMsg == 0 )
					{
						WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, DelayedPressList[ i ].NeedPresswParam, DelayedPressList[ i ].NeedPresslParam );
						WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, DelayedPressList[ i ].NeedPresswParam, ( LPARAM )( 0xC0000000 | DelayedPressList[ i ].NeedPresslParam ) );
					}
					else
					{
						WarcraftRealWNDProc_ptr( Warcraft3Window, DelayedPressList[ i ].NeedPressMsg, DelayedPressList[ i ].NeedPresswParam, DelayedPressList[ i ].NeedPresslParam );
					}

				}
				else
				{

					for ( KeyActionStruct & keyAction : KeyActionList )
					{
						if ( keyAction.VK == ( int )DelayedPressList[ i ].NeedPresswParam && IsGameFrameActive( ) )
						{

							if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift )
								|| ( keyAction.IsAlt && DelayedPressList[ i ].IsAlt )
								|| ( keyAction.IsCtrl && DelayedPressList[ i ].IsCtrl )
								|| ( keyAction.IsShift && DelayedPressList[ i ].IsCustom )
								)
							{
								int selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) );

								int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
								if ( selectedunit > 0 && selectedunitcout > 0 )
								{
									int unitowner = GetUnitOwnerSlot( selectedunit );
									if ( unitowner != 15 )
									{
										BOOL PressedButton = FALSE;

										if ( IsNULLButtonFound( GetSkillPanelButton( 11 ) ) )
										{
											if ( keyAction.altbtnID >= 0 )
											{
												if ( !( DelayedPressList[ i ].NeedPresslParam & 0x40000000 ) )
												{
													if ( keyAction.IsSkill )
														PressedButton = PressSkillPanelButton( keyAction.altbtnID, keyAction.IsRightClick );
													else
														PressedButton = PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
												}

											}
										}
										else
										{
											if ( !( DelayedPressList[ i ].NeedPresslParam & 0x40000000 ) )
											{
												if ( keyAction.IsSkill )
													PressedButton = PressSkillPanelButton( keyAction.btnID, keyAction.IsRightClick );
												else
													PressedButton = PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
												//PressedButton = TRUE;
											}

										}

										if ( !PressedButton )
										{
											if ( SetInfoObjDebugVal )
											{
												PrintText( "NO ButtonPressed!" );
											}
										}


										if ( keyAction.IsQuickCast && PressedButton )
										{
											if ( SetInfoObjDebugVal )
											{
												PrintText( "IsQuickCast && ButtonPressed!" );
											}


											/*int x = ( int )( *GetWindowXoffset );
											int y = ( int )( *GetWindowYoffset );
*/
											POINT cursorhwnd;
											GetCursorPos( &cursorhwnd );
											ScreenToClient( Warcraft3Window, &cursorhwnd );

											WarcraftRealWNDProc_ptr( Warcraft3Window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( cursorhwnd.x, cursorhwnd.y ) );
											WarcraftRealWNDProc_ptr( Warcraft3Window, WM_LBUTTONUP, 0, MAKELPARAM( cursorhwnd.x, cursorhwnd.y ) );

											HANDLE thr = CreateThread( 0, 0, PRESSRIGHTMOUSEIFCURSORTARGET, 0, 0, 0 );
											if ( thr != NULL )
												CloseHandle( thr );

											/*
											if ( SetInfoObjDebugVal )
											{
												int button = GetSkillPanelButton( 11 );
												PrintText( "ButtonAddr:" + to_string( button ) + ". IsCursorSelectTarget: " + to_string( IsCursorSelectTarget( ) ) );
											}*/

											/*	if ( IsCursorSelectTarget( ) )
												{
													PressSkillPanelButton( 11, FALSE );
												}*/

												//POINT cursor;
												//GetCursorPos( &cursor );

												//x = x - cursorhwnd.x;
												//y = y - cursorhwnd.y;

												//cursor.x = cursor.x + x;
												//cursor.y = cursor.y + y;
												////( toXX, toYY );

												//MouseClick( cursor.x, cursor.y );
										}
										else if ( !keyAction.IsQuickCast )
										{
											if ( SetInfoObjDebugVal )
											{
												PrintText( "Skip quick cast: no flag!" );
											}
										}
										else if ( !PressedButton )
										{
											if ( SetInfoObjDebugVal )
											{
												PrintText( "Skip quick cast: Button not pressed!" );
											}
										}


										if ( PressedButton )
											break;
									}

								}
								break;
							}



						}
					}

				}
				DelayedPressList[ i ].ISNULL = TRUE;

			}

		}


	}

}


BOOL IsNumpadPressed( int VK )
{
	return VK == VK_NUMPAD1 ||
		VK == VK_NUMPAD2 ||
		VK == VK_NUMPAD4 ||
		VK == VK_NUMPAD5 ||
		VK == VK_NUMPAD7 ||
		VK == VK_NUMPAD8;
}

// | 0 | 1
// | 2 | 3
// | 4 | 5

//int __stdcall GetItemPanelButton( int idx )

int GetBtnIdByNumpad( int VK )
{
	switch ( VK )
	{
	case VK_NUMPAD1:
		return  4;
	case VK_NUMPAD2:
		return 5;
	case VK_NUMPAD4:
		return  2;
	case VK_NUMPAD5:
		return 3;
	case VK_NUMPAD7:
		return 0;
	case VK_NUMPAD8:
		return  1;
	default:
		break;
	}

	return -1;
}
int GetBtnAddrByNumpad( int VK )
{
	int btnid = GetBtnIdByNumpad( VK );
	if ( btnid == -1 )
		return 0;

	return GetItemPanelButton( GetBtnIdByNumpad( VK ) );
}

WPARAM LatestPressedKey = NULL;

POINTS GlobalMousePos = { 0,0 };

//BOOL DebugMsgShow = FALSE;

bool InitTestValues = false;
unsigned int TestValues[ 10 ];



unsigned int __stdcall GetTestValue( int id )
{
	if ( id >= 0 && id <= 7 )
	{
		return TestValues[ id ];
	}
	return 0;
}

BOOL ForceLvl1 = FALSE;
BOOL ForceLvl2 = FALSE;
BOOL ForceLvl3 = FALSE;

void __stdcall SetForceHotkeyProcess( BOOL lvl1, BOOL lvl2, BOOL lvl3 )
{
	ForceLvl1 = lvl1;
	ForceLvl2 = lvl2;
	ForceLvl3 = lvl3;

}

// �������� ����� � ������ 
bool replaceAll( std::string& str, const std::string& from, const std::string& to, int addtofrom = 0 )
{
	if ( from.empty( ) )
		return false;
	bool replaced = false;
	size_t start_pos = 0;
	while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos ) {
		replaced = true;
		str.replace( start_pos, from.length( ) + addtofrom, to );
		start_pos += to.length( ); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return replaced;
}


std::string ReturnStringWithoutWarcraftTags( std::string str )
{
	if ( str.length( ) <= 1 )
		return str;
	replaceAll( str, "|c", "", 8 );
	replaceAll( str, "|n", "" );
	replaceAll( str, "|r", "" );
	replaceAll( str, "|", "" );
	replaceAll( str, "- [Level ", "[" );
	return str;
}


std::string ReturnStringBeforeFirstChar( std::string str, char crepl )
{
	if ( str.length( ) <= 1 )
		return str;

	for ( auto & c : str )
	{
		if ( c == crepl )
		{
			c = '\0';
			break;
		}
	}
	return str;
}

std::string ReturnStringBeforeFirstString( std::string str, char crepl )
{
	if ( str.length( ) <= 1 )
		return str;

	for ( auto & c : str )
	{
		if ( c == crepl )
		{
			c = '\0';
			break;
		}
	}
	return str;
}

std::string GetObjectNameByID( int clid )
{
	if ( clid > 0 )
	{
		int tInfo = GetTypeInfo( clid, 0 );
		int tInfo_1d, tInfo_2id;
		if ( tInfo && ( tInfo_1d = *( int * )( tInfo + 40 ) ) != 0 )
		{
			tInfo_2id = tInfo_1d - 1;
			if ( tInfo_2id >= ( unsigned int )0 )
				tInfo_2id = 0;
			return ( const char * )*( int * )( *( int * )( tInfo + 44 ) + 4 * tInfo_2id );
		}
		else
		{
			return "Default String";
		}
	}
	return "Default String";
}

std::string IsCooldownMessage = "%s > On cooldown ( %02i:%02i ).";
std::string IsCooldownAndNoMana = "%s > On cooldown and no mana.";
std::string IsReadyMessage = "%s > is ready.";
std::string WantToLearnMessage = "I want to %s";
std::string WantToPickMessage = "I want to pick > %s";
std::string NeedToChooseMessage = "We need > %s";


std::string WantToBuyMessage = "I want to buy > %s";
std::string NeedToBuyMessage = "We need to buy > %s";


std::string ItemAbiltPlayerHasItem = "%s has > %s";
std::string NeedMoreMana = "Need %i mana for > %s";
std::string IgotSilence = "Can't use %s while silenced";


int ignorelist[ ] = { 1,2,3 };

std::vector<ObjInfoAction> IgnoreObjInfo;

void __stdcall SetCustomDefaultMessage( int id, const char * message )
{
	if ( id < 1 || id > 10 || message == NULL )
		return;

	switch ( id )
	{
	case 1:
		IsCooldownMessage = message;
		break;
	case 2:
		IsReadyMessage = message;
		break;
	case 3:
		WantToLearnMessage = message;
		break;
	case 4:
		WantToPickMessage = message;
		break;
	case 5:
		NeedToChooseMessage = message;
		break;
	case 6:
		WantToBuyMessage = message;
		break;
	case 7:
		NeedToBuyMessage = message;
		break;
	case 8:
		ItemAbiltPlayerHasItem = message;
		break;
	case 9:
		NeedMoreMana = message;
		break;
	case 10:
		IgotSilence = message;
		break;
	default:
		break;
	}

}

/*
 action

-1 ignore click
0 skip message
1 IsCooldownMessage
2 IsReadyMessage
3 WantToLearnMessage
4 WantToPickMessage
5 WantToBuyMessage
6 ItemAbiltPlayerHasItem
7 CustomMessage ( custommessage )
8 CustomPrintTextMessage ( custommessage )

*/

/*
ObjId

ObjId = CommandData + 4 offset
ObjId2 = CommandData + 8 offset

OrderId
AbilId
ItemId
*/
void __stdcall AddInfoObjCodeCustomAction( int ObjId, int ObjId2, int action, const char * custommessage )
{
	if ( ObjId == 0 && ObjId2 == 0 && action == 0 )
	{
		IgnoreObjInfo.clear( );
	}

	for ( auto & s : IgnoreObjInfo )
	{
		if ( s.ObjId == ObjId && s.ObjId2 == ObjId2 )
		{
			s.Action = action;
			s.CustomMessage = custommessage ? custommessage : "";
			return;
		}
	}
	IgnoreObjInfo.push_back( ObjInfoAction( ObjId, ObjId2, action, custommessage ? custommessage : "" ) );

}


std::vector< int > InfoWhitelistedObj;

void __stdcall AddInfoWhiteListedObj( int ObjId )
{
	if ( ObjId == 0 )
	{
		InfoWhitelistedObj.clear( );
	}

	InfoWhitelistedObj.push_back( ObjId );

}





void __stdcall SetInfoObjDebug( BOOL debug )
{
	SetInfoObjDebugVal = debug;
}

int GetAbilityManacost( int pAbil )
{
	if ( pAbil )
	{
		int vtable = *( int * )pAbil;
		if ( vtable )
		{
			int GetAbilMPcostAddr = *( int* )( vtable + 0x3d4 );
			if ( GetAbilMPcostAddr )
			{
				auto GetAbilMPcost = ( int( __thiscall * )( int ) )( GetAbilMPcostAddr );
				return GetAbilMPcost( pAbil );
			}
		}
	}
	return 0;
}



pSimpleButtonPreClickEvent SimpleButtonPreClickEvent_org;
pSimpleButtonPreClickEvent SimpleButtonPreClickEvent_ptr;
int __fastcall SimpleButtonPreClickEvent_my( int pButton, int unused, int a2 )
{

	/*__try
	{*/
	bool incooldown = false;
	std::string incooldownmessage = "";

	int selectedunit = 0;
	char PrintAbilState[ 2048 ];
	PrintAbilState[ 0 ] = '\0';
	if ( pButton && ( IsKeyPressed( VK_LMENU ) || ( IsKeyPressed( VK_LMENU ) && IsKeyPressed( VK_LCONTROL ) ) ) && IsCommandButton( pButton ) && ( selectedunit = GetSelectedUnit( GetLocalPlayerId( ) ) ) )
	{
		int CommandButtonData = *( int* )( pButton + 0x190 );
		if ( CommandButtonData )
		{
			//CONSOLE_Print( "Command button" );
			int pObjId = *( int* )( CommandButtonData + 4 );
			int pItemUnitID = *( int* )( CommandButtonData + 8 );
			const char * pAbilTitle = ( const char * )( CommandButtonData + 0x2C );
			std::string AbilName = /*ReturnStringBeforeFirstChar(*/ ReturnStringWithoutWarcraftTags( pAbilTitle ? pAbilTitle : "" )/*, '(' )*/;
			//	AbilName = ReturnStringBeforeFirstChar( AbilName, '[' );
			if ( !pObjId )
			{
				AbilName = ReturnStringBeforeFirstChar( AbilName, '(' );
			}
			int pAbil = *( int* )( CommandButtonData + 0x6D4 );
			//bool AbilFound = pAbil > 0 ;
			int pObjId_1 = *( int* )( CommandButtonData + 0x6F8 );
			int pObjId_2 = *( int* )( CommandButtonData + 0x6FC );
			int pBtnFlag = *( int* )( CommandButtonData + 0x5BC );


			int unitownerslot = GetUnitOwnerSlot( ( int )selectedunit );
			int localplayeridslot = GetLocalPlayerId( );


			int UnitMana = 0;
			int AbilManacost = 0;

			if ( SetInfoObjDebugVal )
			{
				char buttoninfo[ 256 ];

				sprintf_s( buttoninfo, " Command button %X data -> %X\n Object id: %X \nObject id2: %X \nObject id2: %X \n Abil addr: %X \n Title :%s \n Manacost: %i \n Unit Mana: %i", pButton, CommandButtonData, pObjId, pItemUnitID, pObjId_1, pAbil, ( pAbilTitle ? pAbilTitle : " no " ), AbilManacost, UnitMana );


				PrintText( buttoninfo );
			}

			if ( pObjId && !pAbil )
			{
				unsigned int abilscount = 0;
				int * abils = FindUnitAbils( selectedunit, &abilscount, pObjId );
				if ( abilscount > 0 )
				{
					pAbil = abils[ 0 ];
				}
			}

			if ( pAbil )
			{
				int pAbilId = *( int* )( pAbil + 0x34 );
				if ( pAbilId )
				{
					if ( AbilName.length( ) <= 2 )
					{
						AbilName = ReturnStringWithoutWarcraftTags( GetObjectNameByID( pAbilId ) );
					}
				}
			}


			if ( *( unsigned int * )( pButton + 0x140 ) & 16 || ( std::find( InfoWhitelistedObj.begin( ), InfoWhitelistedObj.end( ), pObjId ) != InfoWhitelistedObj.end( ) ) )
			{
				if ( pObjId != 'AHer' && pObjId != 'Asel'&& pObjId != 'Asud'&& pObjId != 'Asid' && pObjId != 'Aneu'
					&& pObjId != 0x77123477 && pObjId != 0x07123407 && pObjId != 0x77000077 )
				{
					if ( pAbil || pObjId )
					{
						if ( localplayeridslot == unitownerslot )
						{
							if ( pAbil && *( int* )( pAbil + 0x54 ) )
							{
								pObjId = *( int* )( pAbil + 0x54 );
								pObjId = *( int* )( pObjId + 0x34 );
							}
							if ( ( std::find( InfoWhitelistedObj.begin( ), InfoWhitelistedObj.end( ), pObjId ) != InfoWhitelistedObj.end( ) )
								|| ( pObjId != 'AHer' && pObjId != 'Asel'&& pObjId != 'Asud'&& pObjId != 'Asid'&& pObjId != 'Aneu' ) )
							{

								unsigned int abilscount = 0;
								int * abils = FindUnitAbils( selectedunit, &abilscount, pObjId );
								if ( abilscount > 0 )
								{
									AbilManacost = GetAbilityManacost( abils[ 0 ] );
								}
							}
						}
					}
				}
			}


			if ( AbilManacost )
			{
				float fUnitMana = 0.0f;
				GetUnitFloatState( selectedunit, &fUnitMana, 2 );
				UnitMana = ( int )fUnitMana;
			}


			if ( SetInfoObjDebugVal && AbilManacost == 0 )
			{
				PrintText( "Ignore mana request for button" );
			}


			for ( auto & s : IgnoreObjInfo )
			{
				int tmpObj1 = 0;
				if ( pAbil && *( int* )( pAbil + 0x54 ) )
				{
					tmpObj1 = *( int* )( pAbil + 0x54 );
					tmpObj1 = *( int* )( tmpObj1 + 0x30 );
				}
				if ( ( s.ObjId == pObjId || s.ObjId == tmpObj1 || s.ObjId == 0 ) && ( s.ObjId2 == pObjId || s.ObjId2 == pObjId_1 || s.ObjId2 == pItemUnitID || s.ObjId2 == 0 ) )
				{
					switch ( s.Action )
					{
					case -1:
						return 0;
					case 1:
						sprintf_s( PrintAbilState, IsCooldownMessage.c_str( ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					case 2:
						sprintf_s( PrintAbilState, IsReadyMessage.c_str( ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					case 3:
						sprintf_s( PrintAbilState, WantToLearnMessage.c_str( ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					case 4:
						if ( !IsKeyPressed( VK_LCONTROL ) )
							sprintf_s( PrintAbilState, WantToPickMessage.c_str( ), AbilName.c_str( ) );
						else
							sprintf_s( PrintAbilState, NeedToChooseMessage.c_str( ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					case 5:
						if ( !IsKeyPressed( VK_LCONTROL ) )
							sprintf_s( PrintAbilState, WantToBuyMessage.c_str( ), AbilName.c_str( ) );
						else
							sprintf_s( PrintAbilState, NeedToBuyMessage.c_str( ), AbilName.c_str( ) );

						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					case 6:
						sprintf_s( PrintAbilState, ItemAbiltPlayerHasItem.c_str( ), GetPlayerName( unitownerslot, 1 ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					case 7:
						sprintf_s( PrintAbilState, ( s.CustomMessage.length( ) > 0 ? s.CustomMessage.c_str( ) : AbilName.c_str( ) ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					case 8:
						PrintText( ( s.CustomMessage.length( ) > 0 ? s.CustomMessage.c_str( ) : AbilName.c_str( ) ) );
						return 0;
					default:
						return SimpleButtonPreClickEvent_ptr( pButton, unused, a2 );
					}
				}
			}


			if ( AbilName.length( ) > 1 &&
				( pItemUnitID == 0xD0142
					||
					( ( pBtnFlag != 2 && ( pObjId != 'AHer' || pObjId_1 != 0 ) ) && ( pAbil || pObjId || ( pAbilTitle[ 0 ] != '\0' && localplayeridslot != unitownerslot ) ) )
					||
					( std::find( InfoWhitelistedObj.begin( ), InfoWhitelistedObj.end( ), pObjId ) != InfoWhitelistedObj.end( ) ) ) )
			{
				if ( SetInfoObjDebugVal )
				{
					PrintText( "Click Button 1" );
				}


				if ( unitownerslot != localplayeridslot && unitownerslot != 15 )
				{
					sprintf_s( PrintAbilState, ItemAbiltPlayerHasItem.c_str( ), GetPlayerName( unitownerslot, 1 ), AbilName.c_str( ) );
				}
				else
				{
					sprintf_s( PrintAbilState, IsReadyMessage.c_str( ), AbilName.c_str( ) );
				}

				if ( unitownerslot == localplayeridslot )
				{
					if ( pBtnFlag != 2 && pObjId == 'AHer' )
					{
						sprintf_s( PrintAbilState, WantToLearnMessage.c_str( ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					}

				}

				if ( pObjId == 'Asel' )
				{
					//PrintText( "Sell Button" );
					char * pObjectIdToStr = ( char * )&pItemUnitID;
					if ( isupper( pObjectIdToStr[ 3 ] ) )
					{
						//	PrintText( "Sell hero" );
						if ( !IsKeyPressed( VK_LCONTROL ) )
							sprintf_s( PrintAbilState, WantToPickMessage.c_str( ), AbilName.c_str( ) );
						else
							sprintf_s( PrintAbilState, NeedToChooseMessage.c_str( ), AbilName.c_str( ) );
					}
					else
					{
						//PrintText( "Sell item" );
						if ( !IsKeyPressed( VK_LCONTROL ) )
							sprintf_s( PrintAbilState, WantToBuyMessage.c_str( ), AbilName.c_str( ) );
						else
							sprintf_s( PrintAbilState, NeedToBuyMessage.c_str( ), AbilName.c_str( ) );

					}


					if ( AbilManacost > UnitMana )
					{
						sprintf_s( PrintAbilState, NeedMoreMana.c_str( ), ( AbilManacost - UnitMana ), AbilName.c_str( ) );
						/*SendMessageToChat( PrintAbilState, 0 );
						return 0;*/
					}

					SendMessageToChat( PrintAbilState, 0 );
					return 0;
				}


				if ( pAbil && unitownerslot == localplayeridslot )
				{
					//CONSOLE_Print( 
					if ( SetInfoObjDebugVal )
					{
						PrintText( "Click Button owner ability!" );
					}

					int pAbilId = *( int* )( pAbil + 0x34 );
					if ( pAbilId && ( *( unsigned int * )( pButton + 0x140 ) & 16 || ( std::find( InfoWhitelistedObj.begin( ), InfoWhitelistedObj.end( ), pObjId ) != InfoWhitelistedObj.end( ) ) ) )
					{
						if ( pObjId != 'AHer' && pObjId != 'Asel'&& pObjId != 'Asud'&& pObjId != 'Asid' && pObjId != 'Aneu' )
						{
							//if ( !pObjId )
							//	AbilName = ReturnStringBeforeFirstChar( ReturnStringWithoutWarcraftTags( pAbilTitle ), '(' );
							//else


							if ( SetInfoObjDebugVal )
							{
								PrintText( "Search cooldown " );
							}


							int pAbilData = *( int* )( pAbil + 0xDC );
							if ( pAbilData )
							{
								if ( SetInfoObjDebugVal )
								{
									PrintText( "Found 1" );
								}


								float pAbilDataVal1 = *( float* )( pAbilData + 0x4 );
								int pAbilDataVal2tmp = *( int* )( pAbilData + 0xC );
								if ( pAbilDataVal1 > 0.0f && pAbilDataVal2tmp )
								{
									if ( SetInfoObjDebugVal )
									{
										PrintText( "Found 2" );
									}
									float pAbilDataVal2 = *( float* )( pAbilDataVal2tmp + 0x40 );
									float AbilCooldown = pAbilDataVal1 - pAbilDataVal2;
									int AbilCooldownMinutes = ( int )( AbilCooldown / 60.0f );
									int AbilCooldownSeconds = ( int )( ( int )AbilCooldown % 60 );

									incooldown = true;
									sprintf_s( PrintAbilState, IsCooldownMessage.c_str( ), AbilName.c_str( ), AbilCooldownMinutes, AbilCooldownSeconds );
									incooldownmessage = PrintAbilState;

								}
								else
								{
									if ( SetInfoObjDebugVal )
									{
										PrintText( "No cooldown Button 1.1" );
									}
									sprintf_s( PrintAbilState, IsReadyMessage.c_str( ), AbilName.c_str( ) );
								}
							}
							else
							{
								if ( SetInfoObjDebugVal )
								{
									PrintText( "No cooldown Button 1.2" );
								}
								sprintf_s( PrintAbilState, IsReadyMessage.c_str( ), AbilName.c_str( ) );
							}
						}
					}
					if ( *( int* )( pAbil + 0x3C ) > 0 )
					{
						sprintf_s( PrintAbilState, IgotSilence.c_str( ), AbilName.c_str( ) );
						SendMessageToChat( PrintAbilState, 0 );
						return 0;
					}

				}
				if ( AbilManacost > UnitMana )
				{
					if ( incooldown )
						sprintf_s( PrintAbilState, IsCooldownAndNoMana.c_str( ), AbilName.c_str( ) );
					else
						sprintf_s( PrintAbilState, NeedMoreMana.c_str( ), ( AbilManacost - UnitMana ), AbilName.c_str( ) );
				}
				else if ( incooldown )
				{
					sprintf_s( PrintAbilState, "%s", incooldownmessage.c_str( ) );
				}
				SendMessageToChat( PrintAbilState, 0 );
				return 0;
			}
			else if ( pAbil )
			{
				if ( SetInfoObjDebugVal )
				{
					PrintText( "Click Button 2" );
				}

				if ( *( int* )( pAbil + 0x3C ) > 0 )
				{
					sprintf_s( PrintAbilState, IgotSilence.c_str( ), AbilName.c_str( ) );
				}
				SendMessageToChat( PrintAbilState, 0 );
				return 0;
			}

		}

	}

	//}
	//__except ( g_crashRpt.SendReport( GetExceptionInformation( ) ) )
	//{
	//	::ExitProcess( 0 ); // It is better to stop the process here or else corrupted data may incomprehensibly crash it later.
	//	return 0;
	//}
	return SimpleButtonPreClickEvent_ptr( pButton, unused, a2 );
}



typedef float( __fastcall * pGetCameraHeight/*sub_6F3019A0*/ )( unsigned int a1 );
pGetCameraHeight GetCameraHeight_org;
pGetCameraHeight GetCameraHeight_ptr;

float cameraoffset = 0;


void IncreaseCameraOffset( float offset = 50 )
{
	if ( cameraoffset > 3000 )
		return;
	cameraoffset += offset;
}

void DecreaseCameraOffset( float offset = 50 )
{
	if ( cameraoffset < -1000 )
		return;
	cameraoffset -= offset;
}

void ResetCameraOffset( )
{
	cameraoffset = 0;
}

float __fastcall GetCameraHeight_my( unsigned int a1 )
{
	return GetCameraHeight_ptr( a1 ) + cameraoffset;
}


DWORD GroupSelectLastTime = GetTickCount( );
int LastSelectedGroupHandle = 0;

LRESULT __fastcall BeforeWarcraftWNDProc( HWND hWnd, unsigned int _Msg, WPARAM _wParam, LPARAM lParam )
{
	if ( !InitTestValues )
	{
		InitTestValues = true;
		memset( TestValues, 0, sizeof( TestValues ) );
	}
	TestValues[ 0 ]++;


	unsigned int Msg = _Msg;
	BOOL NeedSkipThisKey = FALSE;
	BOOL ClickHelperWork = FALSE;
	WPARAM wParam = _wParam;

	BOOL _IsCtrlPressed = _wParam & MK_CONTROL;
	BOOL _IsShiftPressed = _wParam & MK_SHIFT;
	BOOL _IsAltPressed = HIBYTE( GetKeyState( VK_MENU ) ) & 0x80;


	if ( _Msg == WM_SIZE )
	{
		for ( auto & v : ListOfRawImages )
			v.needResetTexture = TRUE;
	}

	// NEXT BLOCK ONLY FOR TEST!!!!
	//if ( Msg == WM_KEYDOWN && TestModeActivated )
	//{
	//	ShowConfigWindow( ".\\config.dota" );
	//}

	if ( SkipAllMessages || TerminateStarted )
	{
		return DefWindowProc( hWnd, Msg, wParam, lParam );// WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
	}


	//if ( Msg == WM_KEYDOWN && wParam == VK_ESCAPE )
	//{
	//	MessageBoxA( 0, "WMKEYDOWNESCAPE", "", 0 );
	//	if ( IsKeyPressed( '1' ) )
	//		return DefWindowProc( hWnd, Msg, wParam, lParam );
	//}



	//if ( Msg == WM_KEYUP && wParam == VK_ESCAPE )
	//{
	//	MessageBoxA( 0, "WMKEYUPESCAPE", "", 0 );
	//	if ( IsKeyPressed( '1' ) )
	//		return DefWindowProc( hWnd, Msg, wParam, lParam );
	//}

	//if ( !DebugMsgShow )
	//{
	//	DebugMsgShow = TRUE;
	//	if ( IsKeyPressed( VK_F1 ) )
	//	{
	//		char debugmsg[ 200 ];
	//		sprintf_s( debugmsg, "Current file cache size:%i", ICONMDLCACHELIST.size( ) );
	//		MessageBoxA( 0, debugmsg, debugmsg, 0 );
	//	}
	//	DebugMsgShow = FALSE;
	//}
#ifdef DOTA_HELPER_LOG
	if ( wParam == '0' && Msg == WM_KEYUP )
	{
		Storm::ShowAllLeaks( );
	}
#endif

	if ( !( IsGame( ) ) )
		return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );


	//if ( _Msg == WM_LBUTTONDOWN )
	//{
	//	if ( IsKeyPressed( VK_F1 ) )
	//	{
	//		char debugmsg[ 200 ];
	//		sprintf_s( debugmsg, "_Msg:%X WPARAM:%X LPARAM:%X", Msg, _wParam, lParam );
	//		MessageBoxA( 0, debugmsg, debugmsg, 0 );
	//	}
	//}



	TestValues[ 1 ]++;

	if ( GameVersion == 0x26a )
	{
		if ( _Msg == WM_MOUSEWHEEL && IsKeyPressed( VK_LCONTROL ) )
		{
			short wheeltarg = HIWORD( _wParam );
			if ( wheeltarg > 0 )
			{
				DecreaseCameraOffset( );
			}
			else
			{
				IncreaseCameraOffset( );
			}

			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_PRIOR, NULL );
			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_NEXT, NULL );

			return DefWindowProc( hWnd, Msg, wParam, lParam );
		}

		if ( _Msg == WM_MBUTTONDOWN && IsKeyPressed( VK_LCONTROL ) )
		{
			ResetCameraOffset( );

			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_PRIOR, NULL );
			WarcraftRealWNDProc_ptr( hWnd, WM_SYSKEYDOWN, VK_NEXT, NULL );

			return DefWindowProc( hWnd, Msg, wParam, lParam );
		}
	}

	if ( ( lParam & 0x40000000 ) && Msg == WM_KEYDOWN && !*( int* )ChatFound )
	{
		return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
	}


	TestValues[ 2 ]++;



#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif



	if ( Msg == WM_MOUSEMOVE )
	{
		GlobalMousePos = MAKEPOINTS( lParam );
	}



	//#ifdef DOTA_HELPER_LOG
	//	if ( Msg == WM_KEYUP && wParam == '0' )
	//	{
	//		ScanJassStringForErrors( 1 );
	//		char tmpst[ 100 ];
	//		sprintf_s( tmpst, "%i strings found", GetJassStringCount( 1 ) );
	//		MessageBoxA( 0, tmpst, tmpst, 0 );
	//	}
	//#endif


	//#ifdef DOTA_HELPER_LOG
	//if ( Msg == WM_KEYUP && wParam == '0' )
	//{
	//	int unitaddr = GetSelectedUnit( GetLocalPlayerId( ) );
	//	if ( unitaddr > 0 )
	//	{
	//		unsigned int abilscount = 0;
	//		int * abils = FindUnitAbils( unitaddr, &abilscount, 0, 0 );
	//		for ( unsigned int i = 0; i < abilscount; i++ )
	//		{
	//			char tmpdat[ 100 ];
	//			int pData = *( int* )( abils[ i ] + 0x54 );
	//			if ( pData != 0 )
	//			{
	//				sprintf_s( tmpdat, "A/A [%i]: %X / %X", i, *( int* )( pData + 0x30 ), *( int* )( pData + 0x34 ) );
	//			}
	//		}
	//	}
	//}
	//#endif


	if ( *IsWindowActive || ForceLvl2 )
	{
		TestValues[ 3 ]++;


		if ( WM_TIMER )
		{
			switch ( wParam )
			{
			case 'atod':
				PressKeyWithDelay_timed( );
				break;
			}
		}

		if ( usedcustomframes && *( int* )ChatFound == 0 && IsGameFrameActive( ) )
		{
			*( int* )pCurrentFrameFocusedAddr = 0;
		}

		if ( Msg == WM_LBUTTONUP )
		{
			ProcessClickAtCustomFrames( );
		}
		if ( GlobalRawImageCallbackData )
		{
			if ( Msg == WM_LBUTTONUP )
			{
				GlobalRawImageCallbackData->IsLeftButton = TRUE;
				RawImageGlobalCallbackFunc( RawImageEventType::MouseUp, ( float )GlobalMousePos.x, ( float )GlobalMousePos.y );
			}

			if ( Msg == WM_LBUTTONDOWN )
			{
				GlobalRawImageCallbackData->IsLeftButton = TRUE;
				if ( RawImageGlobalCallbackFunc( RawImageEventType::MouseDown, ( float )GlobalMousePos.x, ( float )GlobalMousePos.y ) )
				{
					if ( SetInfoObjDebugVal )
					{
						PrintText( "Skip WM_LBUTTONDOWN" );
					}
					return DefWindowProc( hWnd, Msg, wParam, lParam );
				}
				else
				{
					if ( SetInfoObjDebugVal )
					{
						PrintText( "No Skip WM_LBUTTONDOWN" );
					}
				}
			}

			if ( Msg == WM_RBUTTONUP )
			{
				GlobalRawImageCallbackData->IsLeftButton = FALSE;
				RawImageGlobalCallbackFunc( RawImageEventType::MouseUp, ( float )GlobalMousePos.x, ( float )GlobalMousePos.y );
			}

			if ( Msg == WM_RBUTTONDOWN )
			{
				GlobalRawImageCallbackData->IsLeftButton = FALSE;
				if ( RawImageGlobalCallbackFunc( RawImageEventType::MouseDown, ( float )GlobalMousePos.x, ( float )GlobalMousePos.y ) )
					return DefWindowProc( hWnd, Msg, wParam, lParam );
			}

			if ( Msg == WM_MOUSEMOVE )
			{
				RawImageGlobalCallbackFunc( RawImageEventType::MouseMove, ( float )GlobalMousePos.x, ( float )GlobalMousePos.y );
			}
		}

		if ( LOCK_MOUSE_IN_WINDOW )
		{
			POINT p;
			tagWINDOWINFO pwi;
			if ( Warcraft3Window && GetCursorPos( &p ) && GetWindowInfo( Warcraft3Window, &pwi ) && IsMouseOverWindow( pwi.rcClient, p ) )
			{
				ClipCursor( &pwi.rcClient );
			}
			else
			{
				ClipCursor( 0 );
			}
		}


		if ( FPS_LIMIT_ENABLED )
		{
			auto t_end = std::chrono::high_resolution_clock::now( );
			if ( std::chrono::duration<float, std::milli>( t_end - t_start ).count( ) > 250.0 )
			{
				t_start = t_end;
				UpdateFPS( );
			}
		}

		if ( AutoSelectHero )
			if ( Msg == WM_KEYDOWN && wParam >= VK_F1 && wParam <= VK_F5 )
			{
				LPARAM lpFKEYScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( wParam, 0 ) << 16 ) );
				LPARAM lpFKEYScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( wParam, 0 ) << 16 ) );

				WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, wParam, lpFKEYScanKeyDOWN );
				WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, wParam, lpFKEYScanKeyUP );
			}


		if ( ( Msg == WM_KEYDOWN || Msg == WM_KEYUP ) && ( _wParam == VK_SHIFT || _wParam == VK_LSHIFT || _wParam == VK_RSHIFT ) )
		{
			ShiftPressed = ( unsigned char )( Msg == WM_KEYDOWN ? 0x1u : 0x0u );
		}

		if ( Msg == WM_RBUTTONDOWN )
		{
			ShiftPressed = ( unsigned char )( _IsShiftPressed ? 0x1u : 0x0u );
		}

		// SHIFT+NUMPAD TRICK
		if ( IsGameFrameActive( ) && ( Msg == WM_KEYDOWN || Msg == WM_KEYUP ) && (
			wParam == 0xC ||
			wParam == 0x23 ||
			wParam == 0x24 ||
			wParam == 0x25 ||
			wParam == 0x26 ||
			wParam == 0x28
			) )
		{
			int  scanCode = ( int )( ( lParam >> 24 ) & 0x1 );

			if ( scanCode != 1 )
			{
				switch ( wParam )
				{
				case 0x23:
					wParam = VK_NUMPAD1;
					break;
				case 0x28:
					wParam = VK_NUMPAD2;
					break;
				case 0x25:
					wParam = VK_NUMPAD4;
					break;
				case 0xC:
					wParam = VK_NUMPAD5;
					break;
				case 0x24:
					wParam = VK_NUMPAD7;
					break;
				case 0x26:
					wParam = VK_NUMPAD8;
					break;
				default:
					break;
				}
				if ( wParam != _wParam )
				{
					if ( !_IsShiftPressed )
					{
						BOOL NumLock = ( ( ( unsigned short )GetKeyState( VK_NUMLOCK ) ) & 0xffff ) != 0;
						if ( NumLock )
							ShiftPressed = 0x1;
						else
							ShiftPressed = 0x0;
					}


					if ( Msg == WM_KEYDOWN && IsNumpadPressed( wParam ) )
					{
						bool NotFoundInHotKeys = true;
						for ( KeyActionStruct & keyAction : KeyActionList )
						{
							if ( keyAction.VK == wParam )
							{
								NotFoundInHotKeys = false;
							}
						}

						if ( NotFoundInHotKeys )
						{
							int btnaddr = GetBtnAddrByNumpad( wParam );
							if ( btnaddr )
							{
								SimpleButtonClickEvent_my( btnaddr, 0, 1 );
								/*if ( CheckBtnForClickPortrain( btnaddr ) )
								{
									SimpleButtonClickEvent_my( btnaddr, 0, 1 );*/
								return DefWindowProc( hWnd, Msg, wParam, lParam );

								//int PortraitButtonAddr = GetGlobalClassAddr( );
								//if ( PortraitButtonAddr > 0 )
								//{
								//	PortraitButtonAddr = *( int* )( PortraitButtonAddr + 0x3F4 );
								//}
								//if ( PortraitButtonAddr > 0 )
								//{
								//	//PrintText( "Click to portrain." );
								//	Wc3ControlClickButton_org( PortraitButtonAddr, 1 );
								//}
							//}
							}
						}
					}
					LRESULT retval1 = WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );



					return retval1;
				}

			}
			else
			{
				if ( !_IsShiftPressed )
				{
					ShiftPressed = 0;
				}
			}
		}


		if ( Msg == WM_KEYDOWN && IsNumpadPressed( wParam ) && IsGameFrameActive( ) )
		{
			bool NotFoundInHotKeys = true;
			for ( KeyActionStruct & keyAction : KeyActionList )
			{
				if ( keyAction.VK == wParam )
				{
					NotFoundInHotKeys = false;
				}
			}

			if ( NotFoundInHotKeys )
			{
				int btnaddr = GetBtnAddrByNumpad( wParam );
				if ( btnaddr )
				{
					SimpleButtonClickEvent_my( btnaddr, 0, 1 );
					return DefWindowProc( hWnd, Msg, wParam, lParam );
				}
			}
		}



		for ( unsigned int i = 0; i < SkipMessagesList.size( ); i++ )
		{
			if ( SkipMessagesList[ i ].Msg == Msg && SkipMessagesList[ i ].wParam == wParam )
			{
				SkipMessagesList.erase( SkipMessagesList.begin( ) + ( int )i );
				return DefWindowProc( hWnd, Msg, wParam, lParam );
			}
		}

		if ( Msg == WM_MOUSEMOVE && BLOCKMOUSEMOVING )
		{
			return DefWindowProc( hWnd, Msg, wParam, lParam );
		}

		TestValues[ 4 ]++;



		if ( ( *( int* )ChatFound == 0 || ForceLvl3 ) && ( IsGameFrameActive( ) || ForceLvl1 ) )
		{
			TestValues[ 5 ]++;

			if ( Msg == WM_KEYDOWN || Msg == WM_KEYUP || Msg == WM_RBUTTONDOWN || Msg == WM_RBUTTONUP )
			{
				for ( int & keyCode : RegisteredKeyCodes )
				{
					if ( keyCode == ( int )wParam )
					{

						if ( Msg == WM_KEYDOWN /*&& !( lParam & 0x40000000 )*/ )
						{

#ifdef DOTA_HELPER_LOG
							AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
							//BytesToSend.push_back( 0x50 );
							//// packet header
							//BytesToSend.push_back( 0xFF );
							//// packet size
							//BytesToSend.push_back( 0 );
							//BytesToSend.push_back( 0 );
							//BytesToSend.push_back( 0 );
							//BytesToSend.push_back( 0 );
							SendKeyEvent.push_back( 0x50 );
							// header custom packets
							SendKeyEvent.push_back( 0xFF );
							// size custom packets 
							SendKeyEvent.push_back( 0 );
							SendKeyEvent.push_back( 0 );
							SendKeyEvent.push_back( 0 );
							SendKeyEvent.push_back( 0 );
							// packet type
							int packettype = 'IKEY';
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&packettype, ( ( unsigned char * )&packettype ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							// data
							int locpid = GetLocalPlayerId( );
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&locpid, ( ( unsigned char * )&locpid ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&Msg, ( ( unsigned char * )&Msg ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&wParam, ( ( unsigned char * )&wParam ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							SendPacket( ( BYTE* )&SendKeyEvent[ 0 ], SendKeyEvent.size( ) );
							SendKeyEvent.clear( );
#ifdef DOTA_HELPER_LOG
							AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
							//*KeyboardAddrForKey = ( int ) wParam;
							//*KeyboardAddrForKeyEvent = ( int ) Msg;
							//	TriggerExecute( KeyboardTriggerHandle );
						}
						else if ( Msg == WM_KEYUP )
						{
#ifdef DOTA_HELPER_LOG
							AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
							SendKeyEvent.push_back( 0x50 );
							// header custom packets
							SendKeyEvent.push_back( 0xFF );
							// size custom packets 
							SendKeyEvent.push_back( 0 );
							SendKeyEvent.push_back( 0 );
							SendKeyEvent.push_back( 0 );
							SendKeyEvent.push_back( 0 );
							// packet type
							int packettype = 'IKEY';
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&packettype, ( ( unsigned char * )&packettype ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							// data
							int locpid = GetLocalPlayerId( );
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&locpid, ( ( unsigned char * )&locpid ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&Msg, ( ( unsigned char * )&Msg ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&wParam, ( ( unsigned char * )&wParam ) + 4 );
							*( int* )&SendKeyEvent[ 2 ] += 4;
							SendPacket( ( BYTE* )&SendKeyEvent[ 0 ], SendKeyEvent.size( ) );
							SendKeyEvent.clear( );
#ifdef DOTA_HELPER_LOG
							AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
							//*KeyboardAddrForKey = ( int ) wParam;
							//*KeyboardAddrForKeyEvent = ( int ) Msg;
							//TriggerExecute( KeyboardTriggerHandle );
						}
#ifdef DOTA_HELPER_LOG
						AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
						return DefWindowProc( hWnd, Msg, wParam, lParam );
					}

				}
			}
			//char keystateprint[ 200 ];
			if ( Msg == WM_KEYDOWN ||/* Msg == WM_KEYUP || */Msg == WM_RBUTTONDOWN )
			{
				if ( BlockKeyboardAndMouseWhenTeleport )
				{
					if ( Msg == WM_RBUTTONDOWN )
					{
						int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
						if ( selectedunit > 0 )
						{
							unsigned int abilscount = 0;
							FindUnitAbils( selectedunit, &abilscount, 'A3VO' );
							if ( abilscount > 0 )
							{
								if ( TeleportShiftPress )
								{
									if ( ShiftPressed == 0 && !ShiftPressed )
									{
										SingleShift = TRUE;
										ShiftPressed = 1;
									}
								}
								else
									return DefWindowProc( hWnd, Msg, wParam, lParam );
							}
						}
					}



					if ( ( wParam >= 0x41 && wParam <= 0x5A ) || ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 ) )
					{
						BOOL NeedSkipForTP = TRUE;

						for ( int & VK : WhiteListForTeleport )
						{
							if ( wParam == VK )
							{
								NeedSkipForTP = FALSE;
								break;
							}
						}

						if ( NeedSkipForTP )
						{
							int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
							if ( selectedunit > 0 )
							{
								unsigned int abilscount = 0;
								FindUnitAbils( selectedunit, &abilscount, 'A3VO' );
								if ( abilscount > 0 )
								{
									if ( TeleportShiftPress )
									{
										if ( ShiftPressed == 0 && !ShiftPressed )
										{
											SingleShift = TRUE;
											ShiftPressed = 1;
										}

									}
									else
										return DefWindowProc( hWnd, Msg, wParam, lParam );
								}
							}
						}

					}

				}
			}

			if ( Msg == WM_KEYDOWN || Msg == WM_XBUTTONDOWN || Msg == WM_MBUTTONDOWN ||
				Msg == WM_SYSKEYDOWN )
			{
				BOOL itempressed = FALSE;

				if ( _Msg == WM_XBUTTONDOWN )
				{
					Msg = WM_KEYDOWN;
					wParam = _wParam & MK_XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
				}

				if ( _Msg == WM_MBUTTONDOWN )
				{
					Msg = WM_KEYDOWN;
					wParam = VK_MBUTTON;
				}

				if ( ShopHelperEnabled  && IsGameFrameActive( ) && /*(*/ Msg == WM_KEYDOWN /*|| Msg == WM_KEYUP ) */ )
				{

					if (
						wParam == 'Q' ||
						wParam == 'W' ||
						wParam == 'E' ||
						wParam == 'R' ||
						wParam == 'A' ||
						wParam == 'S' ||
						wParam == 'D' ||
						wParam == 'F' ||
						wParam == 'Z' ||
						wParam == 'X' ||
						wParam == 'C' ||
						wParam == 'V'
						)
					{
						int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
						if ( selectedunit > 0 && GetSelectedUnitCountBigger( GetLocalPlayerId( ) ) > 0 )
						{
							if ( GetUnitOwnerSlot( selectedunit ) == 15 )
							{
								// | 0 | 3 | 6 | 9  |
								// | 1 | 4 | 7 | 10 | 
								// | 2 | 5 | 8 | 11 |

								NeedSkipThisKey = TRUE;

								/*	if ( Msg == WM_KEYDOWN && !( lParam & 0x40000000 ) )
									{*/

								if ( wParam == 'Q' )
									PressSkillPanelButton( 0, FALSE );
								else if ( wParam == 'W' )
									PressSkillPanelButton( 3, FALSE );
								else if ( wParam == 'E' )
									PressSkillPanelButton( 6, FALSE );
								else if ( wParam == 'R' )
									PressSkillPanelButton( 9, FALSE );
								else if ( wParam == 'A' )
									PressSkillPanelButton( 1, FALSE );
								else if ( wParam == 'S' )
									PressSkillPanelButton( 4, FALSE );
								else if ( wParam == 'D' )
									PressSkillPanelButton( 7, FALSE );
								else if ( wParam == 'F' )
									PressSkillPanelButton( 10, FALSE );
								else if ( wParam == 'Z' )
									PressSkillPanelButton( 2, FALSE );
								else if ( wParam == 'X' )
									PressSkillPanelButton( 5, FALSE );
								else if ( wParam == 'C' )
									PressSkillPanelButton( 8, FALSE );
								else if ( wParam == 'V' )
									PressSkillPanelButton( 11, FALSE );
								else
									NeedSkipThisKey = FALSE;
								//}
							}
						}
					}
				}


				if ( !NeedSkipThisKey && IsGameFrameActive( ) )
				{
					TestValues[ 7 ] = KeyActionList.size( );

					for ( int lol = 0; lol < 2; lol++ )
					{
						//if ( !NeedSkipThisKey )
						{
							for ( KeyActionStruct & keyAction : KeyActionList )
							{
								if ( keyAction.VK == ( int )wParam )
								{
									if ( SetInfoObjDebugVal )
									{
										PrintText( "Hotkey vk code found." );
									}
									TestValues[ 6 ]++;
									if ( Msg == WM_SYSKEYDOWN )
										Msg = WM_KEYDOWN;

									/*if ( Msg == WM_SYSKEYUP )
										Msg = WM_KEYUP;*/



									if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift && lol == 1 )
										|| ( keyAction.IsAlt && _IsAltPressed )
										|| ( keyAction.IsCtrl && _IsCtrlPressed )
										|| ( keyAction.IsShift && _IsShiftPressed )
										)
									{

										lol++;
										if ( SetInfoObjDebugVal )
										{
											PrintText( "Hotkey availabled!" );
										}
										itempressed = !keyAction.IsSkill;


										int selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) );

										int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
										if ( selectedunit > 0 && selectedunitcout > 0 )
										{
											int unitowner = GetUnitOwnerSlot( selectedunit );
											if ( !keyAction.IsSkill && ClickHelper && unitowner != 15 )
											{
												if ( wParam == LatestPressedKey )
												{
													if ( IsCursorSelectTarget( ) )
													{
														if ( SetInfoObjDebugVal )
														{
															PrintText( "Select hero!" );
														}
														if ( PressMouseAtSelectedHero( itempressed ) == 0 )
														{
															ClickHelperWork = TRUE;
															NeedSkipThisKey = TRUE;


														}
													}
												}
											}

											LatestPressedKey = wParam;


											if ( unitowner != 15 && !ClickHelperWork )
											{
												NeedSkipThisKey = TRUE;



												if ( EnableSelectHelper )
												{

													if ( unitowner != GetLocalPlayerId( ) && !GetPlayerAlliance( Player( unitowner ), Player( GetLocalPlayerId( ) ), 6 ) )
													{
														//	sprintf_s( keystateprint, 200, "Start emulate #2..." );
														//	PrintText( keystateprint );
															//PressHeroPanelButton( 0, FALSE );
														WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
														WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );
														if ( SetInfoObjDebugVal )
														{
															PrintText( "Hotkey delay press start!!" );
														}

														DelayedPress tmpDelayPress = DelayedPress( );
														tmpDelayPress.IsCustom = TRUE;
														tmpDelayPress.IsAlt = _IsAltPressed;
														tmpDelayPress.IsCtrl = _IsCtrlPressed;
														tmpDelayPress.IsShift = _IsShiftPressed;
														tmpDelayPress.NeedPresslParam = lParam;
														tmpDelayPress.NeedPresswParam = wParam;
														tmpDelayPress.NeedPressMsg = Msg;
														tmpDelayPress.TimeOut = 140;
														DelayedPressList_pushback( tmpDelayPress );

														//lol++;
														return DefWindowProc( hWnd, Msg, wParam, lParam );
													}
												}

												BOOL PressedButton = FALSE;

												if ( IsNULLButtonFound( GetSkillPanelButton( 11 ) ) )
												{
													if ( keyAction.altbtnID >= 0 )
													{
														if ( keyAction.IsSkill )
															PressedButton = PressSkillPanelButton( keyAction.altbtnID, keyAction.IsRightClick );
														else
															PressedButton = PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
														//PressedButton = TRUE;
													}
												}
												else
												{
													if ( keyAction.IsSkill )
														PressedButton = PressSkillPanelButton( keyAction.btnID, keyAction.IsRightClick );
													else
														PressedButton = PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
													//PressedButton = TRUE;
												}

												if ( !PressedButton )
												{
													if ( SetInfoObjDebugVal )
													{
														PrintText( "NO ButtonPressed!" );
													}
												}

												if ( keyAction.IsQuickCast && PressedButton )
												{
													if ( SetInfoObjDebugVal )
													{
														PrintText( "IsQuickCast && ButtonPressed!" );
													}
													int x = ( int )( *GetWindowXoffset );
													int y = ( int )( *GetWindowYoffset );

													POINT cursorhwnd;
													GetCursorPos( &cursorhwnd );
													ScreenToClient( Warcraft3Window, &cursorhwnd );

													WarcraftRealWNDProc_ptr( hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM( cursorhwnd.x, cursorhwnd.y ) );
													WarcraftRealWNDProc_ptr( hWnd, WM_LBUTTONUP, 0, MAKELPARAM( cursorhwnd.x, cursorhwnd.y ) );


													HANDLE thr = CreateThread( 0, 0, PRESSRIGHTMOUSEIFCURSORTARGET, 0, 0, 0 );
													if ( thr != NULL )
														CloseHandle( thr );


													/*	if ( SetInfoObjDebugVal )
														{
															int button = GetSkillPanelButton( 11 );
															PrintText( "ButtonAddr:" + to_string( button ) + ". IsCursorSelectTarget: " + to_string( IsCursorSelectTarget( ) ) );
														}

														if ( IsCursorSelectTarget( ) )
														{
															PressSkillPanelButton( 11, FALSE );
														}*/

														/*POINT cursor;
														GetCursorPos( &cursor );
	*/
	/*x = x - cursorhwnd.x;
	y = y - cursorhwnd.y;

	cursor.x = cursor.x + x;
	cursor.y = cursor.y + y;*/
	//( toXX, toYY );

	//`MouseClick( cursor.x, cursor.y );

	//SetCursorPos( cursor.x, cursor.y );
												}
												else if ( !keyAction.IsQuickCast )
												{
													if ( SetInfoObjDebugVal )
													{
														PrintText( "Skip quick cast: no flag!" );
													}
												}
												else if ( !PressedButton )
												{
													if ( SetInfoObjDebugVal )
													{
														PrintText( "Skip quick cast: Button not pressed!" );
													}
												}

												//if ( IsNULLButtonFound( GetSkillPanelButton( 11 ) ) )
												//{
												//	if ( keyAction.altbtnID >= 0 )
												//	{
												//		if ( SetInfoObjDebugVal )
												//		{
												//			PrintText( "OK. Now press panel button." );
												//		}
												//		//if ( !( lParam & 0x40000000 ) )
												//	//	{
												//		if ( keyAction.IsSkill )
												//			PressSkillPanelButton( keyAction.altbtnID, keyAction.IsRightClick );
												//		else
												//			PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
												//		//	}

												//	}
												//	else
												//	{
												//		if ( SetInfoObjDebugVal )
												//		{
												//			PrintText( "ERROR. NO ACTION FOUND!" );
												//		}
												//	}
												//}
												//else
												//{
												//	if ( SetInfoObjDebugVal )
												//	{
												//		PrintText( "OK. Now press panel button." );
												//	}
												//	//if ( !( lParam & 0x40000000 ) )
												////	{
												//	if ( keyAction.IsSkill )
												//		PressSkillPanelButton( keyAction.btnID, keyAction.IsRightClick );
												//	else
												//		PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
												//	//	}

												//}
											}
											else
											{
												if ( SetInfoObjDebugVal )
												{
													PrintText( "Bad selected unit( player 15 ) or hotkey disabled." );
												}

											}
										}
										else
										{
											if ( SetInfoObjDebugVal )
											{
												PrintText( "No selected unit!" );
											}

										}

										//lol++;
										break;
									}
									else
									{
										if ( SetInfoObjDebugVal )
										{
											PrintText( "Hotkey not equal skip..." );
										}

									}
								}
							}
						}

					}
					for ( int lol = 0; lol < 2; lol++ )
					{
						//if ( !NeedSkipThisKey )
						{


							for ( auto keyAction : KeyChatActionList )
							{
								if ( keyAction.VK == ( int )wParam )
								{

									if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift && lol == 1 )
										|| ( keyAction.IsAlt && _IsAltPressed )
										|| ( keyAction.IsCtrl && _IsCtrlPressed )
										|| ( keyAction.IsShift && _IsShiftPressed )
										)
									{

										lol++;
										NeedSkipThisKey = TRUE;
										if ( ( keyAction.IsAlt && _IsAltPressed )
											|| ( keyAction.IsCtrl && _IsCtrlPressed )
											|| ( keyAction.IsShift && _IsShiftPressed )
											)
										{
											if ( Msg == WM_SYSKEYDOWN )
												Msg = WM_KEYDOWN;
										}
										else
										{
											if ( _IsAltPressed
												|| _IsCtrlPressed )
											{
												NeedSkipThisKey = FALSE;
											}
										}


										SendMessageToChat( keyAction.Message.c_str( ), keyAction.SendToAll );
									}
								}
							}

						}
					}
					for ( int lol = 0; lol < 2; lol++ )
					{
						//if ( !NeedSkipThisKey )
						{
							int selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) );

							int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
							if ( selectedunit > 0 && selectedunitcout > 0 )
							{
								int unitowner = GetUnitOwnerSlot( selectedunit );
								if ( unitowner != 15 )
								{
									for ( auto keyAction : KeyCalbackActionList )
									{
										if ( keyAction.VK == ( int )wParam )
										{
											if ( SetInfoObjDebugVal )
											{
												PrintText( "Need keyAction!" );
											}

											if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift && lol == 1 )
												|| ( keyAction.IsAlt && _IsAltPressed )
												|| ( keyAction.IsCtrl && _IsCtrlPressed )
												|| ( keyAction.IsShift && _IsShiftPressed )
												)
											{
												lol++;
												NeedSkipThisKey = TRUE;
												if ( ( keyAction.IsAlt && _IsAltPressed )
													|| ( keyAction.IsCtrl && _IsCtrlPressed )
													|| ( keyAction.IsShift && _IsShiftPressed )
													)
												{
													if ( Msg == WM_SYSKEYDOWN )
														Msg = WM_KEYDOWN;
												}
												else
												{
													if ( _IsAltPressed
														|| _IsCtrlPressed )
													{
														NeedSkipThisKey = FALSE;
													}
												}

												( ( int( __thiscall * )( int, int, int, int, int, int, int ) )( GameDll + 0x37C420 ) )
													( keyAction.args[ 0 ], keyAction.args[ 1 ], keyAction.args[ 2 ],
														keyAction.args[ 3 ], keyAction.args[ 4 ], keyAction.args[ 5 ], keyAction.args[ 6 ] );
											}
										}
									}

								}
							}
						}
					}
					for ( int lol = 0; lol < 2; lol++ )
					{
						//if ( !NeedSkipThisKey )
						{
							for ( auto keyAction : KeySelectActionList )
							{
								if ( keyAction.VK == ( int )wParam )
								{

									if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift && lol == 1 )
										|| ( keyAction.IsAlt && _IsAltPressed )
										|| ( keyAction.IsCtrl && _IsCtrlPressed )
										|| ( keyAction.IsShift && _IsShiftPressed )
										)
									{

										lol++;
										NeedSkipThisKey = TRUE;
										if ( ( keyAction.IsAlt && _IsAltPressed )
											|| ( keyAction.IsCtrl && _IsCtrlPressed )
											|| ( keyAction.IsShift && _IsShiftPressed )
											)
										{
											if ( Msg == WM_SYSKEYDOWN )
												Msg = WM_KEYDOWN;
										}
										else
										{
											if ( _IsAltPressed
												|| _IsCtrlPressed )
											{
												NeedSkipThisKey = FALSE;
											}
										}

										if ( SetInfoObjDebugVal )
										{
											PrintText( "Clear selection" );
										}

										vector<int> units = GetUnitsFromGroup( keyAction.GroupHandle );
										//reverse( units.begin( ), units.end( ) );


										if ( SetInfoObjDebugVal )
										{
											PrintText( ( "Select units:" + uint_to_hex( units.size( ) ) ).c_str( ) );
										}


										if ( units.size( ) > 0 )
										{
											ClearSelection( );

											for ( int unit : units )
											{
												if ( SetInfoObjDebugVal )
												{
													PrintText( ( "Select unit:" + uint_to_hex( unit ) ).c_str( ) );
												}
												SelectUnit( unit );
											}


											if ( GetTickCount( ) - GroupSelectLastTime < 450 &&
												LastSelectedGroupHandle == keyAction.GroupHandle )
											{
												int PortraitButtonAddr = GetGlobalClassAddr( );
												if ( PortraitButtonAddr > 0 )
												{
													PortraitButtonAddr = *( int* )( PortraitButtonAddr + 0x3F4 );
												}
												if ( PortraitButtonAddr > 0 )
												{
													if ( SetInfoObjDebugVal )
													{
														PrintText( "double click to portrain." );
													}
													Wc3ControlClickButton_org( PortraitButtonAddr, 1 );
													Wc3ControlClickButton_org( PortraitButtonAddr, 1 );
												}
											}


										}

										LastSelectedGroupHandle = keyAction.GroupHandle;
										GroupSelectLastTime = GetTickCount( );
									}
								}
							}

						}
					}
				}

				if ( !NeedSkipThisKey )
				{


					/*	if ( _Msg == WM_XBUTTONDOWN
							|| _Msg == WM_MBUTTONDOWN )
						*/

						//	}

					for ( int & keyCode : BlockedKeyCodes )
					{
						if ( keyCode == ( int )wParam )
						{
#ifdef DOTA_HELPER_LOG
							AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
							return DefWindowProc( hWnd, Msg, wParam, lParam );
						}

					}

					Msg = _Msg;
					wParam = _wParam;


				}


				if ( ( wParam >= 0x41 && wParam <= 0x5A ) ||
					( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 )
					|| NeedSkipThisKey )
				{
					/*if ( ( wParam >= 0x41 && wParam <= 0x5A ) || ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 ) )
					{*/


#ifdef DOTA_HELPER_LOG
					AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif



					int selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) );
					int unitowner = selectedunitcout > 0 ? GetUnitOwnerSlot( GetSelectedUnit( GetLocalPlayerId( ) ) ) : 0;


					if ( selectedunitcout == 1 )
					{
						if ( EnableSelectHelper )
						{
							if ( selectedunitcout == 0 ||
								( unitowner != GetLocalPlayerId( ) && !GetPlayerAlliance( Player( unitowner ), Player( GetLocalPlayerId( ) ), 6 ) ) )
							{

								/*sprintf_s( processdoubleclic, "%s", "2" );
								PrintText( processdoubleclic );*/

								//PressHeroPanelButton( 0, FALSE );
								WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
								WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );


								if ( NeedSkipThisKey )
									return DefWindowProc( hWnd, Msg, wParam, lParam );


								DelayedPress tmpDelayPress;
								tmpDelayPress.NeedPresslParam = lParam;
								tmpDelayPress.NeedPresswParam = wParam;
								tmpDelayPress.NeedPressMsg = 0;
								tmpDelayPress.TimeOut = 60;
								DelayedPressList_pushback( tmpDelayPress );

#ifdef DOTA_HELPER_LOG
								AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif


								return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
							}
						}
					}


					if ( !ClickHelperWork && ClickHelper )
					{

						if ( LastPressedKeysTime[ wParam ] + 750 > GetTickCount( ) )
						{


							itempressed = itempressed || ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 );

							if ( IsCursorSelectTarget( ) )
							{
								if ( PressMouseAtSelectedHero( itempressed ) == 0 )
								{
									if ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 )
									{
										LastPressedKeysTime[ wParam ] = 0;
#ifdef DOTA_HELPER_LOG
										AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
										return DefWindowProc( hWnd, Msg, wParam, lParam );
									}
								}
							}


						}
						else
							LastPressedKeysTime[ wParam ] = GetTickCount( );
					}


#ifdef DOTA_HELPER_LOG
					AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

				}
			}

			if ( NeedSkipThisKey )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
				return DefWindowProc( hWnd, Msg, wParam, lParam );
			}

			if ( Msg == WM_LBUTTONDOWN )
			{
				oldlParam = lParam;
			}


			if ( Msg == WM_RBUTTONDOWN )
			{
				if ( EnableSelectHelper )
				{
					int selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) );
					int unitowner = selectedunitcout > 0 ? GetUnitOwnerSlot( GetSelectedUnit( GetLocalPlayerId( ) ) ) : 0;


					if ( selectedunitcout == 0 ||
						( unitowner != GetLocalPlayerId( ) && ( unitowner == 15 || !GetPlayerAlliance( Player( unitowner ), Player( GetLocalPlayerId( ) ), 6 ) ) )
						)
					{
						//PressHeroPanelButton( 0, FALSE );
						WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
						WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );
#ifdef DOTA_HELPER_LOG
						AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
					}
				}
			}
		}

	}
	else
	{

		// Process RawImages. Mouse up and leave;
		RawImageGlobalCallbackFunc( RawImageEventType::ALL, 0.0f, 0.0f );

		if ( LOCK_MOUSE_IN_WINDOW )
			ClipCursor( 0 );

		if ( BlockKeyAndMouseEmulation )
		{
			if ( Msg == WM_RBUTTONDOWN || Msg == WM_KEYDOWN || Msg == WM_KEYUP )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
				return DefWindowProc( hWnd, Msg, wParam, lParam );
			}
		}
	}


#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
}



int __stdcall ToggleBlockKeyAndMouseEmulation( BOOL enable )
{
	BlockKeyAndMouseEmulation = enable;
	return 0;
}


int __stdcall ToggleForcedSubSelection( BOOL enable )
{
	EnableSelectHelper = enable;
	return 0;
}

int __stdcall ToggleClickHelper( BOOL enable )
{
	ClickHelper = enable;
	return 0;
}

typedef int( __stdcall * IssueWithoutTargetOrder )( int a1, int a2, unsigned int a3, unsigned int a4 );
typedef int( __stdcall * IssueTargetOrPointOrder2 )( int a1, int a2, float a3, float a4, int a5, int a6 );
typedef int( __stdcall * sub_6F339D50 )( int a1, int a2, int a3, unsigned int a4, unsigned int a5 );
typedef int( __stdcall * IssueTargetOrPointOrder )( int a1, int a2, float a3, float a4, int a5, int a6, int a7 );
typedef int( __stdcall * sub_6F339E60 )( int a1, int a2, float a3, float a4, int a5, int a6, int a7, int a8 );
typedef int( __stdcall * sub_6F339F00 )( int a1, int a2, int a3, unsigned int a4, unsigned int a5 );
typedef int( __stdcall * sub_6F339F80 )( int a1, int a2, float a3, float a4, int a5, int a6, int a7 );
typedef int( __stdcall * sub_6F33A010 )( int a1, int a2, float a3, float a4, int a5, int a6, int a7, int a8 );


IssueWithoutTargetOrder IssueWithoutTargetOrderorg;
IssueWithoutTargetOrder IssueWithoutTargetOrderptr;

IssueTargetOrPointOrder2 IssueTargetOrPointOrder2org;
IssueTargetOrPointOrder2 IssueTargetOrPointOrder2ptr;

sub_6F339D50 sub_6F339D50org;
sub_6F339D50 sub_6F339D50ptr;

IssueTargetOrPointOrder IssueTargetOrPointOrderorg;
IssueTargetOrPointOrder IssueTargetOrPointOrderptr;

sub_6F339E60 sub_6F339E60org;
sub_6F339E60 sub_6F339E60ptr;

sub_6F339F00 sub_6F339F00org;
sub_6F339F00 sub_6F339F00ptr;

sub_6F339F80 sub_6F339F80org;
sub_6F339F80 sub_6F339F80ptr;

sub_6F33A010 sub_6F33A010org;
sub_6F33A010 sub_6F33A010ptr;


int __stdcall IssueWithoutTargetOrdermy( int a1, int a2, unsigned int a3, unsigned int a4 )
{
	int retvalue = IssueWithoutTargetOrderptr( a1, a2, a3, ( ( a4 & ShiftPressed ) > 0 ) ? a4 : a4 | ShiftPressed );

	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}

	return retvalue;
}
int __stdcall IssueTargetOrPointOrder2my( int a1, int a2, float a3, float a4, int a5, int a6 )
{
	int retvalue = IssueTargetOrPointOrder2ptr( a1, a2, a3, a4, a5, ( ( a6 & ShiftPressed ) > 0 ) ? a6 : a6 | ShiftPressed );
	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}
	return retvalue;
}
int __stdcall sub_6F339D50my( int a1, int a2, int a3, unsigned int a4, unsigned int a5 )
{
	int retvalue = sub_6F339D50ptr( a1, a2, a3, a4, ( ( a5 & ShiftPressed ) > 0 ) ? a5 : a5 | ShiftPressed );
	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}
	return retvalue;
}
int __stdcall IssueTargetOrPointOrdermy( int a1, int a2, float a3, float a4, int a5, int a6, int a7 )
{
	int retvalue = IssueTargetOrPointOrderptr( a1, a2, a3, a4, a5, a6, ( ( a7 & ShiftPressed ) > 0 ) ? a7 : a7 | ShiftPressed );
	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}
	return retvalue;
}
int __stdcall sub_6F339E60my( int a1, int a2, float a3, float a4, int a5, int a6, int a7, int a8 )
{
	int retvalue = sub_6F339E60ptr( a1, a2, a3, a4, a5, a6, a7, ( ( a8 & ShiftPressed ) > 0 ) ? a8 : a8 | ShiftPressed );
	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}
	return retvalue;
}
int __stdcall sub_6F339F00my( int a1, int a2, int a3, unsigned int a4, unsigned int a5 )
{
	int retvalue = sub_6F339F00ptr( a1, a2, a3, a4, ( ( a5 & ShiftPressed ) > 0 ) ? a5 : a5 | ShiftPressed );
	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}
	return retvalue;
}
int __stdcall sub_6F339F80my( int a1, int a2, float a3, float a4, int a5, int a6, int a7 )
{
	int retvalue = sub_6F339F80ptr( a1, a2, a3, a4, a5, a6, ( ( a7 & ShiftPressed ) > 0 ) ? a7 : a7 | ShiftPressed );
	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}
	return retvalue;
}
int __stdcall sub_6F33A010my( int a1, int a2, float a3, float a4, int a5, int a6, int a7, int a8 )
{
	int retvalue = sub_6F33A010ptr( a1, a2, a3, a4, a5, a6, a7, ( ( a8 & ShiftPressed ) > 0 ) ? a8 : a8 | ShiftPressed );
	if ( SingleShift )
	{
		SingleShift = FALSE;
		ShiftPressed = 0;
	}
	return retvalue;
}

int IssueWithoutTargetOrderOffset = 0;
int IssueTargetOrPointOrder2Offset = 0;
int sub_6F339D50Offset = 0;
int IssueTargetOrPointOrderOffset = 0;
int sub_6F339E60Offset = 0;
int sub_6F339F00Offset = 0;
int sub_6F339F80Offset = 0;
int sub_6F33A010Offset = 0;

void IssueFixerInit( )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif


	IssueWithoutTargetOrderorg = ( IssueWithoutTargetOrder )( GameDll + IssueWithoutTargetOrderOffset );
	MH_CreateHook( IssueWithoutTargetOrderorg, &IssueWithoutTargetOrdermy, reinterpret_cast< void** >( &IssueWithoutTargetOrderptr ) );

	IssueTargetOrPointOrder2org = ( IssueTargetOrPointOrder2 )( GameDll + IssueTargetOrPointOrder2Offset );
	MH_CreateHook( IssueTargetOrPointOrder2org, &IssueTargetOrPointOrder2my, reinterpret_cast< void** >( &IssueTargetOrPointOrder2ptr ) );

	sub_6F339D50org = ( sub_6F339D50 )( GameDll + sub_6F339D50Offset );
	MH_CreateHook( sub_6F339D50org, &sub_6F339D50my, reinterpret_cast< void** >( &sub_6F339D50ptr ) );

	IssueTargetOrPointOrderorg = ( IssueTargetOrPointOrder )( GameDll + IssueTargetOrPointOrderOffset );
	MH_CreateHook( IssueTargetOrPointOrderorg, &IssueTargetOrPointOrdermy, reinterpret_cast< void** >( &IssueTargetOrPointOrderptr ) );

	sub_6F339E60org = ( sub_6F339E60 )( GameDll + sub_6F339E60Offset );
	MH_CreateHook( sub_6F339E60org, &sub_6F339E60my, reinterpret_cast< void** >( &sub_6F339E60ptr ) );

	sub_6F339F00org = ( sub_6F339F00 )( GameDll + sub_6F339F00Offset );
	MH_CreateHook( sub_6F339F00org, &sub_6F339F00my, reinterpret_cast< void** >( &sub_6F339F00ptr ) );

	sub_6F339F80org = ( sub_6F339F80 )( GameDll + sub_6F339F80Offset );
	MH_CreateHook( sub_6F339F80org, &sub_6F339F80my, reinterpret_cast< void** >( &sub_6F339F80ptr ) );

	sub_6F33A010org = ( sub_6F33A010 )( GameDll + sub_6F33A010Offset );
	MH_CreateHook( sub_6F33A010org, &sub_6F33A010my, reinterpret_cast< void** >( &sub_6F33A010ptr ) );



	MH_EnableHook( IssueWithoutTargetOrderorg );
	MH_EnableHook( IssueTargetOrPointOrder2org );
	MH_EnableHook( sub_6F339D50org );
	MH_EnableHook( IssueTargetOrPointOrderorg );
	MH_EnableHook( sub_6F339E60org );
	MH_EnableHook( sub_6F339F00org );
	MH_EnableHook( sub_6F339F80org );
	MH_EnableHook( sub_6F33A010org );

	if ( GameVersion == 0x26a )
	{
		GetCameraHeight_org = pGetCameraHeight( GameDll + 0x3019A0 );
		MH_CreateHook( GetCameraHeight_org, &GetCameraHeight_my, reinterpret_cast< void** >( &GetCameraHeight_ptr ) );
		MH_EnableHook( GetCameraHeight_org );
	}


#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
}


void IssueFixerDisable( )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	memset( LastPressedKeysTime, 0, sizeof( LastPressedKeysTime ) );

	if ( IssueWithoutTargetOrderorg )
	{
		MH_DisableHook( IssueWithoutTargetOrderorg );
		IssueWithoutTargetOrderorg = NULL;
	}
	if ( IssueTargetOrPointOrder2org )
	{
		MH_DisableHook( IssueTargetOrPointOrder2org );
		IssueTargetOrPointOrder2org = NULL;
	}
	if ( sub_6F339D50org )
	{
		MH_DisableHook( sub_6F339D50org );
		sub_6F339D50org = NULL;
	}
	if ( IssueTargetOrPointOrderorg )
	{
		MH_DisableHook( IssueTargetOrPointOrderorg );
		IssueTargetOrPointOrderorg = NULL;
	}
	if ( sub_6F339E60org )
	{
		MH_DisableHook( sub_6F339E60org );
		sub_6F339E60org = NULL;
	}
	if ( sub_6F339F00org )
	{
		MH_DisableHook( sub_6F339F00org );
		sub_6F339F00org = NULL;
	}
	if ( sub_6F339F80org )
	{
		MH_DisableHook( sub_6F339F80org );
		sub_6F339F80org = NULL;
	}
	if ( sub_6F33A010org )
	{
		MH_DisableHook( sub_6F33A010org );
		sub_6F33A010org = NULL;
	}

	if ( GetCameraHeight_org )
	{
		MH_DisableHook( GetCameraHeight_org );
		GetCameraHeight_org = NULL;
	}

	if ( !RegisteredKeyCodes.empty( ) )
		RegisteredKeyCodes.clear( );

	if ( !BlockedKeyCodes.empty( ) )
		BlockedKeyCodes.clear( );

	if ( !KeyActionList.empty( ) )
		KeyActionList.clear( );

	if ( !KeyChatActionList.empty( ) )
		KeyChatActionList.clear( );

	if ( !KeySelectActionList.empty( ) )
		KeySelectActionList.clear( );

	if ( !KeyCalbackActionList.empty( ) )
		KeyCalbackActionList.clear( );

	SkipAllMessages = FALSE;
}