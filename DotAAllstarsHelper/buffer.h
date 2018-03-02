#include "Main.h"
#include "Storm.h"

#pragma pack(push,1)


//extern int memoryleakcheck;

void  __stdcall AddNewLineToJassLog( const char * s );
void __stdcall  AddNewLineToDotaChatLog( const char * s );
void __stdcall  AddNewLineToDotaHelperLog( const char * s, int line );//( const char * s, int line );
void __stdcall  AddNewLineToJassNativesLog( const char * s );
void __stdcall EnableErrorHandler( int );
void __stdcall DisableErrorHandler( int );


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
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		buf = 0;
		length = 0;
	}
	StormBuffer( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		//	memoryleakcheck++;
		length = l;
		buf = ( char * )Storm::MemAlloc( l + 1 );
		buf[ l ] = '\0';
	}
	StormBuffer( char* b, unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		buf = b;
		length = l;
	}
	void Resize( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		Clear( );
		buf = ( char * )Storm::MemAlloc( l + 1 );
		buf[ l ] = '\0';
		length = l;
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	}

	char * GetData( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		return buf;
	}
	char * GetData( int offset )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		return buf + offset;
	}

	unsigned long GetSize( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		return length;
	}

	void Clear( )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		//	memoryleakcheck--;
		length = 0;
		if ( buf != NULL )
		{
			Storm::MemFree( buf );
		}
		buf = NULL;
	}

	StormBuffer&  Clone( StormBuffer& CopyObject )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length );
		return ( *this );
	}

	StormBuffer& operator =( StormBuffer& CopyObject )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
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
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		Resize( static_cast< INT >( CopyString.size( ) ) );
		std::memcpy( buf, CopyString.c_str( ), length );
		return ( *this );
	}

	CHAR& operator []( INT Index )
	{
//#ifdef DOTA_HELPER_LOG
//		AddNewLineToDotaHelperLog( __func__,__LINE__ );
//#endif
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
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		buf = 0;
		length = 0;
	}
	StormBufferList( unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		buf = ( char** )Storm::MemAlloc( l );
		length = l;
	}
	StormBufferList( char** b, unsigned long l )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
		buf = b;
		length = l;
	}
} StormBufferList;


#pragma pack(pop)