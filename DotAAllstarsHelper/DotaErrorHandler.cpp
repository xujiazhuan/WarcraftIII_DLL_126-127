#include "Main.h"


std::string ReadRegistryKeyString( const char *registryKey, const char *registryValue )
{
	// Open the key
	HKEY hKey;
	if ( RegOpenKeyExA( HKEY_LOCAL_MACHINE, registryKey, 0, KEY_QUERY_VALUE, &hKey ) != ERROR_SUCCESS )
		return std::string( );

	char str[ 256 ] = {};
	DWORD dwLen = 255;
	LONG ret = RegQueryValueExA( hKey, registryValue, NULL, NULL, ( LPBYTE )str, &dwLen );
	RegCloseKey( hKey );

	if ( ret == ERROR_SUCCESS )
		return str;
	else
		return std::string( );
}

std::string LatestOsDisplayString;

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

	LatestOsDisplayString =  productName + " " + bitness + " " + servicePack;
	return LatestOsDisplayString;
}

const char * __stdcall get_os_string( int )
{
	GetOSDisplayString( );

	return LatestOsDisplayString.c_str( );
}


#ifdef DOTA_HELPER_LOG

#pragma optimize("",off)


#include <iomanip>
#include <eh.h>



BOOL IsVEHex = FALSE;


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

	static std::string information( struct _EXCEPTION_POINTERS* ep, BOOL has_exception_code = FALSE, exception_code_t code = 0 )
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


void safevector_erase( safevector<string> & s )
{
	// try just swap, erase not available in concurrent vector?
	for ( unsigned int i = 0; i < ( s.size( ) - 1 ); i++ )
	{
		s[ i ] = s[ i + 1 ];
	}
}


#define MAX_LOG_LEN 100

safevector<string> DotaHelperLog;
safevector<string> JNativesFuncLog;
safevector<string> JassLogList;
safevector<string> DotaChatLog;

safevector<string> Blizzard1Log;
safevector<string> Blizzard2Log;
safevector<string> Blizzard3Log;
safevector<string> Blizzard4Log;
safevector<string> Blizzard4Log_2;
safevector<string> Blizzard5Log;
safevector<string> Blizzard6Log;

vector<int> CNetEvents;

enum class LogType : UINT
{
	DotaChatLog,
	DotaHelperLog,
	JassNativesFuncLog,
	JassLogList
};

HWND ExternErrorHandler = FindWindowA( 0, "Dota Allstars Extern Error Handler" );
DWORD ExternErrorHandlerLastCheck = 0;
void ExternalLog( string s, LogType logtype )
{
	if ( ExternErrorHandler > 0 && IsWindow( ExternErrorHandler ) )
	{
		COPYDATASTRUCT cds;
		cds.dwData = ( ULONG_PTR )logtype;
		cds.cbData = s.length( );
		cds.lpData = ( PVOID )s.c_str( );
		if ( S_OK != SendMessage( ExternErrorHandler, WM_COPYDATA, ( WPARAM )ExternErrorHandler, ( LPARAM )( LPVOID )&cds ) )
		{
			ExternErrorHandler = NULL;
		}
	}
	else if ( ExternErrorHandlerLastCheck + 1000 < GetTickCount( ) )
	{
		ExternErrorHandlerLastCheck = GetTickCount( );
		ExternErrorHandler = FindWindowA( 0, "Dota Allstars Extern Error Handler" );
	}
}

int __stdcall JassLog( const char * s )
{

	AddNewLineToJassLog( s );

	return 0;
}



void __stdcall  AddNewLineToDotaChatLog( const char * s )
{
	if ( bDllLogEnable && s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( DotaChatLog.size( ) > MAX_LOG_LEN )
		{
			DotaChatLog[ MAX_LOG_LEN ] = s;
			safevector_erase( DotaChatLog );
		}
		else
			DotaChatLog.push_back( s );
	}
}

char ErrorHelperBuffer[ 1024 ];

void __stdcall  AddNewLineToDotaHelperLog( const char * s, int line )
{
	if ( bDllLogEnable && s && s[ 0 ] != '\0' )
	{
		memset( ErrorHelperBuffer, 0, 1024 );
		sprintf_s( ErrorHelperBuffer, "Func:%s:%i", s, line );
		ExternalLog( ErrorHelperBuffer, LogType::DotaHelperLog );
		if ( DotaHelperLog.size( ) > MAX_LOG_LEN )
		{
			DotaHelperLog[ MAX_LOG_LEN ] = ErrorHelperBuffer;
			safevector_erase( DotaHelperLog );
		}
		else
			DotaHelperLog.push_back( ErrorHelperBuffer );
	}
}

