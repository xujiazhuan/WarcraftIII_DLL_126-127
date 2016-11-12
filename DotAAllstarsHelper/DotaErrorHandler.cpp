#include "Main.h"
#include <iomanip>
#include <eh.h>

#define PSAPI_VERSION 1


#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

class InfoFromSE
{
public:
	typedef unsigned int exception_code_t;

	static const char* opDescription( const ULONG opcode )
	{
		switch ( opcode )
		{
			case 0: return "read";
			case 1: return "write";
			case 8: return "user-mode data execution prevention (DEP) violation";
			default: return "unknown";
		}
	}

	static const char* seDescription( const exception_code_t& code )
	{
		switch ( code )
		{
			case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
			case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
			case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
			case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
			case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
			case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
			case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
			case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
			case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
			case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
			case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
			case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
			case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
			case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
			case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
			case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
			case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
			case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
			case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
			case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
			default: return "UNKNOWN EXCEPTION";
		}
	}

	static std::string information( struct _EXCEPTION_POINTERS* ep, bool has_exception_code = false, exception_code_t code = 0 )
	{
		HMODULE hm;
		::GetModuleHandleEx( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast< LPCTSTR >( ep->ExceptionRecord->ExceptionAddress ), &hm );
		MODULEINFO mi;
		::GetModuleInformation( ::GetCurrentProcess( ), hm, &mi, sizeof( mi ) );
		char fn[ MAX_PATH ];
		::GetModuleFileNameExA( ::GetCurrentProcess( ), hm, fn, MAX_PATH );

		std::ostringstream oss;
		oss << "SE " << ( has_exception_code ? seDescription( code ) : "" ) << " at address 0x" << std::hex << ep->ExceptionRecord->ExceptionAddress << std::dec
			<< " inside " << fn << " loaded at base address 0x" << std::hex << mi.lpBaseOfDll << "\n";

		if ( has_exception_code && (
			code == EXCEPTION_ACCESS_VIOLATION ||
			code == EXCEPTION_IN_PAGE_ERROR ) )
		{
			oss << "Invalid operation: " << opDescription( ep->ExceptionRecord->ExceptionInformation[ 0 ] ) << " at address 0x" << std::hex << ep->ExceptionRecord->ExceptionInformation[ 1 ] << std::dec << "\n";
		}

		if ( has_exception_code && code == EXCEPTION_IN_PAGE_ERROR )
		{
			oss << "Underlying NTSTATUS code that resulted in the exception " << ep->ExceptionRecord->ExceptionInformation[ 2 ] << "\n";
		}

		return oss.str( );
	}


};

