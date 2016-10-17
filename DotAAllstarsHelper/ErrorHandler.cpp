#include "Main.h"

vector<string> DotaHelperLog;
vector<string> JassNativesLog;
vector<string> JassFuncLog;
vector<int> CNetEvents;

void AddNewLineToDotaHelperLog( string s )
{
	if ( DotaHelperLog.size( ) > 30 )
	{
		DotaHelperLog.erase( DotaHelperLog.begin( ) );
	}
	DotaHelperLog.push_back( s );
}

void AddNewLineToJassNativesLog( string s )
{
	if ( JassNativesLog.size( ) > 30 )
	{
		JassNativesLog.erase( JassNativesLog.begin( ) );
	}
	JassNativesLog.push_back( s );
}


void AddNewLineToJassFuncLog( string s )
{
	if ( JassFuncLog.size( ) > 30 )
	{
		JassFuncLog.erase( JassFuncLog.begin( ) );
	}
	JassFuncLog.push_back( s );
}


void AddNewCNetEventLog( int EventID )
{
	if ( CNetEvents.size( ) > 30 )
	{
		CNetEvents.erase( CNetEvents.begin( ) );
	}
	CNetEvents.push_back( EventID );
}


LookupNative LookupNative_org = NULL;
LookupNative LookupNative_ptr = NULL;

int __fastcall LookupNative_my( LPSTR funcname )
{
	int retval = LookupNative_ptr( funcname );
	if ( funcname && *funcname )
	{
		AddNewLineToJassNativesLog( funcname );
	}
	return retval;
}



LookupJassFunc LookupJassFunc_org;
LookupJassFunc LookupJassFunc_ptr;

signed int __fastcall LookupJassFunc_my ( int a1, int unused, char * funcname )
{
	signed int retval = LookupJassFunc_ptr( a1, unused, funcname );
	if ( funcname && *funcname )
	{
		AddNewLineToJassFuncLog( funcname );
	}
	return retval;
}


const char * GetNetEventStrByID( int EventID )
{
	switch ( EventID )
	{
		case 1:
			return "CNetEventConnect";
		case 2:
			return "CNetEventDisconnect";
		case 3:
			return "CNetEventGameListStart";
		case 4:
			return "CNetEventGameListStop";
		case 5:
			return "CNetEventGameListError";
		case 6:
			return "CNetEventGameListAdd";
		case 7:
			return "CNetEventGameListUpdate";
		case 8:
			return "CNetEventGameListDelete";
		case 9:
			return "CNetEventTeamGameListStart";
		case 10:
			return "CNetEventTeamGameListStop";
		case 11:
			return "CNetEventTeamGameListAdd";
		case 12:
			return "CNetEventTeamGameListUpdate";
		case 13:
			return "CNetEventTeamGameListDelete";
		case 14:
			return "CNetEventAnonGameFind";
		case 15:
			return "CNetEventAnonGameJoin";
		case 16:
			return "CNetEventGameCreate";
		case 17:
			return "CNetEventGameAd";
		case 18:
			return "CNetEventTeamGameAd";
		case 19:
			return "CNetEventTeamInfo";
		case 20:
			return "CNetEventGameFind";
		case 21:
			return "CNetEventGameJoin";
		case 22:
			return "CNetEventPlayerJoin";
		case 23:
			return "CNetEventPlayerLeave";
		case 24:
			return "CNetEventPlayerReady";
		case 25:
			return "CNetEventGameSetup";
		case 26:
			return "CNetEventGameClose";
		case 27:
			return "CNetEventGameStart";
		case 28:
			return "CNetEventGameReady";
		case 29:
			return "CNetEventPlayerUpdate";
		case 36:
			return "CNetEventGameSuspend";
		case 37:
			return "CNetEventPlayerResume";
		case 38:
			return "CNetEventRouterHandoffSearching";
		case 39:
			return "CNetEventRouterHandoffSyncing";
		case 40:
			return "CNetEventRouterHandoffDone";
		case 41:
			return "CNetEventRouterUnresponsive";
		case 42:
			return "CNetEventRouterResponsive";
		case 43:
			return "CNetEventDistFileStart";
		case 44:
			return "CNetEventDistFileProgress";
		case 45:
			return "CNetEventDistFileComplete";
		case 46:
			return "CNetEventOfficialPlayers";
		case 33:
			return "CNetEventSetTurnsLatency";
		case 47:
			return "CNetEventTrigger";
		case 34:
			return "CNETEVENT_ID_TURNSSYNC";
		case 35:
			return "EVENT_ID_TURNSSYNCMISMATCH";
		case 48:
			return "CNetEventTrustedDesync";
		case 49:
			return "CNetEventTrustedResult";
		case 31: 
			return "CNetGameEvents";
		default:
			break;
	}
	return "Unknown CNETEvent";
}