void __stdcall  AddNewLineToJassNativesLog( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::JassNativesFuncLog );
		if ( JNativesFuncLog.size( ) > MAX_LOG_LEN )
		{
			JNativesFuncLog[ MAX_LOG_LEN ] = s;
			safevector_erase( JNativesFuncLog );
		}
		else
			JNativesFuncLog.push_back( s );
	}
}


void __stdcall  AddNewLineToJassLog( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::JassLogList );
		if ( JassLogList.size( ) > MAX_LOG_LEN )
		{
			JassLogList[ MAX_LOG_LEN ] = s;
			safevector_erase( JassLogList );
		}
		else
			JassLogList.push_back( s );
	}
}



void __stdcall  AddNewLineToBlizzard1Log( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( Blizzard1Log.size( ) > MAX_LOG_LEN )
		{
			Blizzard1Log[ MAX_LOG_LEN ] = s;
			safevector_erase( Blizzard1Log );
		}
		else
			Blizzard1Log.push_back( s );
	}
}


void __stdcall  AddNewLineToBlizzard2Log( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( Blizzard2Log.size( ) > MAX_LOG_LEN )
		{
			Blizzard2Log[ MAX_LOG_LEN ] = s;
			safevector_erase( Blizzard2Log );
		}
		else
			Blizzard2Log.push_back( s );
	}
}


void  __stdcall AddNewLineToBlizzard3Log( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( Blizzard3Log.size( ) > MAX_LOG_LEN )
		{
			Blizzard3Log[ MAX_LOG_LEN ] = s;
			safevector_erase( Blizzard3Log );
		}
		else
			Blizzard3Log.push_back( s );
	}
}


void __stdcall  AddNewLineToBlizzard4Log( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( Blizzard4Log.size( ) > MAX_LOG_LEN )
		{
			Blizzard4Log[ MAX_LOG_LEN ] = s;
			safevector_erase( Blizzard4Log );
		}
		else
			Blizzard4Log.push_back( s );
	}
}

void  __stdcall AddNewLineToBlizzard4Log_2( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( Blizzard4Log_2.size( ) > MAX_LOG_LEN )
		{
			Blizzard4Log_2[ MAX_LOG_LEN ] = s;
			safevector_erase( Blizzard4Log_2 );
		}
		else
			Blizzard4Log_2.push_back( s );
	}
}



void  __stdcall AddNewLineToBlizzard5Log( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( Blizzard5Log.size( ) > MAX_LOG_LEN )
		{
			Blizzard5Log[ MAX_LOG_LEN ] = s;
			safevector_erase( Blizzard5Log );
		}
		else
			Blizzard5Log.push_back( s );
	}
}


void __stdcall AddNewLineToBlizzard6Log( const char * s )
{
	if ( s && s[ 0 ] != '\0' )
	{
		ExternalLog( s, LogType::DotaChatLog );
		if ( Blizzard6Log.size( ) > MAX_LOG_LEN )
		{
			Blizzard6Log[ MAX_LOG_LEN ] = s;
			safevector_erase( Blizzard6Log );
		}
		else
			Blizzard6Log.push_back( s );
	}
}

BOOL bDllLogEnable = TRUE;
int __stdcall DllLogEnable( BOOL enable )
{
	bDllLogEnable = enable;
	return 0;
}

//JNativesFuncLog

void AddNewCNetEventLog( int EventID, void * data, int addr2, int EventByte2 )
{
	if ( CNetEvents.size( ) > 50 )
	{
		CNetEvents.erase( CNetEvents.begin( ) );
	}
	CNetEvents.push_back( EventID );

	if ( EventID == 35 && EventByte2 >= 0xB )
	{
		if ( MessageBoxA( Warcraft3Window, "Warning desync detected. Start force crash for detect problem?", "DESYNC, CRASH, YES OR NO?", MB_YESNO | MB_ICONWARNING ) == IDYES )
		{

			AddNewLineToDotaHelperLog( __func__, __LINE__ );

			//ShowWindow( Warcraft3Window, SW_HIDE );
			//MessageBoxA( Warcraft3Window, "Warning desync detected. Start force crash for detect problem", "Desync detected!", 0 );
			throw logic_error( "Warning desync detected. Start force crash for detect problem" );
		}
	}
}

LookupNative LookupNative_org = NULL;
LookupNative LookupNative_ptr;

int __fastcall LookupNative_my( int global, int unused, const char * funcname )
{
	if ( funcname && *funcname != '\0' )
	{
		AddNewLineToJassNativesLog( funcname );
	}
	else
	{
		AddNewLineToJassNativesLog( "NULL-JASS-NATIVE-NAME-FOUND!" );
	}
	int retval = LookupNative_ptr( global, unused, funcname );
	if ( retval == 0 )
		AddNewLineToJassNativesLog( "NULL-JASS-NATIVE-RETURN-FOUND!" );


	return retval;
}

