#include "Main.h"





int GetChatOffset( )
{
	int pclass = GetGlobalClassAddr( );
	if ( pclass > 0 )
	{
		return *( int* )( pclass + 0x3FC );
	}

	return 0;
}




char * GetChatString( )
{
	int pChatOffset = GetChatOffset( );
	if ( pChatOffset > 0 )
	{
		pChatOffset = *( int* )( pChatOffset + 0x1E0 );
		if ( pChatOffset > 0 )
		{
			pChatOffset = *( int* )( pChatOffset + 0x1E4 );
			return ( char * )pChatOffset;
		}
	}
	return 0;
}

LPARAM lpReturnScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );
LPARAM lpReturnScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );



LPARAM lpShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_LSHIFT, 0 ) << 16 ) );
LPARAM lpShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_LSHIFT, 0 ) << 16 ) );


int __stdcall SendMessageToChat( const char * msg, BOOL toAll )
{
	BYTE tmpbuf[ 256 ];
	BYTE tmpbuf2[ 256 ];
	ZeroMemory( tmpbuf2, 256 );
	GetKeyboardState( tmpbuf );
	SetKeyboardState( tmpbuf2 );

	char * pChatString = GetChatString( );
	if ( msg && pChatString && Warcraft3Window )
	{
		if ( *( int* )ChatFound > 0 && !toAll )
		{
			sprintf_s( pChatString, 150, "%s", msg );

			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );
		}
		else if ( *( int* )ChatFound > 0 && toAll )
		{
			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );

			if ( ShiftPressed == 0 )
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_LSHIFT, lpShiftScanKeyDOWN );

				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );

				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_LSHIFT, lpShiftScanKeyUP );

				sprintf_s( pChatString, 150, "%s", msg );

				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );


			}
			else
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );

				sprintf_s( pChatString, 150, "%s", msg );

				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );
			}
		}
		else
		{
			if ( toAll && ShiftPressed == 0 )
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_LSHIFT, lpShiftScanKeyDOWN );



				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );


				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_LSHIFT, lpShiftScanKeyUP );

				sprintf_s( pChatString, 150, "%s", msg );

				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );



			}
			else if ( toAll && ShiftPressed == 1 )
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );

				sprintf_s( pChatString, 150, "%s", msg );

				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );
			}
			else
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );

				sprintf_s( pChatString, 150, "%s", msg );

				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );
			}
		}

	}

	SetKeyboardState( tmpbuf );

	return 0;
}