ProcessNetEvents ProcessNetEvents_org;
ProcessNetEvents ProcessNetEvents_ptr;


void __fastcall ProcessNetEvents_my( void *data, int unused, int Event )
{
	int EventID = *( BYTE* ) ( Event + 20 );
	ProcessNetEvents_ptr( data, unused, Event );
	AddNewCNetEventLog( EventID );
}


StormErrorHandler StormErrorHandler_org;
StormErrorHandler StormErrorHandler_ptr;

LONG __fastcall  StormErrorHandler_my (int a1, void( *PrintErrorLog )( int, const char *, ... ), int a3, BYTE *a4, DWORD a5 )
{
	LONG result = NULL;
	PrintErrorLog( a3, "%s", "[Dota Allstars Error Handler v0.1a]" );
	result = StormErrorHandler_ptr( a1, PrintErrorLog, a3, a4, a5 );
	PrintErrorLog( a3, "%s", "[Dota Allstars DLL log]" );
	for ( string s : DotaHelperLog )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Jass Native log]" );
	for ( string s : JassNativesLog )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Jass Func log]" );
	for ( string s : JassFuncLog )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars CNET events]" );
	for ( int EventID : CNetEvents )
	{
		PrintErrorLog( a3, "%s(%i)", GetNetEventStrByID(EventID), EventID );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Error Handler END]" );
	return result;
}

void EnableErrorHandler( )
{
	if ( StormErrorHandler_org )
	{
		MH_CreateHook( StormErrorHandler_org, &StormErrorHandler_my, reinterpret_cast< void** >( &StormErrorHandler_ptr ) );
		MH_EnableHook( StormErrorHandler_org );
	}

	if ( LookupNative_org )
	{
		MH_CreateHook( LookupNative_org, &LookupNative_my, reinterpret_cast< void** >( &LookupNative_ptr ) );
		MH_EnableHook( LookupNative_org );
	}

	if ( LookupJassFunc_org )
	{
		MH_CreateHook( LookupJassFunc_org, &LookupJassFunc_my, reinterpret_cast< void** >( &LookupJassFunc_ptr ) );
		MH_EnableHook( LookupJassFunc_org );
	}

	if ( ProcessNetEvents_org )
	{
		MH_CreateHook( ProcessNetEvents_org, &ProcessNetEvents_my, reinterpret_cast< void** >( &ProcessNetEvents_ptr ) );
		MH_EnableHook( ProcessNetEvents_org );
	}
	
	
}

void DisableErrorHandler( )
{
	if ( StormErrorHandler_org )
	{
		MH_DisableHook( StormErrorHandler_org );
	}

	if ( LookupNative_org )
	{
		MH_DisableHook( LookupNative_org );
	}

	if ( LookupJassFunc_org )
	{
		MH_DisableHook( LookupJassFunc_org );
	}
	if ( ProcessNetEvents_org )
	{
		MH_DisableHook( ProcessNetEvents_org );
	}

	if ( !DotaHelperLog.empty( ) )
		DotaHelperLog.clear( );

	if ( !DotaHelperLog.empty( ) )
		JassNativesLog.clear( );

	if ( !DotaHelperLog.empty( ) )
		JassFuncLog.clear( );

	if ( !CNetEvents.empty( ) )
		CNetEvents.clear( );
}

