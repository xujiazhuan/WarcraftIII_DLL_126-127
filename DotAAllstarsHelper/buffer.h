#ifndef BUFFER_H
#define BUFFER_H

//extern int memoryleakcheck;

class Buffer
{
public:
	char *buf;
	unsigned long length;
	/*~Buffer( )
	{
		Clear( );
	}*/
	Buffer( )
	{
		buf = 0;
		length = 0;
	}
	Buffer( unsigned long l )
	{
	//	memoryleakcheck++;
		length = l;
		buf = new char[ l + 1 ];
		buf[ l ] = '\0';
	}
	Buffer( char* b, unsigned long l )
	{
		buf = b;
		length = l;
	}
	void Resize( unsigned long l )
	{
		Clear( );
		buf = new char[ l + 1 ];
		buf[ l ] = '\0';
		length = l;
	}

	char * GetData( )
	{
		return buf;
	}
	char * GetData( int offset )
	{
		return buf + offset;
	}

	unsigned long GetSize( )
	{
		return length;
	}

	void Clear( )
	{
	//	memoryleakcheck--;
		length = 0;
		delete[ ] buf;
		buf = NULL;
	}

	Buffer&  Clone( Buffer& CopyObject )
	{
		Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length ); 
		return ( *this );
	}
	
	Buffer& operator =(  Buffer& CopyObject )
	{
		/*Resize( CopyObject.length );
		std::memcpy( buf, CopyObject.GetData( ), length );*/
		length = CopyObject.length;
		buf = CopyObject.buf;
		return ( *this );
	}
	Buffer& operator =(  std::string& CopyString )
	{
		Resize( static_cast<INT>( CopyString.size( ) ) );
		std::memcpy( buf, CopyString.c_str( ), length );
		return ( *this );
	}

	CHAR& operator []( INT Index ) 
	{
		return buf[ Index ];
	}
};

typedef struct BufferList
{
	char **buf;
	unsigned long length;
	BufferList( )
	{
		buf = 0;
		length = 0;
	}
	BufferList( unsigned long l )
	{
		buf = new char*[ l ];
		length = l;
	}
	BufferList( char** b, unsigned long l )
	{
		buf = b;
		length = l;
	}
} BufferList;
#endif
