#include "Main.h"

vector<string> DotaHelperLog;
vector<string> JassNativesLog;
vector<string> JassFuncLog;
vector<string> Blizzard1Log;
vector<string> Blizzard2Log;
vector<string> Blizzard3Log;
vector<string> Blizzard4Log;
vector<string> Blizzard4Log_2;
vector<string> Blizzard5Log;
vector<string> Blizzard6Log;
vector<int> CNetEvents;

void AddNewLineToDotaHelperLog( string s )
{
	if ( DotaHelperLog.size( ) > 35 )
	{
		DotaHelperLog.erase( DotaHelperLog.begin( ) );
	}
	DotaHelperLog.push_back( s );
}

void AddNewLineToJassNativesLog( string s )
{
	if ( JassNativesLog.size( ) > 35 )
	{
		JassNativesLog.erase( JassNativesLog.begin( ) );
	}
	JassNativesLog.push_back( s );
}


void AddNewLineToJassFuncLog( string s )
{
	if ( JassFuncLog.size( ) > 35 )
	{
		JassFuncLog.erase( JassFuncLog.begin( ) );
	}
	JassFuncLog.push_back( s );
}

void AddNewLineToBlizzard1Log( string s )
{
	if ( Blizzard1Log.size( ) > 35 )
	{
		Blizzard1Log.erase( Blizzard1Log.begin( ) );
	}
	Blizzard1Log.push_back( s );
}


void AddNewLineToBlizzard2Log( string s )
{
	if ( Blizzard2Log.size( ) > 35 )
	{
		Blizzard2Log.erase( Blizzard2Log.begin( ) );
	}
	Blizzard2Log.push_back( s );
}


void AddNewLineToBlizzard3Log( string s )
{
	if ( Blizzard3Log.size( ) > 35 )
	{
		Blizzard3Log.erase( Blizzard3Log.begin( ) );
	}
	Blizzard3Log.push_back( s );
}


void AddNewLineToBlizzard4Log( string s )
{
	if ( Blizzard4Log.size( ) > 35 )
	{
		Blizzard4Log.erase( Blizzard4Log.begin( ) );
	}
	Blizzard4Log.push_back( s );
}

void AddNewLineToBlizzard4Log_2( string s )
{
	if ( Blizzard4Log_2.size( ) > 35 )
	{
		Blizzard4Log_2.erase( Blizzard4Log_2.begin( ) );
	}
	Blizzard4Log_2.push_back( s );
}



void AddNewLineToBlizzard5Log( string s )
{
	if ( Blizzard5Log.size( ) > 35 )
	{
		Blizzard5Log.erase( Blizzard5Log.begin( ) );
	}
	Blizzard5Log.push_back( s );
}


void AddNewLineToBlizzard6Log( string s )
{
	if ( Blizzard6Log.size( ) > 35 )
	{
		Blizzard6Log.erase( Blizzard6Log.begin( ) );
	}
	Blizzard6Log.push_back( s );
}

void AddNewCNetEventLog( int EventID, void * data, int addr2, int EventByte2 )
{
	if ( CNetEvents.size( ) > 50 )
	{
		CNetEvents.erase( CNetEvents.begin( ) );
	}
	CNetEvents.push_back( EventID );

	if ( EventID == 35 && EventByte2 >= 0xB )
	{
		AddNewLineToDotaHelperLog( "Warning desync detected. Start force crash for detect problem" );
		//ShowWindow( Warcraft3Window, SW_HIDE );
		//MessageBox( Warcraft3Window, "Warning desync detected. Start force crash for detect problem", "Desync detected!", 0 );
		throw logic_error( "Warning desync detected. Start force crash for detect problem" );
	}
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

signed int __fastcall LookupJassFunc_my( int a1, int unused, char * funcname )
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
	AddNewCNetEventLog( EventID, data, Event , *( BYTE* ) ( Event + 12) );
}





StormErrorHandler StormErrorHandler_org;
StormErrorHandler StormErrorHandler_ptr;

