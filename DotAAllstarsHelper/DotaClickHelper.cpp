#include "Main.h"




HWND Warcraft3Window = 0;


WarcraftRealWNDProc WarcraftRealWNDProc_org = NULL;
WarcraftRealWNDProc WarcraftRealWNDProc_ptr;


LPARAM lpF1ScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_F1, 0 ) << 16 ) );
LPARAM lpF1ScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_F1, 0 ) << 16 ) );

LPARAM lpShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_SHIFT, 0 ) << 16 ) );
LPARAM lpShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_SHIFT, 0 ) << 16 ) );

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
	POINT cursorPos;
	GetCursorPos( &cursorPos );

	double dx = toX*( 65535.0f / DesktopScreen_Width );
	double dy = toY*( 65535.0f / DesktopScreen_Height );
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
	SetHeroFrameXY( );
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
		if ( IsItem || doubleclickSkillIDs.empty( ) ||
			std::find( doubleclickSkillIDs.begin( ), doubleclickSkillIDs.end( ), GetCursorSkillID( ) ) != doubleclickSkillIDs.end( )
			)
		{
			int PortraitButtonAddr = GetGlobalClassAddr( );
			if ( PortraitButtonAddr > 0 )
			{
				PortraitButtonAddr = *( int* )( PortraitButtonAddr + 0x3F4 );
			}
			if ( PortraitButtonAddr > 0 )
			{
				//	PrintText( "ALL OKAY" );
				Wc3ControlClickButton_org( PortraitButtonAddr, 1 );
			}
			else
			{
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




DWORD LastPressedKeysTime[ 256 ];


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
	tmpstr.altbtnID = ( GetAltBtnID( btnID ) );
	tmpstr.IsSkill = IsSkill;
	tmpstr.IsAlt = KeyCode & 0x10000;
	tmpstr.IsCtrl = KeyCode & 0x20000;
	tmpstr.IsShift = KeyCode & 0x40000;
	tmpstr.IsRightClick = KeyCode & 0x80000;

	if ( KeyCode & 0xF0000 )
	{
		for ( KeyActionStruct & curstr : KeyActionList )
		{
			if ( curstr.btnID == tmpstr.btnID )
			{
				curstr = tmpstr;
				return 0;
			}
		}
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
	return 0;
}

// | 0 | 1
// | 2 | 3
// | 4 | 5

int __stdcall GetItemPanelButton( int idx )
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
	ClickPortrainForIdList.push_back( tmpClickPortrainForId );

	return TRUE;
}

BOOL CheckBtnForClickPortrain( int pButton )
{
	if ( pButton && IsCommandButton( pButton ) )
	{
		//PrintText( "SimpleButton IsCommandButton" );
		int CommandButtonData = *( int* )( pButton + 0x190 );
		if ( CommandButtonData )
		{
			//PrintText( "Click command button." );
			int pAbil = *( int* )( CommandButtonData + 0x6D4 );
			if ( pAbil )
			{
				//PrintText( "Abil found." );
				int pAbilId = *( int* )( pAbil + 0x34 );
				if ( pAbilId )
				{
					//PrintText( "Abil id found." );
					for ( auto tmpClick : ClickPortrainForIdList )
					{
						if ( pAbilId == tmpClick.abilid && !IsKeyPressed( tmpClick.keycode ) )
						{
							return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}

int __fastcall SimpleButtonClickEvent_my( int pButton, int unused, int ClickEventType )
{
	if ( CheckBtnForClickPortrain( pButton ) )
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


void __stdcall SimpleButtonClick( int simplebtnaddr, BOOL LeftMouse )
{
	SimpleButtonClickEvent_org( simplebtnaddr, simplebtnaddr, LeftMouse ? 1 : 4 );
}

void PressSkillPanelButton( int idx, BOOL RightClick )
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
		SimpleButtonClickEvent_org( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
	}
}

void PressItemPanelButton( int idx, BOOL RightClick )
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
		SimpleButtonClickEvent_org( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
	}
}

void PressHeroPanelButton( int idx, BOOL RightClick )
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
		SimpleButtonClickEvent_org( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
	}
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

BOOL IsGameFrameActive( )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
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


void PressKeyWithDelay_timed( )
{
	if ( *InGame && *IsWindowActive )
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
						if ( keyAction.VK == ( int )DelayedPressList[ i ].NeedPresswParam )
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

										if ( IsNULLButtonFound( GetSkillPanelButton( 11 ) ) )
										{
											if ( keyAction.altbtnID >= 0 )
											{
												if ( !( DelayedPressList[ i ].NeedPresslParam & 0x40000000 ) )
												{
													if ( keyAction.IsSkill )
														PressSkillPanelButton( keyAction.altbtnID, keyAction.IsRightClick );
													else
														PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
													break;
												}

											}
										}
										else
										{
											if ( !( DelayedPressList[ i ].NeedPresslParam & 0x40000000 ) )
											{
												if ( keyAction.IsSkill )
													PressSkillPanelButton( keyAction.btnID, keyAction.IsRightClick );
												else
													PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
												break;
											}

										}
									}

								}
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


	// NEXT BLOCK ONLY FOR TEST!!!!
	//if ( Msg == WM_KEYDOWN && TestModeActivated )
	//{
	//	ShowConfigWindow( ".\\config.dota" );
	//}

	if ( SkipAllMessages || TerminateStarted )
	{
		return DefWindowProc( hWnd, Msg, wParam, lParam );// WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
	}


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

	if ( !*InGame )
		return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );

	TestValues[ 1 ]++;


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
					return DefWindowProc( hWnd, Msg, wParam, lParam );
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
			ShiftPressed = ( unsigned char )( IsKeyPressed( VK_SHIFT ) ? 0x1u : 0x0u );
		}

		// SHIFT+NUMPAD TRICK
		if ( ( Msg == WM_KEYDOWN || Msg == WM_KEYUP ) && (
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
					if ( !IsKeyPressed( VK_SHIFT ) )
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
				if ( !IsKeyPressed( VK_SHIFT ) )
				{
					ShiftPressed = 0;
				}
			}
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

				if ( ShopHelperEnabled && /*(*/ Msg == WM_KEYDOWN /*|| Msg == WM_KEYUP ) */ )
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


				if ( !NeedSkipThisKey )
				{
					TestValues[ 7 ] = KeyActionList.size( );


					for ( KeyActionStruct & keyAction : KeyActionList )
					{
						if ( keyAction.VK == ( int )wParam )
						{
							TestValues[ 6 ]++;
							if ( Msg == WM_SYSKEYDOWN )
								Msg = WM_KEYDOWN;

							/*if ( Msg == WM_SYSKEYUP )
								Msg = WM_KEYUP;*/



							if ( ( !keyAction.IsAlt && !keyAction.IsCtrl && !keyAction.IsShift )
								|| ( keyAction.IsAlt && IsKeyPressed( VK_MENU ) )
								|| ( keyAction.IsCtrl && IsKeyPressed( VK_CONTROL ) )
								|| ( keyAction.IsShift && IsKeyPressed( VK_SHIFT ) )
								)
							{

								itempressed = !keyAction.IsSkill;

								NeedSkipThisKey = TRUE;


								int selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) );

								int selectedunit = GetSelectedUnit( GetLocalPlayerId( ) );
								if ( selectedunit > 0 && selectedunitcout > 0 )
								{
									int unitowner = GetUnitOwnerSlot( selectedunit );
									if ( selectedunitcout == 1 && ( !keyAction.IsSkill || ClickHelper ) )
									{
										if ( wParam == LatestPressedKey )
										{
											if ( IsCursorSelectTarget( ) )
											{
												if ( PressMouseAtSelectedHero( itempressed ) == 0 )
													ClickHelperWork = TRUE;
											}
										}
									}

									LatestPressedKey = wParam;


									if ( unitowner != 15 && !ClickHelperWork )
									{


										if ( EnableSelectHelper )
										{

											if ( unitowner != GetLocalPlayerId( ) && !GetPlayerAlliance( Player( unitowner ), Player( GetLocalPlayerId( ) ), 6 ) )
											{
												//	sprintf_s( keystateprint, 200, "Start emulate #2..." );
												//	PrintText( keystateprint );
													//PressHeroPanelButton( 0, FALSE );
												WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
												WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );


												DelayedPress tmpDelayPress = DelayedPress( );
												tmpDelayPress.IsCustom = TRUE;
												tmpDelayPress.IsAlt = IsKeyPressed( VK_MENU );
												tmpDelayPress.IsCtrl = IsKeyPressed( VK_CONTROL );
												tmpDelayPress.IsShift = IsKeyPressed( VK_SHIFT );
												tmpDelayPress.NeedPresslParam = lParam;
												tmpDelayPress.NeedPresswParam = wParam;
												tmpDelayPress.NeedPressMsg = Msg;
												tmpDelayPress.TimeOut = 140;
												DelayedPressList_pushback( tmpDelayPress );

												return DefWindowProc( hWnd, Msg, wParam, lParam );
											}
										}


										if ( IsNULLButtonFound( GetSkillPanelButton( 11 ) ) )
										{
											if ( keyAction.altbtnID >= 0 )
											{
												//if ( !( lParam & 0x40000000 ) )
											//	{
												if ( keyAction.IsSkill )
													PressSkillPanelButton( keyAction.altbtnID, keyAction.IsRightClick );
												else
													PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
												break;
												//	}

											}
										}
										else
										{
											//if ( !( lParam & 0x40000000 ) )
										//	{
											if ( keyAction.IsSkill )
												PressSkillPanelButton( keyAction.btnID, keyAction.IsRightClick );
											else
												PressItemPanelButton( keyAction.btnID, keyAction.IsRightClick );
											break;
											//	}

										}
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

						if ( LastPressedKeysTime[ wParam ] + 600 > GetTickCount( ) )
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
		MH_DisableHook( IssueWithoutTargetOrderorg );
	if ( IssueTargetOrPointOrder2org )
		MH_DisableHook( IssueTargetOrPointOrder2org );
	if ( sub_6F339D50org )
		MH_DisableHook( sub_6F339D50org );
	if ( IssueTargetOrPointOrderorg )
		MH_DisableHook( IssueTargetOrPointOrderorg );
	if ( sub_6F339E60org )
		MH_DisableHook( sub_6F339E60org );
	if ( sub_6F339F00org )
		MH_DisableHook( sub_6F339F00org );
	if ( sub_6F339F80org )
		MH_DisableHook( sub_6F339F80org );
	if ( sub_6F33A010org )
		MH_DisableHook( sub_6F33A010org );

	if ( !RegisteredKeyCodes.empty( ) )
		RegisteredKeyCodes.clear( );
	if ( !BlockedKeyCodes.empty( ) )
		BlockedKeyCodes.clear( );
	if ( !KeyActionList.empty( ) )
		KeyActionList.clear( );

	SkipAllMessages = FALSE;
}