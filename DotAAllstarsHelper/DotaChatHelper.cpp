#include "Main.h"



#define MAX_CHAT_MSG_LEN 128

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



LPARAM lpRShiftScanKeyUP = ( LPARAM )( 0xC0000001 | ( LPARAM )( MapVirtualKey( VK_RSHIFT, 0 ) << 16 ) );
LPARAM lpRShiftScanKeyDOWN = ( LPARAM )( 0x00000001 | ( LPARAM )( MapVirtualKey( VK_RSHIFT, 0 ) << 16 ) );

pGameChatSetState GameChatSetState;


int __stdcall SendMessageToChat( const char * msg, BOOL toAll )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	if ( !GetChatOffset( ) )
	{
		return FALSE;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	if ( !GetChatString( ) )
	{
		return FALSE;
	}

	BYTE tmpbuf[ 256 ];
	BYTE tmpbuf2[ 256 ];
	memset( tmpbuf2, 0, 256 );
	GetKeyboardState( tmpbuf );
	SetKeyboardState( tmpbuf2 );


#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	char * pChatString = GetChatString( );
	if ( msg > 0 && pChatString > 0 && Warcraft3Window > 0 )
	{
		if ( *( int* )ChatFound > 0 )
		{
			if ( toAll )
			{
				pChatString[ 0 ] = '\0';
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RSHIFT, lpRShiftScanKeyDOWN );

				GameChatSetState( GetChatOffset( ), 0, 0 );

				GameChatSetState( GetChatOffset( ), 0, 1 );
			}
			sprintf_s( pChatString, MAX_CHAT_MSG_LEN, "%s", msg );

			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );


			if ( toAll )
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RSHIFT, lpRShiftScanKeyUP );
			}
		}
		else if ( *( int* )ChatFound == 0 && !toAll )
		{
			if ( toAll )
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RSHIFT, lpRShiftScanKeyDOWN );
			}
			
			GameChatSetState( GetChatOffset( ), 0, 1 );
			
			sprintf_s( pChatString, MAX_CHAT_MSG_LEN, "%s", msg );

			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYDOWN, VK_RETURN, lpReturnScanKeyDOWN );
			WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RETURN, lpReturnScanKeyUP );

			if ( toAll )
			{
				WarcraftRealWNDProc_ptr( Warcraft3Window, WM_KEYUP, VK_RSHIFT, lpRShiftScanKeyUP );
			}
		}

	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	SetKeyboardState( tmpbuf );

	return 0;
}