LONG __fastcall  StormErrorHandler_my( int a1, void( *PrintErrorLog )( int, const char *, ... ), int a3, BYTE *a4, DWORD a5 )
{
	LONG result = NULL;
	PrintErrorLog( a3, "%s", "[Dota Allstars Error Handler v0.1a]" );
	result = StormErrorHandler_ptr( a1, PrintErrorLog, a3, a4, a5 );
	PrintErrorLog( a3, "%s", "[Dota Allstars DLL log]" );

	stringstream BugReport;
	BugReport << "%5BDotaHelperLog%5D";
	for ( string s : DotaHelperLog )
	{
		BugReport << s << "%20";
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Jass Native log]" );
	BugReport << "%0A%5BJassNativesLog%5D";
	for ( string s : JassNativesLog )
	{
		BugReport << s << "%20";
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Jass Func log]" );
	BugReport << "%0A%5BJassFuncLog%5D";
	for ( string s : JassFuncLog )
	{
		BugReport << s << "%20";
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	BugReport << "%5BEND%5D";
	PrintErrorLog( a3, "%s", "[Dota Allstars CNET events]" );
	for ( int EventID : CNetEvents )
	{
		PrintErrorLog( a3, "%s(%i)", GetNetEventStrByID( EventID ), EventID );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard1 log]" );
	for ( string s : Blizzard1Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard2 log]" );
	for ( string s : Blizzard2Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard3 log]" );
	for ( string s : Blizzard3Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard4 log]" );
	for ( string s : Blizzard4Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "-----------------------------------------------" );
	for ( string s : Blizzard4Log_2 )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard5 log]" );
	for ( string s : Blizzard5Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard6 log]" );
	for ( string s : Blizzard6Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Error Handler END]" );

	DownloadBytesGet( "d1stats.ru", "/fatal.php?msg=" + BugReport.str( ) ).clear( );

	return result;
}





BlizzardDebug1 BlizzardDebug1_org;
BlizzardDebug1 BlizzardDebug1_ptr;

int __fastcall BlizzardDebug1_my( const char * str )
{
	int retval = 0;
	__asm mov retval, eax;
	AddNewLineToBlizzard1Log( str );
	return retval;
}

BlizzardDebug2 BlizzardDebug2_org;
BlizzardDebug2 BlizzardDebug2_ptr;

int __cdecl BlizzardDebug2_my( const char * src, int lineid, const char * classname )
{
	int retval = 0;
	__asm mov retval, eax;
	if ( src && classname )
	{
		char tmpdebug[ 256 ];
		sprintf_s( tmpdebug, 256, "Sorce:%s\nLine:%i\nClass:%s", src, lineid, classname );
		AddNewLineToBlizzard2Log( tmpdebug );
	}
	return retval;
}

BlizzardDebug3 BlizzardDebug3_org;
BlizzardDebug3 BlizzardDebug3_ptr;

int __cdecl BlizzardDebug3_my( const char * format, ... )
{
	int retval = 0;
	__asm mov retval, eax;
	char dest[ 256 ];
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( dest, 256, format, argptr );
	va_end( argptr );
	AddNewLineToBlizzard3Log( dest );
	return retval;
}

BlizzardDebug4 BlizzardDebug4_org;
BlizzardDebug4 BlizzardDebug4_ptr;
int __cdecl BlizzardDebug4_my( BOOL type1, const char * format, ... )
{
	int retval = 0;
	__asm mov retval, eax;
	char dest[ 256 ];
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( dest, 256, format, argptr );
	va_end( argptr );
	if ( type1 )
		AddNewLineToBlizzard4Log( dest );
	else
		AddNewLineToBlizzard4Log_2( dest );
	return retval;
}

BlizzardDebug5 BlizzardDebug5_org;
BlizzardDebug5 BlizzardDebug5_ptr;

int __cdecl BlizzardDebug5_my( const char * format, ... )
{
	int retval = 0;
	__asm mov retval, eax;
	char dest[ 256 ];
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( dest, 256, format, argptr );
	va_end( argptr );
	AddNewLineToBlizzard5Log( dest );
	return retval;
}