LookupJassFunc LookupJassFunc_org = NULL;
LookupJassFunc LookupJassFunc_ptr;

signed int __fastcall LookupJassFunc_my( int global, int unused, const char * funcname )
{
	BOOL funcnamefound = FALSE;
	if ( funcname &&  *funcname != '\0' )
	{
		funcnamefound = TRUE;

		AddNewLineToJassNativesLog( funcname );

	}
	else
	{

		AddNewLineToJassNativesLog( "NULL-JASS-FUNCTION-NAME-FOUND!" );

	}

	signed int retval = LookupJassFunc_ptr( global, unused, funcname );


	if ( retval == 0 )
	{
		if ( *InGame )
		{
			if ( funcnamefound )
				MessageBoxA( 0, "LookupJassFunc want to return 0", funcname, MB_OK );
			else
				MessageBoxA( 0, "LookupJassFunc want to return 0", "NULL FUNC", MB_OK );


			AddNewLineToJassNativesLog( "LookupJassFunc want to return 0!" );
			AddNewLineToJassNativesLog( ( funcname && *funcname != '\0' ) ? funcname : "NULL FUNC" );



			DumpExceptionInfoToFile( 0 );
		}
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

	AddNewLineToDotaHelperLog( __func__, __LINE__ );

	/*if ( Event > 0 )
	{*/
	//int EventID = *( BYTE* )( Event + 20 );
	if ( data && Event > 0 )
	{
		int EventID = *( BYTE* )( Event + 20 );
		AddNewCNetEventLog( EventID, data, Event, *( BYTE* )( Event + 12 ) );
	}

	ProcessNetEvents_ptr( data, unused, Event );




	//
//}
}


std::string GetLastErrorAsString( )
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError( );
	if ( errorMessageID == 0 )
		return std::string( ); //No error message has been recorded

	LPSTR messageBuffer = NULL;
	size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), ( LPSTR )&messageBuffer, 0, NULL );

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


string url_encode( const string & value )
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
		escaped << '%' << setw( 2 ) << int( ( unsigned char )c );
		escaped << nouppercase;
	}

	return escaped.str( );
}

string LastExceptionError;
DWORD ESP_for_DUMP = 0;
LPTOP_LEVEL_EXCEPTION_FILTER OriginFilter = NULL;

int __stdcall TraceEsp_Print( int )
{
	int esp_val = 0;
	__asm mov esp_val, esp;
	ostringstream escaped;
	escaped << "Test: Esp=0x" << std::hex << esp_val;
	AddNewLineToDotaChatLog( escaped.str( ).c_str( ) );
	escaped.clear( );
	return esp_val;
}

int dumperrorid = 0;

