#include "Main.h"
// only for test ( http://i.imgur.com/40qQr8X.jpg )
BOOL NeedOpenConfigWindow = FALSE;

void Do_Nothing( )
{

};

struct SimpleMessageBoxCallBack
{
	int _NothingFunc;
	char _zero[ 8 ];
	int CallBackFuncAddr;
};

SimpleMessageBoxCallBack Global;

struct MessageCallBackStruct
{
	SimpleMessageBoxCallBack * vtable;
	int zero;
};

MessageCallBackStruct MyCallBackTest;


char ConfigPath[ MAX_PATH ];


class WarcraftFramesClass
{

public:
	// Supported frames
	enum FRAME_TYPE :int
	{
		FRAME_TEXT,
		FRAME_MENU,
		FRAME_BUTTON,
		FRAME_EDITBOX,
		FRAME_UNKNOWN
	};


	~WarcraftFramesClass( )
	{
		Destructor( );
	}

	WarcraftFramesClass( )
	{
		SetAllAddrs( );
		FrameAddr = 0;
	}
	WarcraftFramesClass( const char * framename )
	{
		if ( !framename )
		{
			SetAllAddrs( );
			FrameAddr = GetFrameAddress( framename, 0 );
		}
	}

	WarcraftFramesClass( const char * framename, int frameid )
	{
		SetAllAddrs( );
		FrameAddr = GetFrameAddress( framename, frameid );
	}

	WarcraftFramesClass( const char * framename, FRAME_TYPE FrameType )
	{
		SetAllAddrs( );
		FrameAddr = GetFrameAddress( framename, 0 );
		this->FrameType = FrameType;
	}

	WarcraftFramesClass( const char * framename, int frameid, FRAME_TYPE FrameType )
	{
		SetAllAddrs( );
		FrameAddr = GetFrameAddress( framename, frameid );
		this->FrameType = FrameType;
	}

	WarcraftFramesClass( const char * framename, FRAME_TYPE FrameType, int GameDll )
	{
		SetAllAddrs( );
		FrameAddr = GetFrameAddress( framename, 0 );
		this->FrameType = FrameType;
		this->GameDll = GameDll;
	}

	WarcraftFramesClass( const char * framename, int frameid, FRAME_TYPE FrameType, int GameDll )
	{
		SetAllAddrs( );
		FrameAddr = GetFrameAddress( framename, frameid );
		this->FrameType = FrameType;
		this->GameDll = GameDll;
	}

	void SetGameDllAddr( int GameDll )
	{
		this->GameDll = GameDll;
	}

	void SetGetFrameAddr( int GetFrameAddress )
	{
		this->GetFrameAddress = ( GetFrameAddress_p )GetFrameAddress;
	}

	void SetFrameType( FRAME_TYPE FrameType )
	{
		this->FrameType = FrameType;
	}

	int GetFrameAddr( )
	{
		return FrameAddr;
	}

	int FrameGetTextAddr( )
	{
		if ( !FrameAddr )
			return 0;

		switch ( FrameType )
		{
		case FRAME_TEXT:
		case FRAME_BUTTON:
		case FRAME_EDITBOX:
			return *( int * )( FrameAddr + 0x1EC ) ? FrameAddr + 0x1EC : 0;
		case FRAME_MENU:
			int offset = *( int * )( FrameAddr + 0x1E4 );
			if ( offset )
			{
				offset = *( int * )( offset + 0x1E4 );
				if ( offset )
				{
					return *( int * )( offset + 0x1EC ) ? offset + 0x1EC : 0;
				}
			}
			break;
		}
		return 0;
	}


	int FrameGetTextFrameAddr( )
	{
		if ( !FrameAddr )
			return 0;

		switch ( FrameType )
		{
		case FRAME_TEXT:
		case FRAME_BUTTON:
		case FRAME_EDITBOX:
			return FrameAddr;
		case FRAME_MENU:
			int offset = *( int * )( FrameAddr + 0x1E4 );
			if ( offset )
			{
				offset = *( int * )( offset + 0x1E4 );
				return offset;
			}
			break;
		}
		return 0;
	}

	const char * FrameGetText( )
	{
		int textaddr = FrameGetTextAddr( );
		if ( !textaddr )
		{
			return NULL;
		}
		const char * rettext = ( char * )*( int* )textaddr;
		if ( !rettext )
			return NULL;
		return rettext;
	}

	void WriteTextSafe( const char * text )
	{
		if ( lockedframe ) return;

		int textaddr = FrameGetTextAddr( );
		if ( !textaddr )
		{
			return;
		}

		if ( text != NULL &&  FrameGetText( ) != NULL )
		{
			if ( _stricmp( text, FrameGetText( ) ) == 0 )
				return;
		}

		SetFrameText( FrameGetTextFrameAddr( ), text );

	}

	void SetEditboxLength( int len )
	{
		SetEditBoxLength(FrameAddr, len );
	}

	void WriteTextSafe( const char * text, int len )
	{
		if ( lockedframe ) return;

		int textaddr = FrameGetTextAddr( );
		if ( !textaddr )
		{
			return;
		}

		if ( text != NULL &&  FrameGetText( ) != NULL )
		{
			if ( _stricmp( text, FrameGetText( ) ) == 0 )
				return;
		}

		SetFrameText( FrameGetTextFrameAddr( ), text );
	}

