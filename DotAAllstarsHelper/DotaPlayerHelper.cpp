#include "Main.h"


int GlobalPlayerOffset = 0;
int IsPlayerEnemyOffset = 0;
int GetPlayerByIDOffset = 0;



int GetGlobalPlayerData( )
{
	return  *( int * ) ( GlobalPlayerOffset + GameDll );
}

int GetPlayerByNumber( int number )
{
	int arg1 = GetGlobalPlayerData( );
	int result = 0;
	if ( arg1 > NULL )
	{
		result = ( int ) arg1 + ( number * 4 ) + 0x58;

		if ( result )
		{
			result = *( int* ) result;
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
	AddNewLineToDotaHelperLog( "GetLocalPlayerId" );
	int gldata = GetGlobalPlayerData( );
	if ( gldata > 0 )
	{
		short retval = *( short * ) ( gldata + 0x28 );
		AddNewLineToDotaHelperLog( "GetLocalPlayerOk" );
		return retval;
	}
	AddNewLineToDotaHelperLog( "GetLocalPlayerIdBad" );
	return 0;
}


// Получить имя игрока по ID
p_GetPlayerName GetPlayerName = NULL;




vector<char *> mutedplayers;
//sub_6F2FB480
pOnChatMessage pOnChatMessage_org = NULL;
pOnChatMessage pOnChatMessage_ptr;
void __fastcall pOnChatMessage_my( int a1, int unused, int PlayerID, char * message, int a4, float a5 )
{
	AddNewLineToDotaHelperLog( "pOnChatMessage_my" );
	char * playername = GetPlayerName( PlayerID, 1 );

	if ( playername && *playername != '\0' )
	{
		AddNewLineToDotaChatLog( string( playername ) + ":" + string( message ) );

		for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
		{
			if ( _stricmp( playername, mutedplayers[ i ] ) == 0 )
			{
				return;
			}
		}
	}

	pOnChatMessage_ptr( a1, unused, PlayerID, message, a4, a5 );
}


__declspec( dllexport ) int __stdcall MutePlayer( const char * str )
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

__declspec( dllexport ) int __stdcall UnMutePlayer( const char * str )
{
	if ( !str || *str == 0 )
		return 1;
	for ( unsigned int i = 0; i < mutedplayers.size( ); i++ )
	{
		if ( _stricmp( str, mutedplayers[ i ] ) == 0 )
		{
			free( mutedplayers[ i ] );
			mutedplayers[ i ] = NULL;
			mutedplayers.erase( mutedplayers.begin( ) + ( int ) i );
			return 1;
		}
	}
	return 1;
}
