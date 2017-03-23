#include "Main.h"




HWND Warcraft3Window = 0;


WarcraftRealWNDProc WarcraftRealWNDProc_org = NULL;
WarcraftRealWNDProc WarcraftRealWNDProc_ptr;


LPARAM lpF1ScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_F1, 0 ) << 16 ) );
LPARAM lpF1ScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_F1, 0 ) << 16 ) );


HANDLE hPressKeyWithDelay = NULL;

BOOL PressKeyWithDelayEND = FALSE;

BOOL EmulateKeyInputForHWND = FALSE;


unsigned char ShiftPressed = 0;
BOOL SkipAllMessages = FALSE;


struct DelayedPress
{
	DWORD NeedPressMsg;
	WPARAM NeedPresswParam;
	LPARAM NeedPresslParam;
	DWORD TimeOut;
};


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


vector<DelayedPress> DelayedPressList;

DWORD WINAPI PressKeyWithDelay( LPVOID )
{
	while ( true && !PressKeyWithDelayEND )
	{
		Sleep( 20 );

		for ( unsigned int i = 0; i < DelayedPressList.size( ); i++ )
		{
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

				if ( DelayedPressList[ i ].NeedPressMsg == 0 )
				{
					//SkipAllMessages = TRUE;
				//	PressKeyboard( ( int )DelayedPressList[ i ].NeedPresswParam );
					WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, DelayedPressList[ i ].NeedPresswParam, DelayedPressList[ i ].NeedPresslParam );
					WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, DelayedPressList[ i ].NeedPresswParam, ( LPARAM )( 0xC0000000 | DelayedPressList[ i ].NeedPresslParam ) );
					//SkipAllMessages = FALSE;
				}
				else
				{


					WarcraftRealWNDProc_ptr( Warcraft3Window, DelayedPressList[ i ].NeedPressMsg, DelayedPressList[ i ].NeedPresswParam, DelayedPressList[ i ].NeedPresslParam );

				}
				DelayedPressList.erase( DelayedPressList.begin( ) + ( int )i );
				i--;
			}

		}


	}
	hPressKeyWithDelay = NULL;
	return 0;
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
	if ( skillID == 0 && !doubleclickSkillIDs.empty( ) )
	{
		doubleclickSkillIDs.clear( );
	}
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
	double screenRes_width = ::GetSystemMetrics( SM_CXSCREEN ) - 1;
	double screenRes_height = ::GetSystemMetrics( SM_CYSCREEN ) - 1;
	double dx = toX*( 65535.0f / screenRes_width );
	double dy = toY*( 65535.0f / screenRes_height );
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

	dx = cursorPos.x*( 65535.0f / screenRes_width );
	dy = cursorPos.y*( 65535.0f / screenRes_height );

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = LONG( dx );
	Input.mi.dy = LONG( dy );
	SendInput( 1, &Input, sizeof( INPUT ) );
	SetHeroFrameXY( );
}




DWORD WINAPI ThreadTest( void * lpp )
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
	CreateThread( 0, 0, ThreadTest, ClickPoint, 0, 0 );
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
int PressMouseAtSelectedHero( bool IsItem )
{
	int errorvalue = 0;
	if ( !IsCursorSelectTarget( ) )
		errorvalue = 1;
	if ( GetCursorOrder( ) == 0xD000F ||
		GetCursorOrder( ) == 0xD0012 ||
		GetCursorOrder( ) == 0xD0016 )
		errorvalue = 2;

	if ( IsCursorSelectTarget( ) &&
		GetCursorOrder( ) != 0xD000F &&
		GetCursorOrder( ) != 0xD0012 &&
		GetCursorOrder( ) != 0xD0016 )
	{
		if ( IsItem || doubleclickSkillIDs.empty( ) ||
			std::find( doubleclickSkillIDs.begin( ), doubleclickSkillIDs.end( ), GetCursorSkillID( ) ) != doubleclickSkillIDs.end( ) )
		{
			BOOL ButtonDown = FALSE;
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

			MouseClick( cursor.x, cursor.y );
		}
		else 	errorvalue = 3;

	}

	return errorvalue;
}


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


DWORD LastPressedKeysTime[ 256 ];


vector<int> RegisteredKeyCodes;
vector<int> BlockedKeyCodes;