	int GetFrameTextAddr( )
	{
		if ( !FrameAddr )
			return 0;

		switch ( FrameType )
		{
		case FRAME_TEXT:
		case FRAME_BUTTON:
		case FRAME_EDITBOX:
			return FrameAddr;
		case FRAME_MENU:
			int offset = *( int * )( FrameAddr + 0x1E4 );
			if ( offset )
			{
				offset = *( int * )( offset + 0x1E4 );
				if ( offset )
				{
					return offset;
				}
			}
			break;
		}
		return 0;
	}

	bool IsLocked( )
	{
		return lockedframe;
	}

	void Lock( )
	{
		lockedframe = true;
	}

	void UnLock( )
	{
		lockedframe = false;
	}

	void Reset( )
	{
		Destructor( );
		SetAllAddrs( );
	}

private:

	typedef int( __fastcall * GetFrameAddress_p )( const char * name, int id );
	GetFrameAddress_p GetFrameAddress = 0;

	typedef void *( __thiscall * UpdateFrameTextSizep )( void *a1, int a2 );
	UpdateFrameTextSizep UpdateFrameTextSize = 0;

	typedef void **( __thiscall * SetFrameText_p )( int frameaddr, const char * newtext );
	SetFrameText_p SetFrameText = 0;

	typedef int( __thiscall * FreeFrameText_p )( int frameaddr, int freetype );
	FreeFrameText_p FreeFrameText = 0;

	typedef int( __thiscall * SetEditBoxLength_p )( int frameaddr, int textsize );
	SetEditBoxLength_p SetEditBoxLength = 0;

	int GameDll = 0;
	int FrameAddr = 0;

	FRAME_TYPE FrameType = FRAME_UNKNOWN;

	bool laststringcmp = false;
	bool sizechanged = false;
	bool lockedframe = false;

	void SetAllAddrs( )
	{
		hexstrlen = strlen( hexstr );

		if ( GameDll == 0 )
		{
			GameDll = ( int )GetModuleHandleA( "Game.dll" );
		}

		if ( GetFrameAddress == 0 )
		{
			GetFrameAddress = ( GetFrameAddress_p )( GameDll + 0x5FA970 );
		}

		if ( UpdateFrameTextSize == 0 )
		{
			UpdateFrameTextSize = ( UpdateFrameTextSizep )( GameDll + 0x605CC0 );
		}

		if ( SetFrameText == 0 )
		{
			SetFrameText = ( SetFrameText_p )( GameDll + 0x611D40 );
		}

		if ( FreeFrameText == 0 )
		{
			FreeFrameText = ( FreeFrameText_p )( GameDll + 0x611A80 );
		}

		if ( SetEditBoxLength == 0 )
		{
			SetEditBoxLength = ( SetEditBoxLength_p )( GameDll + 0x616250 );
		}

	}

	void Destructor( )
	{

		FrameAddr = 0;
		GetFrameAddress = 0;
		UpdateFrameTextSize = 0;
		GameDll = 0;
		laststringcmp = 0;
		laststringcmp = false;
		sizechanged = false;
		lockedframe = false;
		hexstrlen = 0;

		FrameType = FRAME_UNKNOWN;
	}

	const char * hexstr = "0123456789ABCDEFabcdef";
	int hexstrlen = 0;

	bool IsHex( char c )
	{
		for ( int i = 0; i < hexstrlen; i++ )
		{
			if ( hexstr[ i ] == c )
				return true;
		}
		return false;
	}

};



void ReadAllConfig( )
{
	WarcraftFramesClass tmpclass = WarcraftFramesClass( "SkillButton1",0,  WarcraftFramesClass::FRAME_TEXT );
	if ( tmpclass.GetFrameAddr( ) > 0 )
	{
	
		char addr[ 200 ];
		sprintf_s( addr, 200, "%X->%X->%X", tmpclass.GetFrameAddr( ), tmpclass.GetFrameTextAddr( ), tmpclass.FrameGetTextAddr( ) );
		MessageBoxA( 0, addr, addr, 0 );
		tmpclass.SetEditboxLength( 10 );
	}
}


void WriteAllConfig( )
{

}



void __fastcall SimpleCallbackFunction( int, int, int )
{
	if ( NeedOpenConfigWindow )
	{
		NeedOpenConfigWindow = FALSE;
		WriteAllConfig( );
		LoadFrameDefList( "UI\\FrameDef\\FrameDef.toc", 0 );
	}
}
struct FrameDefStatus
{
	int FDefVtable;
	int zeroint;
	int this_add_8;
	int this_add_8_negative;
	int this_one;
};
FrameDefStatus fStatus;


int __stdcall ShowConfigWindow( const char * filename)
{
	sprintf_s( ConfigPath, MAX_PATH, "%s", filename );


	if ( !NeedOpenConfigWindow )
	{
		NeedOpenConfigWindow = TRUE;

		Global.CallBackFuncAddr = ( int )SimpleCallbackFunction;
		Global._NothingFunc = ( int )Do_Nothing;
		MyCallBackTest.zero = 0;
		MyCallBackTest.vtable = &Global;

	/*	fStatus.FDefVtable = GameDll + 0x875E98;
		fStatus.this_one = 1;
		fStatus.zeroint = 0;
		fStatus.this_add_8 = ( int )&fStatus + 8;
		fStatus.this_add_8_negative = ~( fStatus.this_add_8 );*/

		LoadFrameDefList( "UI\\FrameDef\\FrameDef.toc", 0 );
		Game_Wc3MessageBox( 2, "Dota Helper Config Editor [BETA]", 0, ( int )&MyCallBackTest, 0, 4, 1 );
		ReadAllConfig( );
	}

	return TRUE;
}