std::string ReadRegistryKeyString( const char *registryKey, const char *registryValue )
{
	// Open the key
	HKEY hKey;
	if ( RegOpenKeyExA( HKEY_LOCAL_MACHINE, registryKey, 0, KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS )
		return 0;

	char str[ 256 ] = { };
	DWORD dwLen = 255;
	LONG ret = RegQueryValueExA( hKey, registryValue, NULL, NULL, ( LPBYTE ) str, &dwLen );
	RegCloseKey( hKey );

	if ( ret == ERROR_SUCCESS )
		return str;
	else
		return std::string( );
}

std::string GetOSDisplayString( )
{
	std::string productName = ReadRegistryKeyString( "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "ProductName" );
	std::string servicePack = ReadRegistryKeyString( "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "CSDVersion" );
	std::string bitness = ReadRegistryKeyString( "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\", "BuildLabEx" );

	if ( productName.length( ) == 0 )
		productName = "Win98/2000?";

	if ( bitness.find( "amd64" ) != std::string::npos )
		bitness = "64-bit";
	else
		bitness = "32-bit";

	return productName + " " + bitness + " " + servicePack;
}

vector<string> DotaChatLog;
vector<string> DotaHelperLog;
vector<string> JassNativesFuncLog;
vector<string> Blizzard1Log;
vector<string> Blizzard2Log;
vector<string> Blizzard3Log;
vector<string> Blizzard4Log;
vector<string> Blizzard4Log_2;
vector<string> Blizzard5Log;
vector<string> Blizzard6Log;
vector<string> JassLogList;
vector<int> CNetEvents;



void AddNewLineToDotaChatLog( string s )
{
	if ( bDllLogEnable )
	{
		if ( DotaChatLog.size( ) > 11 )
		{
			DotaChatLog.erase( DotaChatLog.begin( ) );
		}
		DotaChatLog.push_back( s );
	}
}

void AddNewLineToDotaHelperLog( string s )
{
	if ( bDllLogEnable )
	{
		if ( DotaHelperLog.size( ) > 20 )
		{
			DotaHelperLog.erase( DotaHelperLog.begin( ) );
		}
		DotaHelperLog.push_back( s );
	}
}

void AddNewLineToJassNativesLog( string s )
{
	if ( JassNativesFuncLog.size( ) > 40 )
	{
		JassNativesFuncLog.erase( JassNativesFuncLog.begin( ) );
	}
	JassNativesFuncLog.push_back( s );
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

void AddNewLineToJassLog( string s )
{
	if ( JassLogList.size( ) > 35 )
	{
		JassLogList.erase( JassLogList.begin( ) );
	}
	JassLogList.push_back( s );
}

__declspec( dllexport )  int __stdcall JassLog( const char * s )
{
	AddNewLineToJassLog( s );
	return 0;
}


BOOL bDllLogEnable = TRUE;
__declspec( dllexport )  int __stdcall DllLogEnable( BOOL enable )
{
	bDllLogEnable = enable;
	return 0;
}

//JassNativesFuncLog

void AddNewCNetEventLog( int EventID, void * data, int addr2, int EventByte2 )
{
	if ( CNetEvents.size( ) > 50 )
	{
		CNetEvents.erase( CNetEvents.begin( ) );
	}
	CNetEvents.push_back( EventID );

	if ( EventID == 35 && EventByte2 >= 0xB )
	{
		if ( MessageBox( Warcraft3Window, "Warning desync detected. Start force crash for detect problem?", "DESYNC, CRASH, YES OR NO?", MB_YESNO | MB_ICONWARNING ) == IDYES )
		{
			AddNewLineToDotaHelperLog( "WarningDesyncDetected.StartForceCrashForDetectProblem" );
			//ShowWindow( Warcraft3Window, SW_HIDE );
			//MessageBox( Warcraft3Window, "Warning desync detected. Start force crash for detect problem", "Desync detected!", 0 );
			throw logic_error( "Warning desync detected. Start force crash for detect problem" );
		}
	}
}

LookupNative LookupNative_org = NULL;
LookupNative LookupNative_ptr;

int __fastcall LookupNative_my( LPSTR funcname )
{
	int retval = LookupNative_ptr( funcname );
	if ( funcname && *funcname != '\0' )
	{
		AddNewLineToJassNativesLog( funcname );
	}
	return retval;
}

LookupJassFunc LookupJassFunc_org = NULL;
LookupJassFunc LookupJassFunc_ptr;

signed int __fastcall LookupJassFunc_my( int a1, int unused, char * funcname )
{
	signed int retval = LookupJassFunc_ptr( a1, unused, funcname );
	if ( funcname &&  *funcname != '\0' )
	{
		AddNewLineToJassNativesLog( funcname );
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
		case 32:
			return "CNet::EVENT_MESSAGE";
		default:
			break;
	}
	return "Unknown CNETEvent";
}

ProcessNetEvents ProcessNetEvents_org = NULL;
ProcessNetEvents ProcessNetEvents_ptr;

void __fastcall ProcessNetEvents_my( void *data, int unused, int Event )
{
	AddNewLineToDotaHelperLog( __func__ );
	int EventID = *( BYTE* ) ( Event + 20 );
	ProcessNetEvents_ptr( data, unused, Event );
	AddNewCNetEventLog( EventID, data, Event, *( BYTE* ) ( Event + 12 ) );
}


std::string GetLastErrorAsString( )
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError( );
	if ( errorMessageID == 0 )
		return std::string( ); //No error message has been recorded

	LPSTR messageBuffer = NULL;
	size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								  NULL, errorMessageID, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), ( LPSTR ) &messageBuffer, 0, NULL );

	if ( size > 2 && messageBuffer != NULL )
	{
		std::string message( messageBuffer, size );

		//Free the buffer.
		LocalFree( messageBuffer );

		return message;
	}
	return "NULL";
}


StormErrorHandler StormErrorHandler_org = NULL;
StormErrorHandler StormErrorHandler_ptr;


string url_encode( const string &value )
{
	ostringstream escaped;
	escaped.fill( '0' );
	escaped << hex;

	for ( string::const_iterator i = value.begin( ), n = value.end( ); i != n; ++i )
	{
		string::value_type c = ( *i );

		// Keep alphanumeric and other accepted characters intact
		/*if ( isalnum( c ) || c == '-' || c == '_' || c == '.' )
		{
			escaped << c;
			continue;
		}*/

		// Any other characters are percent-encoded
		escaped << uppercase;
		escaped << '%' << setw( 2 ) << int( ( unsigned char ) c );
		escaped << nouppercase;
	}

	return escaped.str( );
}

string LastExceptionError;
DWORD ESP_for_DUMP = 0;
LPTOP_LEVEL_EXCEPTION_FILTER OriginFilter = NULL;

LONG __stdcall TopLevelExceptionFilter( _EXCEPTION_POINTERS *ExceptionInfo )
{
	ESP_for_DUMP = ExceptionInfo->ContextRecord->Esp;

	LastExceptionError = InfoFromSE( ).information( ExceptionInfo, true, ExceptionInfo->ExceptionRecord->ExceptionCode );
	FILE * f;
	fopen_s( &f, "lasterror.txt", "w" );
	if ( f )
	{
		fwrite( LastExceptionError.c_str( ), LastExceptionError.length( ), 1, f );
	}



	fclose( f );

	return OriginFilter( ExceptionInfo );
}

void InitTopLevelExceptionFilter( )
{
	SetUnhandledExceptionFilter( TopLevelExceptionFilter );
}

void ResetTopLevelExceptionFilter( )
{
	SetUnhandledExceptionFilter( OriginFilter );
}

#define MAX_PROCESSES 1024 


DWORD FindProcess( __in_z LPCTSTR lpcszFileName )
{
	LPDWORD lpdwProcessIds;
	LPTSTR  lpszBaseName;
	HANDLE  hProcess;
	DWORD   i, cdwProcesses, dwProcessId = 0;

	lpdwProcessIds = ( LPDWORD ) HeapAlloc( GetProcessHeap( ), 0, MAX_PROCESSES * sizeof( DWORD ) );
	if ( lpdwProcessIds != NULL )
	{
		if ( EnumProcesses( lpdwProcessIds, MAX_PROCESSES * sizeof( DWORD ), &cdwProcesses ) )
		{
			lpszBaseName = ( LPTSTR ) HeapAlloc( GetProcessHeap( ), 0, MAX_PATH * sizeof( TCHAR ) );
			if ( lpszBaseName != NULL )
			{
				cdwProcesses /= sizeof( DWORD );
				for ( i = 0; i < cdwProcesses; i++ )
				{
					hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[ i ] );
					if ( hProcess != NULL )
					{
						if ( GetModuleBaseName( hProcess, NULL, lpszBaseName, MAX_PATH ) > 0 )
						{
							if ( !lstrcmpi( lpszBaseName, lpcszFileName ) )
							{
								dwProcessId = lpdwProcessIds[ i ];
								CloseHandle( hProcess );
								break;
							}
						}
						CloseHandle( hProcess );
					}
				}
				HeapFree( GetProcessHeap( ), 0, ( LPVOID ) lpszBaseName );
			}
		}
		HeapFree( GetProcessHeap( ), 0, ( LPVOID ) lpdwProcessIds );
	}
	return dwProcessId;
}

std::string GetPlatformName( )
{
	if ( GetModuleHandle( "iccwc3.icc" ) )
		return "[iCCup]";
	if ( GetModuleHandle( "InputHook.dll" ) && GetModuleHandle( "Overlay.dll" ) )
		return "[Garena Plus]";
	if ( FindProcess( "rgc.exe" ) )
		return "[RGC]";
	if ( GetModuleHandle( "mroc.dll" ) || FindProcess( "myroc.exe" ) )
		return "[RGC]";
	if ( !GetModuleHandle( "w3lh.dll" ) )
		return "[Unknown Or Battle.net]";

	return "[Unknown PVPGN server]";
}
string ConvertMemoryToHex( unsigned char * buffer, int size )
{
	stringstream ss;
	ss << std::hex << std::setfill( '0' );
	for ( int i = 0; i < size; ++i )
	{
		ss << std::setw( 2 ) << static_cast< unsigned >( buffer[ i ] );
	}
	return ss.str( );
}

string ConvertMemoryToHexReverse( unsigned char * buffer, int size )
{
	stringstream ss;
	ss << std::hex << std::setfill( '0' );
	for ( int i = size - 1; i >= 0; i-- )
	{
		ss << std::setw( 2 ) << static_cast< unsigned >( buffer[ i ] );
	}
	return ss.str( );
}

LONG __fastcall  StormErrorHandler_my( int a1, void( *PrintErrorLog )( int, const char *, ... ), int a3, BYTE *a4, LPSYSTEMTIME a5 )
{
	bDllLogEnable = FALSE;
	LONG result = NULL;
	PrintErrorLog( a3, "%s", "[Dota Allstars Error Handler v0.1a]" );
	result = StormErrorHandler_ptr( a1, PrintErrorLog, a3, a4, a5 );
	PrintErrorLog( a3, "%s", "[Dota Allstars DLL log]" );
	string LogTempStr;
	int FuncCount = 0;

	char gamever[ 20 ];
	char lasterror[ 20 ];
	char dllcrc32[ 20 ];
	sprintf_s( gamever, 20, "%X", GameVersion );
	sprintf_s( lasterror, 20, "%#010x", ( UINT ) GetLastError( ) );
	sprintf_s( dllcrc32, 20, "%#010x", ( UINT ) GetDllCrc32( ) );
	ostringstream BugReport;
	BugReport << "[DotaHelperLog]" << std::endl;
	char EspDump[ 16 ];
	DWORD EspDumpBytes;
	ReadProcessMemory( GetCurrentProcess( ), ( void * ) ( ESP_for_DUMP - 16 ), EspDump, 16, &EspDumpBytes );


	BugReport << "GameVer: 1." << gamever << std::endl;
	BugReport << ( const char * ) ( *InGame ? "InGame" : "NotInGame" ) << std::endl;
	BugReport << ( const char * ) ( *( BOOL* ) IsWindowActive ? "WindowOpened" : "WindowMinimized" ) << std::endl;


	if ( *InGame )
	{
		const char * currentplayername = GetPlayerName( GetLocalPlayerId( ), 1 );
		if ( currentplayername && *currentplayername != '\0' )
			BugReport << "[PlayerName]:" << currentplayername << std::endl;
		else
			BugReport << "[PlayerName]:" << "Error!" << std::endl;
		BugReport << "[PlayerList]:" << std::endl;
		for ( int i = 0; i < 15; i++ )
		{
			currentplayername = GetPlayerName( i, 1 );
			if ( currentplayername && *currentplayername != '\0' )
			{
				if ( strstr( currentplayername, "Player " ) )
					continue;
				if ( strstr( currentplayername, "Neutral " ) )
					continue;
				if ( strstr( currentplayername, "The " ) )
					continue;

				BugReport << "[_" << i << "_]:" << currentplayername << std::endl;
			}
		}
		BugReport << "[EndPlayerList]:" << std::endl;
	}


	BugReport << "[SystemTime]: " << a5->wYear << "." << a5->wMonth << "." << a5->wDay << " " << a5->wHour << ":" << a5->wMinute << ":" << a5->wSecond << std::endl;
	BugReport << "[OS]: " << GetOSDisplayString( ) << std::endl;
	BugReport << "[CMD]: " << GetCommandLineA( ) << std::endl;

	BugReport << "[LastError]: \"" << GetLastErrorAsString( ) << "\", CODE: " << lasterror << std::endl;
	BugReport << "[Platform]: " << GetPlatformName( ) << std::endl;
	BugReport << "[Exception Info]: " << std::endl;
	BugReport << LastExceptionError << std::endl;
	BugReport << "[DLL_CRC32]: " << dllcrc32 << std::endl;
	BugReport << "[ESP]: " << ConvertMemoryToHexReverse( ( unsigned char* ) &ESP_for_DUMP, 4 ) << ". [DUMP]:" << ConvertMemoryToHexReverse( ( unsigned char* ) &EspDump, 16 ) << std::endl;

	BugReport << "[DLL_LOG]: " << std::endl;

	string LastError1, LastError2, LastError3, LastError4;

	for ( string s : DotaHelperLog )
	{
		if ( s == LogTempStr )
		{
			FuncCount++;
		}
		else
		{
			if ( FuncCount > 0 )
			{
				BugReport << s << ( "(" + std::to_string( FuncCount + 1 ) + "x)" ) << " ";
				PrintErrorLog( a3, "%s (%ix)", s.c_str( ), ( FuncCount + 1 ) );
			}
			else
			{
				BugReport << s << " ";
				PrintErrorLog( a3, "%s", s.c_str( ) );
			}
			LogTempStr = s;
			LastError1 = s;
			FuncCount = 0;
		}
	}
	FuncCount = 0;

	PrintErrorLog( a3, "%s", "[Dota Allstars Jass Native log]" );
	BugReport << std::endl << "[JassNativesLog]" << std::endl;

	for ( string s : JassNativesFuncLog )
	{
		if ( s == LogTempStr )
		{
			FuncCount++;
		}
		else
		{
			if ( FuncCount > 0 )
			{
				BugReport << s << ( "(" + std::to_string( FuncCount + 1 ) + "x)" ) << " ";
				PrintErrorLog( a3, "%s (%ix)", s.c_str( ), ( FuncCount + 1 ) );
			}
			else
			{
				BugReport << s << " ";
				PrintErrorLog( a3, "%s", s.c_str( ) );
			}
			LogTempStr = s;
			LastError2 = s;
			FuncCount = 0;
		}
	}


	FuncCount = 0;

	PrintErrorLog( a3, "%s", "[Dota Jass Log]" );
	BugReport << std::endl << "[DotaJassLog]" << std::endl;

	for ( string s : JassLogList )
	{
		if ( s == LogTempStr )
		{
			FuncCount++;
		}
		else
		{
			if ( FuncCount > 0 )
			{
				BugReport << s << ( "(" + std::to_string( FuncCount + 1 ) + "x)" ) << " ";
				PrintErrorLog( a3, "%s (%ix)", s.c_str( ), ( FuncCount + 1 ) );
			}
			else
			{
				BugReport << s << " ";
				PrintErrorLog( a3, "%s", s.c_str( ) );
			}
			LogTempStr = s;
			LastError3 = s;
			FuncCount = 0;
		}
	}

	FuncCount = 0;

	//PrintErrorLog( a3, "%s", "[Dota Chat Log]" );
	BugReport << std::endl << "[DotaChatLog]" << std::endl;


	for ( string s : DotaChatLog )
	{
		if ( s == LogTempStr )
		{
			FuncCount++;
		}
		else
		{
			if ( FuncCount > 0 )
			{
				BugReport << s << ( "(" + std::to_string( FuncCount + 1 ) + "x)" ) << std::endl;
				//PrintErrorLog( a3, "%s (%ix)", s.c_str( ), ( FuncCount + 1 ) );
			}
			else
			{
				BugReport << s << std::endl;
				//PrintErrorLog( a3, "%s", url_encode( s).c_str( ) );
			}
			LogTempStr = s;
			LastError4 = s;
			FuncCount = 0;
		}
	}
	BugReport << std::endl << "[DotaChatLogEnd]" << std::endl;

	BugReport << std::endl;
	BugReport << "Crash at:" << "[DLL]:" << LastError1 << ", [JASSFUNC]:" << LastError2 << ", [JASSLOG]:" << LastError3 << ", [CHAT]:" << LastError4;
	BugReport << std::endl << "[END]";

	string strBugReport = url_encode( BugReport.str( ) );
	SendHttpPostRequest( "d1stats.ru", "/fatal.php", ( "msg=" + strBugReport ).c_str( ) );
	//	SendHttpGetRequest( "d1stats.ru", ("/fatal.php?msg=QQGETQQ" + BugReport.str( ) ).c_str( ) );

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

	return result;
}





BlizzardDebug1 BlizzardDebug1_org = NULL;
BlizzardDebug1 BlizzardDebug1_ptr;

int __fastcall BlizzardDebug1_my( const char * str )
{
	int retval = 0;
	__asm mov retval, eax;
	AddNewLineToBlizzard1Log( str );
	return retval;
}

BlizzardDebug2 BlizzardDebug2_org = NULL;
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

BlizzardDebug3 BlizzardDebug3_org = NULL;
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

BlizzardDebug4 BlizzardDebug4_org = NULL;
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

BlizzardDebug5 BlizzardDebug5_org = NULL;
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

BlizzardDebug6 BlizzardDebug6_org = NULL;
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

	InitTopLevelExceptionFilter( );


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
	if ( !DotaChatLog.empty( ) )
		DotaChatLog.clear( );

	if ( !JassLogList.empty( ) )
		JassLogList.clear( );

	if ( !DotaHelperLog.empty( ) )
		DotaHelperLog.clear( );

	if ( !JassNativesFuncLog.empty( ) )
		JassNativesFuncLog.clear( );

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
		MH_DisableHook( BlizzardDebug4_org );
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