void DumpExceptionInfoToFile( _EXCEPTION_POINTERS *ExceptionInfo )
{
	string LastError1, LastError2, LastError3, LastError4;

	if ( dumperrorid > 10 )
		return;
	dumperrorid++;
	if ( !IsVEHex )
	{
		AddNewLineToDotaChatLog( "Test: Found SEH Exception!" );
	}

	try
	{
		FILE * f;
		char filenamedump[ 50 ];
		sprintf_s( filenamedump, "lasterror_%i.txt", dumperrorid );
		fopen_s( &f, filenamedump, "w" );
		if ( f )
		{
			fprintf_s( f, "%s[%s]\n", "Dump default error info...", IsVEHex ? "SEH" : "VEH" );
			fprintf_s( f, "Game.dll:%X\n", GameDll );
			fprintf_s( f, "Storm.dll:%X\n", StormDll );
			fprintf_s( f, "Lasterror:%X\n", GetLastError( ) );


			if ( ExceptionInfo )
			{
				fprintf_s( f, "ExceptionInfo:%X\n", ( int )ExceptionInfo );
				fprintf_s( f, "ExceptionInfo->ContextRecord:%X\n", ( int )ExceptionInfo->ContextRecord );
				fprintf_s( f, "ExceptionInfo->ExceptionRecord:%X\n", ( int )ExceptionInfo->ExceptionRecord );
			}

			fprintf_s( f, "\n" );

			fprintf_s( f, "\n" );

			fprintf_s( f, "\n" );

			fprintf_s( f, "\n" );

			fprintf_s( f, "[LOG FAST DUMP]\n" );


			fprintf_s( f, "[DotaHelperLog]\n" );


			for ( string & s : DotaHelperLog )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}

			fprintf_s( f, "[JNativesFuncLog]\n" );


			for ( string & s : JNativesFuncLog )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}

			fprintf_s( f, "[JassLogList]\n" );


			for ( string &s : JassLogList )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}

			fprintf_s( f, "[DotaChatLog]\n" );

			for ( string& s : DotaChatLog )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}

			fprintf_s( f, "[CNetEvents]\n" );

			for ( int & EventID : CNetEvents )
			{
				fprintf_s( f, "%s(%i)\n", GetNetEventStrByID( EventID ), EventID );
			}
			fprintf_s( f, "[Blizzard1Log]\n" );

			for ( string & s : Blizzard1Log )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}
			fprintf_s( f, "[Blizzard2Log]\n" );

			for ( string& s : Blizzard2Log )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}
			fprintf_s( f, "[Blizzard3Log]\n" );

			for ( string &s : Blizzard3Log )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}
			fprintf_s( f, "[Blizzard4Log]\n" );

			for ( string &s : Blizzard4Log )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}
			fprintf_s( f, "[Blizzard4Log_2]\n" );

			for ( string &s : Blizzard4Log_2 )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}
			fprintf_s( f, "[Blizzard5Log]\n" );

			for ( string& s : Blizzard5Log )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}
			fprintf_s( f, "[Blizzard6Log]\n" );

			for ( string& s : Blizzard6Log )
			{
				fprintf_s( f, "%s\n", s.c_str( ) );
			}


			fprintf_s( f, "[END LOG FAST DUMP]\n" );


			/*
			DotaHelperLog;
			JassNativesFuncLog;
			JassLogList;
			DotaChatLog;
			*/
			if ( ExceptionInfo )
			{

				ESP_for_DUMP = ExceptionInfo->ContextRecord->Esp;

				LastExceptionError = InfoFromSE( ).information( ExceptionInfo, TRUE, ExceptionInfo->ExceptionRecord->ExceptionCode );

			}
			fprintf_s( f, "%s", LastExceptionError.c_str( ) );

			fprintf_s( f, "\n%s\n", "End dump." );

			fclose( f );
		}
	}
	catch ( ... )
	{
		MessageBoxA( 0, "SUPER FATAL ERROR!", " UNREAL FATAL ERROR ", 0 );
	}
}

LONG __stdcall TopLevelExceptionFilter( _EXCEPTION_POINTERS *ExceptionInfo )
{
	DumpExceptionInfoToFile( ExceptionInfo );
	if ( MessageBoxA( 0, "Fatal error\nDota helper handler v1.0\nSave fatal info(YES) or exit(NO)?", "Fatal error detected![SEH]", MB_YESNO ) == IDYES )
	{
		int retval = ( int )OriginFilter( ExceptionInfo );
		char tmperr[ 100 ];
		sprintf_s( tmperr, 100, "Error code:%X", retval );
		AddNewLineToDotaChatLog( tmperr );
		return retval;
	}
	else
	{
		if ( StormErrorHandler_org )
		{
			MH_DisableHook( StormErrorHandler_org );
		}

		LONG retval = OriginFilter( ExceptionInfo );

		/*	if ( StormErrorHandler_org )
			{
				MH_DisableHook( StormErrorHandler_org );
			}
	*/
		return retval;
	}
}

int EndGameFound = 10;

LONG __stdcall DotaVectoredToSehHandler( _EXCEPTION_POINTERS *ExceptionInfo )
{
	AddNewLineToDotaChatLog( __func__ );
	if ( IsVEHex )
	{
		return 0;
	}


	if ( !ExceptionInfo )
	{
		AddNewLineToDotaChatLog( "Test: error1" );
		ExceptionInfo = new	_EXCEPTION_POINTERS( );
	}
	if ( !ExceptionInfo->ContextRecord )
	{
		AddNewLineToDotaChatLog( "Test: error2" );
		ExceptionInfo->ContextRecord = new CONTEXT( );
	}

	if ( !ExceptionInfo->ExceptionRecord )
	{
		AddNewLineToDotaChatLog( "Test: error3" );
		ExceptionInfo->ExceptionRecord = new EXCEPTION_RECORD( );
	}


	PEXCEPTION_RECORD ex = ExceptionInfo->ExceptionRecord;

	DWORD exceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

	if ( ( exceptionCode & ERROR_SEVERITY_ERROR ) != ERROR_SEVERITY_ERROR ) {
		cerr << "Found ERROR_SEVERITY_ERROR..." << endl;
		DumpExceptionInfoToFile( ExceptionInfo );
		return ExceptionContinueSearch;
	}


	if ( exceptionCode & APPLICATION_ERROR_MASK ) {
		cerr << "Found APPLICATION_ERROR_MASK..." << endl;
		DumpExceptionInfoToFile( ExceptionInfo );
		return ExceptionContinueSearch;
	}


	char continueablecode[ 200 ];
	if ( ( ex->ExceptionFlags & EXCEPTION_NONCONTINUABLE ) == 0 )
	{
		sprintf_s( continueablecode, 200, "%s:%X:%s", "Test: [VEH]ExceptionContinueExecution", ex->ExceptionCode, to_string( ex->ExceptionFlags & EXCEPTION_NONCONTINUABLE ).c_str( ) );
		AddNewLineToDotaChatLog( continueablecode );
		if ( *InGame )
		{
			dumperrorid--;
			TopLevelExceptionFilter( ExceptionInfo );
		}
		DumpExceptionInfoToFile( ExceptionInfo );
		return ExceptionContinueSearch;
	}



	IsVEHex = TRUE;


	sprintf_s( continueablecode, 200, "%s:%X:%s", "Test: [VEH]TopLevelExceptionFilter", ex->ExceptionCode, to_string( ex->ExceptionFlags & EXCEPTION_NONCONTINUABLE ).c_str( ) );
	cerr << continueablecode << endl;
	AddNewLineToDotaChatLog( continueablecode );
	dumperrorid--;
	TopLevelExceptionFilter( ExceptionInfo );
	DumpExceptionInfoToFile( ExceptionInfo );

	return ExceptionContinueSearch;
}