BlizzardDebug6 BlizzardDebug6_org;
BlizzardDebug6 BlizzardDebug6_ptr;

int __cdecl BlizzardDebug6_my( const char * format, ... )
{
	int retval = 0;
	__asm mov retval, eax;
	char dest[ 256 ];
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( dest, 256, format, argptr );
	va_end( argptr );
	AddNewLineToBlizzard6Log( dest );
	return retval;
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

__declspec( dllexport )  int __stdcall StartExtraErrorHandler( int )
{
	if ( BlizzardDebug1_org )
	{
		MH_CreateHook( BlizzardDebug1_org, &BlizzardDebug1_my, reinterpret_cast< void** >( &BlizzardDebug1_ptr ) );
		MH_EnableHook( BlizzardDebug1_org );
	}

	if ( BlizzardDebug2_org )
	{
		MH_CreateHook( BlizzardDebug2_org, &BlizzardDebug2_my, reinterpret_cast< void** >( &BlizzardDebug2_ptr ) );
		MH_EnableHook( BlizzardDebug2_org );
	}

	if ( BlizzardDebug3_org )
	{
		MH_CreateHook( BlizzardDebug3_org, &BlizzardDebug3_my, reinterpret_cast< void** >( &BlizzardDebug3_ptr ) );
		MH_EnableHook( BlizzardDebug3_org );
	}

	if ( BlizzardDebug4_org )
	{
		MH_CreateHook( BlizzardDebug4_org, &BlizzardDebug4_my, reinterpret_cast< void** >( &BlizzardDebug4_ptr ) );
		MH_EnableHook( BlizzardDebug4_org );
	}

	if ( BlizzardDebug5_org )
	{
		MH_CreateHook( BlizzardDebug5_org, &BlizzardDebug5_my, reinterpret_cast< void** >( &BlizzardDebug5_ptr ) );
		MH_EnableHook( BlizzardDebug5_org );
	}

	if ( BlizzardDebug6_org )
	{
		MH_CreateHook( BlizzardDebug6_org, &BlizzardDebug6_my, reinterpret_cast< void** >( &BlizzardDebug6_ptr ) );
		MH_EnableHook( BlizzardDebug6_org );
	}
	return 0;
}

void DisableErrorHandler( )
{
	if ( GetModuleHandle( "Game.dll" ) != 0 )
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

		if ( BlizzardDebug1_org )
		{
			MH_DisableHook( BlizzardDebug1_org );
		}

		if ( BlizzardDebug2_org )
		{
			MH_DisableHook( BlizzardDebug2_org );
		}

		if ( BlizzardDebug3_org )
		{
			MH_DisableHook( BlizzardDebug3_org );
		}

		if ( BlizzardDebug4_org )
		{
			MH_DisableHook( BlizzardDebug3_org );
		}

		if ( BlizzardDebug5_org )
		{
			MH_DisableHook( BlizzardDebug5_org );
		}

		if ( BlizzardDebug6_org )
		{
			MH_DisableHook( BlizzardDebug6_org );
		}
	}


	if ( !DotaHelperLog.empty( ) )
		DotaHelperLog.clear( );

	if ( !DotaHelperLog.empty( ) )
		JassNativesLog.clear( );

	if ( !DotaHelperLog.empty( ) )
		JassFuncLog.clear( );

	if ( !CNetEvents.empty( ) )
		CNetEvents.clear( );

	if ( !Blizzard1Log.empty( ) )
		Blizzard1Log.clear( );

	if ( !Blizzard2Log.empty( ) )
		Blizzard2Log.clear( );

	if ( !Blizzard3Log.empty( ) )
		Blizzard3Log.clear( );

	if ( !Blizzard4Log.empty( ) )
		Blizzard4Log.clear( );

	if ( !Blizzard4Log_2.empty( ) )
		Blizzard4Log_2.clear( );

	if ( !Blizzard5Log.empty( ) )
		Blizzard5Log.clear( );

	if ( !Blizzard6Log.empty( ) )
		Blizzard6Log.clear( );
}

