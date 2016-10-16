#include "Main.h"

vector<string> DotaHelperLog;
vector<string> JassNativesLog;
vector<string> JassFuncLog;

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


	if ( !DotaHelperLog.empty( ) )
		DotaHelperLog.clear( );

	if ( !DotaHelperLog.empty( ) )
		JassNativesLog.clear( );

	if ( !DotaHelperLog.empty( ) )
		JassFuncLog.clear( );
}