struct KeyActionStruct
{
	int VK;
	int btnID;
	int altbtnID;
	BOOL IsSkill;
	BOOL IsShift;
	BOOL IsCtrl;
	BOOL IsAlt;
	BOOL IsRightClick;
};
vector<KeyActionStruct> KeyActionList;


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
	KeyActionStruct tmpstr2;
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
			if ( curstr.VK == tmpstr.VK )
			{
				if ( curstr.IsAlt ||
					curstr.IsCtrl ||
					curstr.IsShift )
					continue;


				tmpstr2 = curstr;
				curstr = tmpstr;
				tmpstr = tmpstr2;
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
	AddNewLineToDotaHelperLog( __func__ );
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
	AddNewLineToDotaHelperLog( __func__ );// by Karaulov
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


c_SimpleButtonClickEvent SimpleButtonClickEvent;

void PressSkillPanelButton( int idx, BOOL RightClick )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	int button = GetSkillPanelButton( idx );
	if ( button > 0 && *( int* )button > 0 )
	{
		UINT oldflag = *( UINT * )( button + flagsOffset );
		if ( !( oldflag & 2 ) )
			*( UINT * )( button + flagsOffset ) = oldflag | 2;
		SimpleButtonClickEvent( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
	}
}

void PressItemPanelButton( int idx, BOOL RightClick )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	int button = GetItemPanelButton( idx );
	if ( button > 0 && *( int* )button > 0 )
	{
		UINT oldflag = *( UINT * )( button + flagsOffset );
		if ( !( oldflag & 2 ) )
			*( UINT * )( button + flagsOffset ) = oldflag | 2;
		SimpleButtonClickEvent( button, 0, RightClick ? 4 : 1 );
		*( UINT * )( button + flagsOffset ) = oldflag;
	}
}

BOOL IsMouseOverWindow( RECT pwi, POINT cursorPos )
{
	return PtInRect( &pwi, cursorPos );
}

vector<unsigned int> SendKeyEvent;

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
	int pGlAddr = GetGlobalClassAddr( );
	if ( pGlAddr > 0 )
	{
		pGlAddr = *( int* )( pGlAddr + 0x3D0 );
		if ( pGlAddr > 0 )
		{
			pGlAddr = *( int* )( pGlAddr + 0x164 );
			return pGlAddr > 0;
		}
	}
	return FALSE;
}


WPARAM LatestPressedKey = NULL;

