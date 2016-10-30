#include "Main.h"




HWND Warcraft3Window = 0;

WarcraftRealWNDProc WarcraftRealWNDProc_org = NULL;
WarcraftRealWNDProc WarcraftRealWNDProc_ptr;


LPARAM lpF1ScanKeyUP = ( LPARAM ) ( 0xC0000001 | ( LPARAM ) ( MapVirtualKey( VK_F1, 0 ) << 16 ) );
LPARAM lpF1ScanKeyDOWN = ( LPARAM ) ( 0x00000001 | ( LPARAM ) ( MapVirtualKey( VK_F1, 0 ) << 16 ) );


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
	Input.ki.wScan = ( WORD ) MapVirtualKey( ( UINT ) VK, 0 );
	Input.ki.wVk = ( WORD ) VK;
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
					SkipAllMessages = TRUE;
					PressKeyboard( ( int ) DelayedPressList[ i ].NeedPresswParam );
					//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, DelayedPressList[ i ].NeedPresswParam, DelayedPressList[ i ].NeedPresslParam );
					//	WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, DelayedPressList[ i ].NeedPresswParam, ( LPARAM ) ( 0xC0000000 | DelayedPressList[ i ].NeedPresslParam ) );
					SkipAllMessages = FALSE;
				}
				else
				{
					WarcraftRealWNDProc_ptr( Warcraft3Window, DelayedPressList[ i ].NeedPressMsg, DelayedPressList[ i ].NeedPresswParam, DelayedPressList[ i ].NeedPresslParam );
				}
				DelayedPressList.erase( DelayedPressList.begin( ) + ( int ) i );
				i--;
			}

		}


	}
	
	return 0;
}


DWORD LastKeyPressedTime = 0;
DWORD LastKeyPressedKey = 0;

BOOL IsCursorSelectTarget( )
{
	int pOffset1 = *( int* ) ( pW3XGlobalClass );
	if ( pOffset1 > 0 && *( int* ) ( pOffset1 + 0x1BC ) == 1 )
	{
		return TRUE;
	}
	return FALSE;
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
	HeroFrameX_old = *( float* ) ( GameFrameAtMouseStructOffset + 0x14 );
	*( float* ) ( GameFrameAtMouseStructOffset + 0x14 ) = HeroFrameX;
	HeroFrameY_old = *( float* ) ( GameFrameAtMouseStructOffset + 0x18 );
	*( float* ) ( GameFrameAtMouseStructOffset + 0x18 ) = HeroFrameY;
}

