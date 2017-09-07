#include "Main.h"
#include "Storm.h"

#pragma pack(push,1)


#pragma region ErrorHandler.cpp
#ifdef DOTA_HELPER_LOG
void  __stdcall AddNewLineToJassLog( const char * s );
void __stdcall  AddNewLineToDotaChatLog( const char * s );
void __stdcall  AddNewLineToDotaHelperLog( const char * s, int line );
void __stdcall  AddNewLineToJassNativesLog( const char * s );
void __stdcall EnableErrorHandler( int );
void __stdcall DisableErrorHandler( int );
void DumpExceptionInfoToFile( _EXCEPTION_POINTERS *ExceptionInfo );

extern void ResetTopLevelExceptionFilter( );
extern LPTOP_LEVEL_EXCEPTION_FILTER OriginFilter;
extern BOOL bDllLogEnable;
typedef LONG( __fastcall * StormErrorHandler )( int a1, void( *a2 )( int, const char *, ... ), int a3, BYTE *a4, LPSYSTEMTIME a5 );
extern StormErrorHandler StormErrorHandler_org;
typedef int( __fastcall *LookupNative )( int global, int unused, const char * name );
extern LookupNative LookupNative_org;
typedef signed int( __fastcall * LookupJassFunc )( int global, int unused, const char * funcname );
extern LookupJassFunc LookupJassFunc_org;
typedef void( __fastcall * ProcessNetEvents )( void * data, int unused_, int Event );
extern ProcessNetEvents ProcessNetEvents_org;
typedef void( __fastcall * BlizzardDebug1 ) ( const char*str );
extern BlizzardDebug1 BlizzardDebug1_org;
typedef void( __cdecl * BlizzardDebug2 )( const char * src, int lineid, const char * classname );
extern BlizzardDebug2 BlizzardDebug2_org;
typedef void( __cdecl * BlizzardDebug3 )( const char *format, ... );
extern BlizzardDebug3 BlizzardDebug3_org;
typedef void( __cdecl * BlizzardDebug4 )( BOOL type1, const char *format, ... );
extern BlizzardDebug4 BlizzardDebug4_org;
typedef void( __cdecl * BlizzardDebug5 )( const char *format, ... );
extern BlizzardDebug5 BlizzardDebug5_org;
typedef void( __cdecl * BlizzardDebug6 )( const char *format, ... );
extern BlizzardDebug6 BlizzardDebug6_org;
#endif
#pragma endregion

//extern int memoryleakcheck;



//extern int memoryleakcheck;

class StormBuffer
{
private:

public:
	char *buf;
	unsigned long length;
	/*~StormBuffer( )
	{
	Clear( );
	}*/
	StormBuffer( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = 0;
		length = 0;
	}
	StormBuffer( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		//	memoryleakcheck++;
		length = l;
		buf = ( char * )Storm::MemAlloc( l + 1 );
		buf[ l ] = '\0';
	}
	StormBuffer( char* b, unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = b;
		length = l;
	}
	void Resize( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		Clear( );
		buf = ( char * )Storm::MemAlloc( l + 1 );
		buf[ l ] = '\0';
		length = l;
	}

	char * GetData( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return buf;
	}
	char * GetData( int offset )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return buf + offset;
	}

	unsigned long GetSize( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return length;
	}

	void Clear( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		//	memoryleakcheck--;
		length = 0;
		Storm::MemFree( buf );
		buf = NULL;
	}

	StormBuffer&  Clone( StormBuffer& CopyObject )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length );
		return ( *this );
	}

	StormBuffer& operator =( StormBuffer& CopyObject )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		/*Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length );*/
		length = CopyObject.length;
		buf = CopyObject.buf;
		return ( *this );
	}
	StormBuffer& operator =( std::string& CopyString )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		Resize( static_cast< INT >( CopyString.size( ) ) );
		std::memcpy( buf, CopyString.c_str( ), length );
		return ( *this );
	}

	CHAR& operator []( INT Index )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return buf[ Index ];
	}


};

typedef struct StormBufferList
{
	char **buf;
	unsigned long length;
	StormBufferList( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = 0;
		length = 0;
	}
	StormBufferList( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = ( char** )Storm::MemAlloc( l );
		length = l;
	}
	StormBufferList( char** b, unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		buf = b;
		length = l;
	}
} StormBufferList;


#pragma pack(pop)