#include "Main.h"


int GlobalPlayerOffset = 0;
int IsPlayerEnemyOffset = 0;

pGetPlayerColor GetPlayerColor2;
pPlayer _Player;

int playercache[ 16 ];
int player_real_cache[ 16 ];
BOOL player_observers[ 16 ];
int player_local_id = 0;

int __stdcall Player( int slotid )
{
	if ( slotid < 0 || slotid > 15 )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
		AddNewLineToDotaHelperLog( "ERROR! BAD PLAYER SLOT", __LINE__ );
		MessageBoxA( 0, "Error, bad player slot!", "FATAL ERROR", 0 );
		DumpExceptionInfoToFile( 0 );
#endif
		return _Player( 0 );
	}
	
	return playercache[ slotid ];
}

std::map<std::pair<int, int>, BOOL> PlayerEnemyCache;

BOOL IsPlayerEnemy( int hPlayer1, int hPlayer2 )
{
	auto tmppair = std::make_pair( hPlayer1, hPlayer2 );

	if ( PlayerEnemyCache.find( tmppair ) != PlayerEnemyCache.end( ) )
	{
		return PlayerEnemyCache[ tmppair ];
	}

	return ( PlayerEnemyCache[ tmppair ] = ( ( ( IsPlayerEnemy_org )( GameDll + IsPlayerEnemyOffset ) )( hPlayer1, hPlayer2 ) ) );
}


int GetGlobalPlayerData( )
{
	return  *( int * )( GlobalPlayerOffset + GameDll );
}

int GetPlayerByNumber( int number )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	return player_real_cache[ number ];
}

int _GetPlayerByNumber( int number )
{
	int arg1 = GetGlobalPlayerData( );
	int result = 0;

	if ( number < 0 || number > 15 )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
		AddNewLineToDotaHelperLog( "ERROR! BAD PLAYER SLOT", __LINE__ );
		MessageBoxA( 0, "Error, bad player slot!", "FATAL ERROR", 0 );
		DumpExceptionInfoToFile( 0 );
#endif
		return 0;
	}

	if ( arg1 > NULL )
	{
		result = ( int )arg1 + ( number * 4 ) + 0x58;

		if ( result )
		{
			result = *( int* )result;
		}
		else
		{
			return 0;
		}
	}
	return result;
}

// Получить ID игрока
int GetLocalPlayerId( )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	return player_local_id;
}


int _GetLocalPlayerId( )
{
	int gldata = GetGlobalPlayerData( );
	if ( gldata > 0 )
	{
		short retval = *( short * )( gldata + 0x28 );
		return retval;
	}
	return 0;
}


// Получить имя игрока по ID
p_GetPlayerName GetPlayerName = NULL;




vector<char *> mutedplayers;
//sub_6F2FB480
pOnChatMessage pOnChatMessage_org = NULL;
pOnChatMessage pOnChatMessage_ptr;
#ifdef DOTA_HELPER_LOG
char GlobalChatMessageBuffer[ 1024 ];
#endif

void __fastcall pOnChatMessage_my( int a1, int unused, int PlayerID, char * message, int a4, float a5 )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	char * playername = GetPlayerName( PlayerID, 1 );

	if ( playername && *playername != '\0' )
	{

#ifdef DOTA_HELPER_LOG
		if ( message && *message != '\0' )
		{
			memset( GlobalChatMessageBuffer, 0, 1024 );
			sprintf_s( GlobalChatMessageBuffer, "%s:%s", playername, message );
			AddNewLineToDotaChatLog( GlobalChatMessageBuffer );
}
		else
		{
			AddNewLineToDotaHelperLog( "Bad message", __LINE__ );
		}
#endif

		for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
		{
			if ( _stricmp( playername, mutedplayers[ i ] ) == 0 )
			{
				return;
			}
		}

	}
#ifdef DOTA_HELPER_LOG
	else
	{
		AddNewLineToDotaHelperLog( "Bad player", __LINE__ );
	}
#endif
	pOnChatMessage_ptr( a1, unused, PlayerID, message, a4, a5 );
			}


int __stdcall MutePlayer( const char * str )
{
	if ( !str || *str == 0 )
		return 1;
	for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
	{
		if ( _stricmp( str, mutedplayers[ i ] ) == 0 )
		{
			return 1;
		}
	}

	mutedplayers.push_back( _strdup( str ) );
	return 1;
}

int __stdcall UnMutePlayer( const char * str )
{
	if ( !str || *str == 0 )
		return 1;
	for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
	{
		if ( _stricmp( str, mutedplayers[ i ] ) == 0 )
		{
			free( mutedplayers[ i ] );
			mutedplayers[ i ] = NULL;
			mutedplayers.erase( mutedplayers.begin( ) + ( int )i );
			return 1;
		}
	}
	return 1;
}
pIsPlayerObs IsPlayerObs;

BOOL IsPlayerObserver( int pid )
{
	BOOL retval = FALSE;
	if ( pid >= 0 && pid <= 15 )
	{
		return player_observers[ pid ];
	}

	return retval;
}

BOOL _IsPlayerObserver( int pid )
{
	BOOL retval = FALSE;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	if ( pid >= 0 && pid <= 15 )
	{
		int player = Player( pid );
		retval = IsPlayerObs( player );
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	}

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	return retval;
}

BOOL IsLocalPlayerObserver( )
{
	return IsPlayerObserver( GetLocalPlayerId( ) );
}