void SetHeroFrameXY_old( )
{
	if ( *( float* ) ( GameFrameAtMouseStructOffset + 0x14 ) == HeroFrameX )
	{
		*( float* ) ( GameFrameAtMouseStructOffset + 0x14 ) = HeroFrameX_old;
		*( float* ) ( GameFrameAtMouseStructOffset + 0x18 ) = HeroFrameY_old;
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
	POINT * p = ( POINT * ) lpp;
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
void PressMouseAtSelectedHero( )
{

	if ( IsCursorSelectTarget( ) )
	{

		BOOL ButtonDown = FALSE;
		if ( IsKeyPressed( VK_LBUTTON ) )
		{
			ButtonDown = TRUE;
			SendMessage( Warcraft3Window, WM_LBUTTONUP, 0, oldlParam );
		}

		int x = ( int ) ( *GetWindowXoffset * HeroPortX );
		int y = ( int ) ( *GetWindowYoffset * HeroPortY );

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
}


struct mMessage
{
	UINT Msg;
	UINT wParam;
};

vector<mMessage> SkipMessagesList;


WPARAM LastShift = 0;

LPARAM MakeLParamVK( UINT VK, BOOL up, BOOL Extended = FALSE )
{
	if ( up ) return ( LPARAM ) ( 0xC0000001 | ( ( UINT ) Extended << 24 ) | ( LPARAM ) ( MapVirtualKey( VK, 0 ) << 16 ) );
	else return ( LPARAM ) ( 0x00000001 | ( ( UINT ) Extended << 24 ) | ( LPARAM ) ( MapVirtualKey( VK, 0 ) << 16 ) );
}


DWORD LastPressedKeysTime[ 256 ];



BOOL KeyboardHaveTriggerEvent = FALSE;
UINT KeyboardTriggerHandle = 0;
int KeyboardAddrForKey = 0;
int KeyboardAddrForKeyEvent = 0;

__declspec( dllexport ) int __stdcall TriggerRegisterPlayerKeyboardEvent( int AddrForKey, int AddrForKeyEvent, UINT TriggerHandle )
{
	if ( !KeyboardHaveTriggerEvent )
	{
		KeyboardTriggerHandle = TriggerHandle;

		KeyboardHaveTriggerEvent = TRUE;
	}
	return 0;
}





LRESULT __stdcall BeforeWarcraftWNDProc( HWND hWnd, UINT Msg, WPARAM _wParam, LPARAM lParam )
{
	WPARAM wParam = _wParam;
	if ( SkipAllMessages )
	{
		return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );
	}

	if ( !*InGame )
		return WarcraftRealWNDProc_ptr( hWnd, Msg, wParam, lParam );


	if ( *( BOOL* ) IsWindowActive )
	{
		if ( ( Msg == WM_KEYDOWN || Msg == WM_KEYUP ) && ( _wParam == VK_SHIFT || _wParam == VK_LSHIFT || _wParam == VK_RSHIFT ) )
		{
			ShiftPressed = ( unsigned char ) ( Msg == WM_KEYDOWN ? 0x1u : 0x0u );
		}

		if ( Msg == WM_RBUTTONDOWN )
		{
			ShiftPressed = ( unsigned char ) ( IsKeyPressed( VK_SHIFT ) ? 0x1u : 0x0u );
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
			int  scanCode = ( int ) ( ( lParam >> 24 ) & 0x1 );


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
						BOOL NumLock = ( ( ( unsigned short ) GetKeyState( VK_NUMLOCK ) ) & 0xffff ) != 0;
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
				SkipMessagesList.erase( SkipMessagesList.begin( ) + ( int ) i );
				return DefWindowProc( hWnd, Msg, wParam, lParam );
			}
		}

		if ( Msg == WM_MOUSEMOVE && BLOCKMOUSEMOVING )
		{
			return DefWindowProc( hWnd, Msg, wParam, lParam );
		}

		if ( *( int* ) ChatFound == 0 )
		{

			if ( Msg == WM_KEYDOWN )
			{
				if ( ( wParam >= 0x41 && wParam <= 0x5A ) || ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 ) )
				{

					if ( GetSelectedUnitCountBigger( GetLocalPlayerId( ) ) == 0 )
					{
						if ( EnableSelectHelper )
						{
							WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
							WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );

							DelayedPress tmpDelayPress;
							tmpDelayPress.NeedPresslParam = lParam;
							tmpDelayPress.NeedPresswParam = wParam;
							tmpDelayPress.NeedPressMsg = 0;
							tmpDelayPress.TimeOut = 40;
							DelayedPressList.push_back( tmpDelayPress );
						}
					}
					else if ( ClickHelper )
					{

						if ( IsKeyPressed( VK_LCONTROL ) )
						{
							//JustClickMouse( );
						}
						else
						{

							if ( LastPressedKeysTime[ wParam ] + 400 > GetTickCount( ) )
							{
								if ( IsCursorSelectTarget( ) )
								{
									PressMouseAtSelectedHero( );
									if ( wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD8 )
									{
										return DefWindowProc( hWnd, Msg, wParam, lParam );
									}
									LastPressedKeysTime[ wParam ] = 0;
								}
							}

							LastPressedKeysTime[ wParam ] = GetTickCount( );
						}
					}
				}
			}


			if ( Msg == WM_LBUTTONDOWN )
			{
				oldlParam = lParam;
			}


			if ( Msg == WM_RBUTTONDOWN )
			{
				if ( EnableSelectHelper )
				{
					if ( GetSelectedUnitCountBigger( GetLocalPlayerId( ) ) == 0 )
					{
						AddNewLineToDotaHelperLog( "BeforeWarcraftWNDProc2" );

						WarcraftRealWNDProc_ptr( hWnd, WM_KEYDOWN, VK_F1, lpF1ScanKeyDOWN );
						WarcraftRealWNDProc_ptr( hWnd, WM_KEYUP, VK_F1, lpF1ScanKeyUP );
					}
				}
			}
		}
	}
	else
	{
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



__declspec( dllexport ) int __stdcall ToggleBlockKeyAndMouseEmulation( BOOL enable )
{
	BlockKeyAndMouseEmulation = enable;
	return 0;
}


__declspec( dllexport ) int __stdcall ToggleForcedSubSelection( BOOL enable )
{
	EnableSelectHelper = enable;
	return 0;
}

__declspec( dllexport ) int __stdcall ToggleClickHelper( BOOL enable )
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
	int retvalue = IssueWithoutTargetOrderptr( a1, a2, a3, a4 | ShiftPressed );

	return retvalue;
}
int __stdcall IssueTargetOrPointOrder2my( int a1, int a2, float a3, float a4, int a5, int a6 )
{
	int retvalue = IssueTargetOrPointOrder2ptr( a1, a2, a3, a4, a5, a6 | ShiftPressed );

	return retvalue;
}
int __stdcall sub_6F339D50my( int a1, int a2, int a3, unsigned int a4, unsigned int a5 )
{
	int retvalue = sub_6F339D50ptr( a1, a2, a3, a4, a5 | ShiftPressed );

	return retvalue;
}
int __stdcall IssueTargetOrPointOrdermy( int a1, int a2, float a3, float a4, int a5, int a6, int a7 )
{
	int retvalue = IssueTargetOrPointOrderptr( a1, a2, a3, a4, a5, a6, a7 | ShiftPressed );

	return retvalue;
}
int __stdcall sub_6F339E60my( int a1, int a2, float a3, float a4, int a5, int a6, int a7, int a8 )
{
	int retvalue = sub_6F339E60ptr( a1, a2, a3, a4, a5, a6, a7, a8 | ShiftPressed );

	return retvalue;
}
int __stdcall sub_6F339F00my( int a1, int a2, int a3, unsigned int a4, unsigned int a5 )
{
	int retvalue = sub_6F339F00ptr( a1, a2, a3, a4, a5 | ShiftPressed );

	return retvalue;
}
int __stdcall sub_6F339F80my( int a1, int a2, float a3, float a4, int a5, int a6, int a7 )
{
	int retvalue = sub_6F339F80ptr( a1, a2, a3, a4, a5, a6, a7 | ShiftPressed );

	return retvalue;
}
int __stdcall sub_6F33A010my( int a1, int a2, float a3, float a4, int a5, int a6, int a7, int a8 )
{
	int retvalue = sub_6F33A010ptr( a1, a2, a3, a4, a5, a6, a7, a8 | ShiftPressed );

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
	AddNewLineToDotaHelperLog( "IssueFixerInit" );
	IssueWithoutTargetOrderorg = ( IssueWithoutTargetOrder ) ( GameDll + IssueWithoutTargetOrderOffset );
	MH_CreateHook( IssueWithoutTargetOrderorg, &IssueWithoutTargetOrdermy, reinterpret_cast< void** >( &IssueWithoutTargetOrderptr ) );

	IssueTargetOrPointOrder2org = ( IssueTargetOrPointOrder2 ) ( GameDll + IssueTargetOrPointOrder2Offset );
	MH_CreateHook( IssueTargetOrPointOrder2org, &IssueTargetOrPointOrder2my, reinterpret_cast< void** >( &IssueTargetOrPointOrder2ptr ) );

	sub_6F339D50org = ( sub_6F339D50 ) ( GameDll + sub_6F339D50Offset );
	MH_CreateHook( sub_6F339D50org, &sub_6F339D50my, reinterpret_cast< void** >( &sub_6F339D50ptr ) );

	IssueTargetOrPointOrderorg = ( IssueTargetOrPointOrder ) ( GameDll + IssueTargetOrPointOrderOffset );
	MH_CreateHook( IssueTargetOrPointOrderorg, &IssueTargetOrPointOrdermy, reinterpret_cast< void** >( &IssueTargetOrPointOrderptr ) );

	sub_6F339E60org = ( sub_6F339E60 ) ( GameDll + sub_6F339E60Offset );
	MH_CreateHook( sub_6F339E60org, &sub_6F339E60my, reinterpret_cast< void** >( &sub_6F339E60ptr ) );

	sub_6F339F00org = ( sub_6F339F00 ) ( GameDll + sub_6F339F00Offset );
	MH_CreateHook( sub_6F339F00org, &sub_6F339F00my, reinterpret_cast< void** >( &sub_6F339F00ptr ) );

	sub_6F339F80org = ( sub_6F339F80 ) ( GameDll + sub_6F339F80Offset );
	MH_CreateHook( sub_6F339F80org, &sub_6F339F80my, reinterpret_cast< void** >( &sub_6F339F80ptr ) );

	sub_6F33A010org = ( sub_6F33A010 ) ( GameDll + sub_6F33A010Offset );
	MH_CreateHook( sub_6F33A010org, &sub_6F33A010my, reinterpret_cast< void** >( &sub_6F33A010ptr ) );



	MH_EnableHook( IssueWithoutTargetOrderorg );
	MH_EnableHook( IssueTargetOrPointOrder2org );
	MH_EnableHook( sub_6F339D50org );
	MH_EnableHook( IssueTargetOrPointOrderorg );
	MH_EnableHook( sub_6F339E60org );
	MH_EnableHook( sub_6F339F00org );
	MH_EnableHook( sub_6F339F80org );
	MH_EnableHook( sub_6F33A010org );

	AddNewLineToDotaHelperLog( "IssueFixerInitEnd" );
}


void IssueFixerDisable( )
{
	AddNewLineToDotaHelperLog( "IssueFixerDisable" );
	if ( GetModuleHandle( "Game.dll" ) != 0 )
	{
		MH_DisableHook( IssueWithoutTargetOrderorg );
		MH_DisableHook( IssueTargetOrPointOrder2org );
		MH_DisableHook( sub_6F339D50org );
		MH_DisableHook( IssueTargetOrPointOrderorg );
		MH_DisableHook( sub_6F339E60org );
		MH_DisableHook( sub_6F339F00org );
		MH_DisableHook( sub_6F339F80org );
		MH_DisableHook( sub_6F33A010org );
	}
}