void InitTopLevelExceptionFilter( )
{
	//SetUnhandledExceptionFilter( 0 );
	//SetUnhandledExceptionFilter( TopLevelExceptionFilter );
	AddVectoredExceptionHandler( 0, DotaVectoredToSehHandler );
}

void ResetTopLevelExceptionFilter( )
{
	SetUnhandledExceptionFilter( OriginFilter );
}

#define MAX_PROCESSES 1024 


DWORD FindProcess( char * lpcszFileName )
{
	LPDWORD lpdwProcessIds;
	char *  lpszBaseName;
	HANDLE  hProcess;
	DWORD   i, cdwProcesses, dwProcessId = 0;

	lpdwProcessIds = ( LPDWORD )HeapAlloc( GetProcessHeap( ), 0, MAX_PROCESSES * sizeof( DWORD ) );
	if ( lpdwProcessIds != NULL )
	{
		if ( EnumProcesses( lpdwProcessIds, MAX_PROCESSES * sizeof( DWORD ), &cdwProcesses ) )
		{
			lpszBaseName = ( char * )HeapAlloc( GetProcessHeap( ), 0, MAX_PATH * sizeof( TCHAR ) );
			if ( lpszBaseName != NULL )
			{
				cdwProcesses /= sizeof( DWORD );
				for ( i = 0; i < cdwProcesses; i++ )
				{
					hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[ i ] );
					if ( hProcess != NULL )
					{
						if ( GetModuleBaseNameA( hProcess, NULL, lpszBaseName, MAX_PATH ) > 0 )
						{
							if ( !lstrcmpiA( lpszBaseName, lpcszFileName ) )
							{
								dwProcessId = lpdwProcessIds[ i ];
								CloseHandle( hProcess );
								break;
							}
						}
						CloseHandle( hProcess );
					}
				}
				HeapFree( GetProcessHeap( ), 0, ( LPVOID )lpszBaseName );
			}
		}
		HeapFree( GetProcessHeap( ), 0, ( LPVOID )lpdwProcessIds );
	}
	return dwProcessId;
}

