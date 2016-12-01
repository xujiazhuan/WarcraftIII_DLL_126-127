#include "Main.h"
#include "HttpClass.h"

#include <WinInet.h>
#pragma comment ( lib, "Wininet.lib" ) 

int DownProgress = 0, DownStatus = 0;
string LatestDownloadedString;

string SendHttpPostRequest( const char * host, const char * path, const char * data )
{
	AddNewLineToDotaHelperLog( "SendHttpPostRequest" );
	HTTPRequest req( host, 80 );
	if ( req.GetErrorCode( ) == 0 )
	{
		req.post_request( path, data );
		LatestDownloadedString = req.get_response( );
	}
	return LatestDownloadedString;
}

string SendHttpGetRequest( const char * host, const char * path )
{
	AddNewLineToDotaHelperLog( "SendHttpGetRequest" );
	HTTPRequest req( host, 80 );
	if ( req.GetErrorCode( ) == 0 )
	{
		req.get_request( path );
		LatestDownloadedString = req.get_response( );
	}
	return LatestDownloadedString;
}


void DownloadNewMapToFile( char* szUrl, char * filepath )
{
	AddNewLineToDotaHelperLog( "DownloadNewMapToFile" );
	DownStatus = 0;
	HINTERNET hOpen = NULL;
	HINTERNET hFile = NULL;
	DWORD dataSize = 0;
	DWORD dwBytesRead = 0;
	vector<unsigned char> OutData;
	FILE * outfile = NULL;
	BOOL AllOkay = FALSE;

	if ( filepath == NULL || filepath[ 0 ] == '\0' || FileExist( filepath ) )
	{
		DownStatus = 2;
		return;
	}

	hOpen = InternetOpen( "Microsoft Internet Explorer", NULL, NULL, NULL, 0 );
	if ( !hOpen )
	{
		DownStatus = -1;
		return;
	}
	DownProgress = 10;
	hFile = InternetOpenUrl( hOpen, szUrl, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, 0 );

	if ( !hFile )
	{
		InternetCloseHandle( hOpen );
		DownStatus = -1;
		return;
	}
	DownProgress = 20;
	int code = 0;
	DWORD codeLen = 4;
	HttpQueryInfo( hFile, HTTP_QUERY_STATUS_CODE |
				   HTTP_QUERY_FLAG_NUMBER, &code, &codeLen, 0 );

	if ( code != HTTP_STATUS_OK )// 200 OK
	{
		InternetCloseHandle( hFile );
		InternetCloseHandle( hOpen );
		DownStatus = -1;
		return;
	}

	unsigned int sizeBuffer = 0;
	DWORD length = sizeof( sizeBuffer );
	HttpQueryInfo( hFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &sizeBuffer, &length, NULL );



	DownProgress = 30;

	do
	{
		dataSize += dwBytesRead;
		if ( sizeBuffer != 0 )
			DownProgress = ( int ) ( ( dataSize * 100 ) / sizeBuffer );

		dwBytesRead = 0;
		unsigned char buffer[ 2000 ];
		BOOL isRead = InternetReadFile( hFile, ( LPVOID ) buffer, _countof( buffer ), &dwBytesRead );
		if ( dwBytesRead && isRead )
		{
			AllOkay = TRUE;
			for ( unsigned int i = 0; i < dwBytesRead; i++ )
				OutData.push_back( buffer[ i ] );
		}
		else
			break;
	}
	while ( dwBytesRead );

	if ( DownProgress == 30 )
	{
		DownProgress = 70;
	}

	if ( OutData.size( ) > 0 && AllOkay )
	{
		fopen_s( &outfile, filepath, "wb" );
		if ( outfile != NULL )
		{
			fwrite( &OutData[ 0 ], OutData.size( ), 1, outfile );
			OutData.clear( );
			fclose( outfile );
			DownStatus = 1;
		}
		else DownStatus = -1;
	}
	else DownStatus = -1;


	if ( DownProgress == 70 )
	{
		DownProgress = 100;
	}

	InternetCloseHandle( hFile );
	InternetCloseHandle( hOpen );

	return;
}

char * _addr = 0;
char * _request = 0;
char * _filepath = 0;

DWORD WINAPI SENDGETREQUEST( LPVOID )
{
	SendHttpGetRequest( _addr, _request );
	return 0;
}


DWORD WINAPI SENDSAVEFILEREQUEST( LPVOID )
{
	DownloadNewMapToFile( _addr, _filepath );
	return 0;
}

int __stdcall SendGetRequest( char * addr, char * request )
{
	DownProgress = 0;
	_addr = addr; _request = request;
	DownStatus = 0;
	CreateThread( 0, 0, SENDGETREQUEST, 0, 0, 0 );
	return 0;
}

int __stdcall SaveNewDotaVersionFromUrl( char * addr, char * filepath )
{
	DownProgress = 0;
	_addr = addr; _filepath = filepath;
	DownStatus = 0;
	CreateThread( 0, 0, SENDSAVEFILEREQUEST, 0, 0, 0 );
	return 0;
}

int __stdcall GetDownloadStatus( int )
{
	return DownStatus;
}

int __stdcall GetDownloadProgress( int )
{
	return DownProgress;
}

const char * __stdcall GetLatestDownloadedString( int )
{
	return LatestDownloadedString.c_str( );
}