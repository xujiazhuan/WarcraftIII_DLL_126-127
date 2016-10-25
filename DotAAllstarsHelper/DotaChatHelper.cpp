#include "Main.h"





int GetChatOffset( )
{
	if ( *( int* ) pW3XGlobalClass > 0 )
	{
		return *( int* ) ( ( *( int* ) pW3XGlobalClass ) + 0x3FC );
	}

	return 0;
}




char * GetChatString( )
{
	int pChatOffset = GetChatOffset( );
	if ( pChatOffset > 0 )
	{
		pChatOffset = *( int* ) ( pChatOffset + 0x1E0 );
		if ( pChatOffset > 0 )
		{
			pChatOffset = *( int* ) ( pChatOffset + 0x1E4 );
			return ( char * ) pChatOffset;
		}
	}
	return 0;
}

LPARAM lpReturnScanKeyUP = ( LPARAM ) ( 0xC0000001 | ( LPARAM ) ( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );
LPARAM lpReturnScanKeyDOWN = ( LPARAM ) ( 0x00000001 | ( LPARAM ) ( MapVirtualKey( VK_RETURN, 0 ) << 16 ) );


__declspec( dllexport )  int __stdcall SendMessageToChat( const char * msg )
{
	BYTE tmpbuf[ 256 ];
	BYTE tmpbuf2[ 256 ];
	ZeroMemory( tmpbuf2, 256 );
	GetKeyboardState( tmpbuf );
	SetKeyboardState( tmpbuf2 );

	char * pChatString = GetChatString( );
	if ( msg && pChatString && Warcraft3Window)
	{
		if ( *( int* ) ChatFound > 0 )
		{
			if ( *pChatString != '\0' )
			{
				pChatString[ 0 ] = '\0';
			}

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

	SetKeyboardState( tmpbuf );

	return 0;
}