std::string GetPlatformName( )
{
	if ( GetModuleHandleA( "iccwc3.icc" ) )
		return "[iCCup]";
	if ( GetModuleHandleA( "InputHook.dll" ) && GetModuleHandleA( "Overlay.dll" ) )
		return "[Garena Plus]";
	if ( FindProcess( "rgc.exe" ) )
		return "[RGC]";
	if ( GetModuleHandleA( "mroc.dll" ) || FindProcess( "myroc.exe" ) )
		return "[RGC]";
	if ( !GetModuleHandleA( "w3lh.dll" ) )
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
	string LogTempStr;
	ostringstream BugReport;
	int FuncCount = 0;
	char gamever[ 20 ]{ 0 };
	char lasterror[ 20 ]{ 0 };
	char dllcrc32[ 20 ]{ 0 };
	char EspDump[ 16 ]{ 0 };
	DWORD EspDumpBytes = 0;

	bDllLogEnable = FALSE;
	LONG result = NULL;

	PrintErrorLog( a3, "%s", "[Dota Allstars Error Handler v1.1]" );

	sprintf_s( gamever, 20, "%X", GameVersion );

	BugReport << "GameVer: 1." << gamever << std::endl;

	PrintErrorLog( a3, "\n%s\n", "Process ingame..." );




	sprintf_s( lasterror, 20, "%X", ( unsigned int )GetLastError( ) );

	BugReport << ( const char * )( *InGame ? "InGame" : "NotInGame" ) << std::endl;


	sprintf_s( dllcrc32, 20, "%X", ( unsigned int )GetDllCrc32( ) );

	BugReport << "[DLL_CRC32]: " << dllcrc32 << std::endl;

	BugReport << "[DotaHelperLog]" << std::endl;


	if ( ESP_for_DUMP )
		ReadProcessMemory( GetCurrentProcess( ), ( void * )( ESP_for_DUMP - 16 ), EspDump, 16, &EspDumpBytes );



	BugReport << ( const char * )( *IsWindowActive ? "WindowOpened" : "WindowMinimized" ) << std::endl;

	PrintErrorLog( a3, "\n%s\n", BugReport.str( ).c_str( ) );

	PrintErrorLog( a3, "\n%s\n", "Process ingame end" );


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

	PrintErrorLog( a3, "%s\n", "Process ingame...[OK]" );
	PrintErrorLog( a3, "%s\n", "Process other data..." );



	BugReport << "[SystemTime]: " << a5->wYear << "." << a5->wMonth << "." << a5->wDay << " " << a5->wHour << ":" << a5->wMinute << ":" << a5->wSecond << std::endl;
	BugReport << "[OS]: " << GetOSDisplayString( ) << std::endl;
	BugReport << "[CMD]: " << GetCommandLineA( ) << std::endl;

	BugReport << "[LastError]: \"" << GetLastErrorAsString( ) << "\", CODE: " << lasterror << std::endl;
	BugReport << "[Platform]: " << GetPlatformName( ) << std::endl;
	BugReport << "[Exception Info]: " << std::endl;
	BugReport << LastExceptionError << std::endl;
	BugReport << "[ESP]: " << ConvertMemoryToHexReverse( ( unsigned char* )&ESP_for_DUMP, 4 ) << ". [DUMP]:" << ConvertMemoryToHexReverse( ( unsigned char* )&EspDump, 16 ) << std::endl;

	BugReport << "[DLL_LOG]: " << std::endl;
	PrintErrorLog( a3, "%s\n", "Process other data...[OK]" );

	string LastError1, LastError2, LastError3, LastError4;


	PrintErrorLog( a3, "%s\n", "[DotaHelperLog]" );

	for ( string& s : DotaHelperLog )
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

	for ( string& s : JNativesFuncLog )
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

	for ( string& s : JassLogList )
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


	for ( string& s : DotaChatLog )
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
	SendHttpPostRequest( "http://d1stats.ru/fatal.php", ( "msg=" + strBugReport ).c_str( ) );
	//	SendHttpGetRequest( "http://d1stats.ru", ("/fatal.php?msg=QQGETQQ" + BugReport.str( ) ).c_str( ) );

	PrintErrorLog( a3, "%s", "[Dota Allstars CNET events]" );
	for ( int& EventID : CNetEvents )
	{
		PrintErrorLog( a3, "%s(%i)", GetNetEventStrByID( EventID ), EventID );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard1 log]" );
	for ( string& s : Blizzard1Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard2 log]" );
	for ( string& s : Blizzard2Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard3 log]" );
	for ( string& s : Blizzard3Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard4 log]" );
	for ( string& s : Blizzard4Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "-----------------------------------------------" );
	for ( string& s : Blizzard4Log_2 )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard5 log]" );
	for ( string& s : Blizzard5Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Blizzard6 log]" );
	for ( string& s : Blizzard6Log )
	{
		PrintErrorLog( a3, "%s", s.c_str( ) );
	}
	PrintErrorLog( a3, "%s", "[Dota Allstars Error Handler END]" );

	result = StormErrorHandler_ptr( a1, PrintErrorLog, a3, a4, a5 );

	return result;
}


int oldretaddr;
int oldespvalue;
int newespvalue;
int oldebpvalue;



int save_eax, save_ebx, save_ecx, save_edx, save_esi, save_edi, save_ebp, save_esp;
short save_ax, save_cx, save_dx, save_bx, save_bp, save_si, save_di;

#define safepopad \
	__asm mov eax, save_eax \
	__asm mov ebx, save_ebx \
	__asm mov ecx, save_ecx \
	__asm mov edx, save_edx \
	__asm mov esi, save_esi \
	__asm mov edi, save_edi \
	__asm mov ax, save_ax \
	__asm mov cx, save_cx \
	__asm mov dx, save_dx \
	__asm mov bx, save_bx \
	__asm mov bp, save_bp \
	__asm mov si, save_si \
	__asm mov di, save_di \
	__asm mov esp, save_esp \
	__asm mov ebp, oldretaddr \
	__asm mov [esp], ebp \
	__asm mov esp, save_esp \
	__asm mov ebp, save_ebp 