LRESULT __fastcall BeforeWarcraftWNDProc( HWND hWnd, unsigned int _Msg, WPARAM _wParam, LPARAM lParam )
{
	unsigned int Msg = _Msg;
	BOOL NeedSkipThisKey = FALSE;
	BOOL ClickHelperWork = FALSE;
	WPARAM wParam = _wParam;
	if ( SkipAllMessages || TerminateStarted )
	{
		return DefWindowProc( hWnd, Msg, wParam, lParam );// WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
	}

	if ( !*InGame )
		return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );


	//	if ( Msg == WM_KEYDOWN )
	//	{
	//		ShowConfigWindow( ".\\config.dota" );
	//	}

	if ( *( BOOL* )IsWindowActive )
	{
		if ( *( int* )ChatFound == 0 && IsGameFrameActive( ) )
		{
			*( int* )pCurrentFrameFocusedAddr = 0;
		}

		if ( Msg == WM_LBUTTONUP )
		{
			ProcessClickAtCustomFrames( );
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

		auto t_end = std::chrono::high_resolution_clock::now( );
		if ( std::chrono::duration<float, std::milli>( t_end - t_start ).count( ) > 250.0 )
		{
			t_start = t_end;
			if ( FPS_LIMIT_ENABLED )
			{
				UpdateFPS( );
			}
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
		if ( ( Msg == WM_KEYDOWN /*|| Msg == WM_KEYUP */ ) && (
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

		if ( *( int* )ChatFound == 0 && IsGameFrameActive( ) )
		{
			//	char keystateprint[ 200 ];
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

						for ( int VK : WhiteListForTeleport )
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
				bool itempressed = false;

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

								if ( Msg == WM_KEYDOWN && !( lParam & 0x40000000 ) )
								{
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
								}
							}
						}
					}
				}


				/*sprintf_s( keystateprint, 200, "[0]VK:%X->%X", wParam, lParam );
				PrintText( keystateprint );*/
				if ( !NeedSkipThisKey )
					for ( KeyActionStruct keyAction : KeyActionList )
					{
						if ( keyAction.VK == ( int )wParam )
						{
							if ( Msg == WM_SYSKEYDOWN )
								Msg = WM_KEYDOWN;

							/*if ( Msg == WM_SYSKEYUP )
								Msg = WM_KEYUP;*/


								/*	sprintf_s( keystateprint, 200, "[1]VK:%X->%X , IsAlt:%X->%X , IsCtrl:%X->%X , IsShift:%X->%X ", wParam, lParam,
										keyAction.IsAlt, IsKeyPressed( VK_MENU ), keyAction.IsCtrl, IsKeyPressed( VK_CONTROL ), keyAction.IsShift, IsKeyPressed( VK_SHIFT ) );

									PrintText( keystateprint );*/

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

									if ( selectedunitcout == 1 && ( !keyAction.IsSkill  || ClickHelper ))
									{
										if ( wParam == LatestPressedKey )
										{
											if ( IsCursorSelectTarget( ) )
											{
												ClickHelperWork = TRUE;
												PressMouseAtSelectedHero( itempressed );
											}
										}
									}

									LatestPressedKey = wParam;

									if ( GetUnitOwnerSlot( selectedunit ) != 15 )
									{
										if ( !ClickHelperWork )
										{
											if ( IsNULLButtonFound( GetSkillPanelButton( 11 ) ) )
											{
												if ( keyAction.altbtnID >= 0 )
												{
													if ( !( lParam & 0x40000000 ) )
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
												if ( !( lParam & 0x40000000 ) )
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

				if ( !NeedSkipThisKey )
				{
					/*	if ( _Msg == WM_XBUTTONDOWN
							|| _Msg == WM_MBUTTONDOWN )
						*/
					Msg = _Msg;
					wParam = _wParam;
					//	}
				}

				if ( !NeedSkipThisKey )
				{
					for ( int keyCode : BlockedKeyCodes )
					{
						if ( keyCode == ( int )wParam )
						{
							return DefWindowProc( hWnd, Msg, wParam, lParam );
						}
					}



					for ( int keyCode : RegisteredKeyCodes )
					{
						if ( keyCode == ( int )wParam )
						{

							if ( Msg == WM_KEYDOWN && !( lParam & 0x40000000 ) )
							{

								SendKeyEvent.push_back( 0x85 );
								SendKeyEvent.push_back( ( unsigned int )GetLocalPlayerId( ) );
								SendKeyEvent.push_back( Msg );
								SendKeyEvent.push_back( wParam );
								SendPacket( ( BYTE* )&SendKeyEvent[ 0 ], SendKeyEvent.size( ) * 4 );
								SendKeyEvent.clear( );
								//*KeyboardAddrForKey = ( int ) wParam;
								//*KeyboardAddrForKeyEvent = ( int ) Msg;
							//	TriggerExecute( KeyboardTriggerHandle );
							}
							else if ( Msg == WM_KEYUP )
							{

								SendKeyEvent.push_back( 0x85 );
								SendKeyEvent.push_back( ( unsigned int )GetLocalPlayerId( ) );
								SendKeyEvent.push_back( Msg );
								SendKeyEvent.push_back( wParam );
								SendPacket( ( BYTE* )&SendKeyEvent[ 0 ], SendKeyEvent.size( ) * 4 );
								SendKeyEvent.clear( );
								//*KeyboardAddrForKey = ( int ) wParam;
								//*KeyboardAddrForKeyEvent = ( int ) Msg;
								//TriggerExecute( KeyboardTriggerHandle );
							}
							return DefWindowProc( hWnd, Msg, wParam, lParam );
						}

					}

				}


				if ( ( ( wParam >= 0x41 && wParam <= 0x5A ) ||
					( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 ) )
					|| NeedSkipThisKey )
				{
					/*if ( ( wParam >= 0x41 && wParam <= 0x5A ) || ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 ) )
					{*/

					/*	char processdoubleclic[ 30 ];
					sprintf_s( processdoubleclic, "%s", "1" );
					PrintText( processdoubleclic );*/

#ifdef DOTA_HELPER_LOG
					AddNewLineToDotaHelperLog( __func__ );
#endif



					int selectedunitcout = GetSelectedUnitCountBigger( GetLocalPlayerId( ) );
					int unitowner = selectedunitcout > 0 ? GetUnitOwnerSlot( GetSelectedUnit( GetLocalPlayerId( ) ) ) : 0;

					if ( EnableSelectHelper )
					{
						if ( selectedunitcout == 0 ||
							( unitowner != GetLocalPlayerId( ) && !GetPlayerAlliance( Player( unitowner ), Player( GetLocalPlayerId( ) ), 6 ) )
							)
						{

							/*sprintf_s( processdoubleclic, "%s", "2" );
							PrintText( processdoubleclic );*/


							WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
							WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );

							DelayedPress tmpDelayPress;
							tmpDelayPress.NeedPresslParam = lParam;
							tmpDelayPress.NeedPresswParam = wParam;
							tmpDelayPress.NeedPressMsg = 0;
							tmpDelayPress.TimeOut = 120;
							DelayedPressList.push_back( tmpDelayPress );

							
							if ( NeedSkipThisKey )
								return DefWindowProc( hWnd, Msg, wParam, lParam );

							return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
						}
					}

					if ( selectedunitcout == 1 )
					{
						if ( ClickHelper )
						{
							/*sprintf_s( processdoubleclic, "%s", "22" );
							PrintText( processdoubleclic );*/
							/*	if ( !NeedSkipThisKey && IsKeyPressed( VK_LCONTROL ) )
								{
									//JustClickMouse( );
								}
								else
								{*/

							if ( LastPressedKeysTime[ wParam ] + 400 > GetTickCount( ) )
							{
								/*sprintf_s( processdoubleclic, "%s", "33" );
								PrintText( processdoubleclic );*/

								itempressed = wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8;

								if ( IsCursorSelectTarget( ) )
								{
									/*sprintf_s( processdoubleclic, "%s->%i", "44", PressMouseAtSelectedHero( ) );
									PrintText( processdoubleclic );*/
									PressMouseAtSelectedHero( itempressed );
									
									if ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 )
									{
										LastPressedKeysTime[ wParam ] = 0;
										return DefWindowProc( hWnd, Msg, wParam, lParam );
									}
								}
							

							}
							else
								LastPressedKeysTime[ wParam ] = GetTickCount( );
						}
						//}
					}

#ifdef DOTA_HELPER_LOG
					AddNewLineToDotaHelperLog( __func__ + to_string( 2 ) );
#endif

				}
			}

			if ( NeedSkipThisKey )
				return DefWindowProc( hWnd, Msg, wParam, lParam );

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
						( unitowner != GetLocalPlayerId( ) && !GetPlayerAlliance( Player( unitowner ), Player( GetLocalPlayerId( ) ), 6 ) )
						)
					{

						WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
						WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );
#ifdef DOTA_HELPER_LOG
						AddNewLineToDotaHelperLog( __func__ + to_string( 4 ) );
#endif
					}
					}
				}
			}

		}
	else
	{
		if ( LOCK_MOUSE_IN_WINDOW )
			ClipCursor( 0 );

		if ( BlockKeyAndMouseEmulation )
		{
			if ( Msg == WM_RBUTTONDOWN || Msg == WM_KEYDOWN || Msg == WM_KEYUP )
			{
				return DefWindowProc( hWnd, Msg, wParam, lParam );
			}
		}
	}

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
	AddNewLineToDotaHelperLog( __func__ );
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
	AddNewLineToDotaHelperLog( __func__ + to_string( 2 ) );
#endif
}


void IssueFixerDisable( )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif

	memset( LastPressedKeysTime, 0, sizeof( LastPressedKeysTime ) );

	MH_DisableHook( IssueWithoutTargetOrderorg );
	MH_DisableHook( IssueTargetOrPointOrder2org );
	MH_DisableHook( sub_6F339D50org );
	MH_DisableHook( IssueTargetOrPointOrderorg );
	MH_DisableHook( sub_6F339E60org );
	MH_DisableHook( sub_6F339F00org );
	MH_DisableHook( sub_6F339F80org );
	MH_DisableHook( sub_6F33A010org );

	if ( !RegisteredKeyCodes.empty( ) )
		RegisteredKeyCodes.clear( );
	if ( !BlockedKeyCodes.empty( ) )
		BlockedKeyCodes.clear( );
	if ( !KeyActionList.empty( ) )
		KeyActionList.clear( );

	SkipAllMessages = FALSE;
}