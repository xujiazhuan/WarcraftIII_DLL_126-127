#pragma once

class HTTPRequest
{
private:
	const std::string Host;
	u_short Port;
	int ErrorCode;
	SOCKET Sock;
	std::string Response;
	bool loop_recieve( );
	bool resolve_and_connect( );
public:
	HTTPRequest( const std::string& host, u_short  port );
	~HTTPRequest( );
	std::string get_response( );
	int GetErrorCode( );
	bool get_request( const std::string& path );
	bool post_request( const std::string& path, const std::string& dat );

};