#define safepushad \
	__asm mov save_eax, eax  \
	__asm mov save_ebx, ebx  \
	__asm mov save_ecx, ecx  \
	__asm mov save_edx, edx  \
	__asm mov save_esi, esi  \
	__asm mov save_edi, edi  \
	__asm mov save_ebp, ebp  \
	__asm mov save_esp, esp  \
	__asm mov save_ax, ax  \
	__asm mov save_cx, cx  \
	__asm mov save_dx, dx  \
	__asm mov save_bx, bx  \
	__asm mov save_bp, bp  \
	__asm mov save_si, si  \
	__asm mov save_di, di  \
	__asm mov eax, [esp] \
	__asm mov oldretaddr, eax 


//
//
//
//#define safepopad \
//	__asm mov esp, newespvalue \
//	__asm popa \
//	__asm popad \
//	__asm mov ebp, oldretaddr \
//	__asm mov [esp], ebp \
//	__asm mov ebp, oldebpvalue 
//
//
//#define safepushad \
//	__asm mov oldespvalue, esp \
//	__asm mov oldebpvalue, ebp \
//	__asm mov esp,[esp] \
//	__asm mov oldretaddr, esp \
//	__asm mov esp, oldespvalue \
//	__asm pushad \
//	__asm pusha \
//	__asm mov newespvalue, esp \
//	__asm mov esp, oldespvalue





BlizzardDebug1 BlizzardDebug1_org = NULL;
BlizzardDebug1 BlizzardDebug1_ptr;

void __fastcall BlizzardDebug1_my_2( const char * str )
{
	if ( str && *str != '\0' )
	{
		AddNewLineToBlizzard1Log( str );
	}
	else
	{
		AddNewLineToBlizzard1Log( "BlizzardDebug1:NULL" );
	}
}

void __declspec( naked ) BlizzardDebug1_my( )
{
	__asm
	{
		safepushad;
		add esp, 4;
		call BlizzardDebug1_my_2;
		safepopad;
		ret;
	}
}


BlizzardDebug2 BlizzardDebug2_org = NULL;
BlizzardDebug2 BlizzardDebug2_ptr;
char tmpdebug[ 1024 ];

void __cdecl BlizzardDebug2_my_2( const char * src, int lineid, const char * classname )
{
	BOOL srcok = src && *src != '\0';
	BOOL classnameok = classname && *classname != '\0';

	sprintf_s( tmpdebug, 1024, "Sorce:%s\nLine:%i\nClass:%s", srcok ? src : "NULL", lineid, classnameok ? classname : "NULL" );
	//PrintText( tmpdebug );
	AddNewLineToBlizzard2Log( tmpdebug );
}

void __declspec( naked ) BlizzardDebug2_my( )
{
	__asm
	{
		safepushad;
		add esp, 4;
		call BlizzardDebug2_my_2;
		safepopad;
		ret;
	}
}


BlizzardDebug3 BlizzardDebug3_org = NULL;
BlizzardDebug3 BlizzardDebug3_ptr;

void __cdecl BlizzardDebug3_my_2( const char * format, ... )
{
	if ( !format || *format == '\0' )
	{
		AddNewLineToBlizzard3Log( "BlizzardDebug3:NULL" );
		return;
	}
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( tmpdebug, 1024, format, argptr );
	va_end( argptr );
	AddNewLineToBlizzard3Log( tmpdebug );
}

void __declspec( naked ) BlizzardDebug3_my( )
{
	__asm
	{
		safepushad;
		add esp, 4;
		call BlizzardDebug3_my_2;
		safepopad;
		ret;
	}
}


BlizzardDebug4 BlizzardDebug4_org = NULL;
BlizzardDebug4 BlizzardDebug4_ptr;
void __cdecl BlizzardDebug4_my_2( BOOL type1, const char * format, ... )
{
	if ( !format || *format == '\0' )
	{
		AddNewLineToBlizzard4Log( "BlizzardDebug4:NULL" );
		return;
	}
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( tmpdebug, 1024, format, argptr );
	va_end( argptr );
	if ( type1 )
		AddNewLineToBlizzard4Log( tmpdebug );
	else
		AddNewLineToBlizzard4Log_2( tmpdebug );
}


void __declspec( naked ) BlizzardDebug4_my( )
{
	__asm
	{
		safepushad;
		add esp, 4;
		call BlizzardDebug4_my_2;
		safepopad;
		ret;
	}
}


BlizzardDebug5 BlizzardDebug5_org = NULL;
BlizzardDebug5 BlizzardDebug5_ptr;

void __cdecl BlizzardDebug5_my_2( const char * format, ... )
{
	if ( !format || *format == '\0' )
	{
		AddNewLineToBlizzard5Log( "BlizzardDebug5:NULL" );
		return;
	}
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( tmpdebug, 1024, format, argptr );
	va_end( argptr );
	AddNewLineToBlizzard5Log( tmpdebug );
}


void __declspec( naked ) BlizzardDebug5_my( )
{
	__asm
	{
		safepushad;
		add esp, 4;
		call BlizzardDebug5_my_2;
		safepopad;
		ret;
	}
}


BlizzardDebug6 BlizzardDebug6_org = NULL;
BlizzardDebug6 BlizzardDebug6_ptr;

void __cdecl BlizzardDebug6_my_2( const char * format, ... )
{
	if ( !format || *format == '\0' )
	{
		AddNewLineToBlizzard6Log( "BlizzardDebug6:NULL" );
		return;
	}
	va_list argptr;
	va_start( argptr, format );
	vsprintf_s( tmpdebug, 1024, format, argptr );
	va_end( argptr );
	AddNewLineToBlizzard6Log( tmpdebug );
}


void __declspec( naked ) BlizzardDebug6_my( )
{
	__asm
	{
		safepushad;
		add esp, 4;
		call BlizzardDebug6_my_2;
		safepopad;
		ret;
	}
}


int __stdcall StartExtraErrorHandler( int )
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

void __stdcall EnableErrorHandler( int )
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



	StartExtraErrorHandler( 0 );
}

void __stdcall DisableErrorHandler( int )
{

	//if ( !DotaChatLog.empty( ) )
	//	DotaChatLog.clear( );

	//if ( !JassLogList.empty( ) )
	//	JassLogList.clear( );

	//if ( !DotaHelperLog.empty( ) )
	//	DotaHelperLog.clear( );

	//if ( !JNativesFuncLog.empty( ) )
	//	JNativesFuncLog.clear( );

	//if ( !CNetEvents.empty( ) )
	//	CNetEvents.clear( );

	//if ( !Blizzard1Log.empty( ) )
	//	Blizzard1Log.clear( );

	//if ( !Blizzard2Log.empty( ) )
	//	Blizzard2Log.clear( );

	//if ( !Blizzard3Log.empty( ) )
	//	Blizzard3Log.clear( );

	//if ( !Blizzard4Log.empty( ) )
	//	Blizzard4Log.clear( );

	//if ( !Blizzard4Log_2.empty( ) )
	//	Blizzard4Log_2.clear( );

	//if ( !Blizzard5Log.empty( ) )
	//	Blizzard5Log.clear( );

	//if ( !Blizzard6Log.empty( ) )
	//	Blizzard6Log.clear( );

	if ( StormErrorHandler_org )
	{
		MH_DisableHook( StormErrorHandler_org );
		StormErrorHandler_org = NULL;
	}

	if ( LookupNative_org )
	{
		MH_DisableHook( LookupNative_org );
		LookupNative_org = NULL;
	}

	if ( LookupJassFunc_org )
	{
		MH_DisableHook( LookupJassFunc_org );
		LookupJassFunc_org = NULL;
	}

	if ( ProcessNetEvents_org )
	{
		MH_DisableHook( ProcessNetEvents_org );
		ProcessNetEvents_org = NULL;
	}

	if ( BlizzardDebug1_org )
	{
		MH_DisableHook( BlizzardDebug1_org );
		BlizzardDebug1_org = NULL;
	}

	if ( BlizzardDebug2_org )
	{
		MH_DisableHook( BlizzardDebug2_org );
		BlizzardDebug2_org = NULL;
	}

	if ( BlizzardDebug3_org )
	{
		MH_DisableHook( BlizzardDebug3_org );
		BlizzardDebug3_org = NULL;
	}

	if ( BlizzardDebug4_org )
	{
		MH_DisableHook( BlizzardDebug4_org );
		BlizzardDebug4_org = NULL;
	}

	if ( BlizzardDebug5_org )
	{
		MH_DisableHook( BlizzardDebug5_org );
		BlizzardDebug5_org = NULL;
	}

	if ( BlizzardDebug6_org )
	{
		MH_DisableHook( BlizzardDebug6_org );
		BlizzardDebug6_org = NULL;
	}



}


#pragma optimize("",on)


#else
int __stdcall DisableErrorHandler( int )
{
	return 0;
}
int __stdcall EnableErrorHandler( int )
{
	return 0;
}
int __stdcall JassLog( int )
{
	return 0;
}
int __stdcall StartExtraErrorHandler( int )
{
	return 0;
}
int __stdcall TraceEsp_Print( int )
{
	return 0;
}
int __stdcall DllLogEnable( int )
{
	return 0;
}

#endif