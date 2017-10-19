
#include "Main.h"
#include "WarcraftFrameHelper.h"

using namespace NWar3Frame;

struct CFrameBuffer
{
	CWar3Frame * frame;
	string callbackfuncname;
	bool packetcallback;
	unsigned int callbackeventid;
	int FrameCode;
};

vector<CFrameBuffer> CFrameBufferList;


void DotaHelperEventEndCallback( )
{
	if ( pCurrentFrameFocusedAddr )
		*( int* )pCurrentFrameFocusedAddr = 0;

}



RCString NewCallBackFuncName = RCString( );

int NewCallBackTriggerHandle = 0;



int LastEventId = 0;

int __stdcall CFrame_GetLastEventId( int )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	return LastEventId;
}


CWar3Frame * latestcframe = NULL;

CWar3Frame *  __stdcall CFrame_GetTriggerCFrame( int )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	return latestcframe;
}

int DotaHelperFrameCallback( CWar3Frame*frame, int FrameAddr, unsigned int EventId )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	LastEventId = EventId;
	latestcframe = frame;
	//MessageBoxA( 0, frame->FrameName.c_str( ), frame->FrameName.c_str( ), 0 );
	for ( auto s : CFrameBufferList )
	{
		if ( s.frame == frame && s.callbackeventid == EventId )
		{
			if ( s.callbackfuncname.length( ) > 0 )
			{
				str2jstr( &NewCallBackFuncName, s.callbackfuncname.c_str( ) );
				ExecuteFunc( &NewCallBackFuncName );
				//MessageBoxA( 0, CWar3Frame::DumpAllFrames( ).c_str( ), " Frames ", 0 );
			}
			else if ( s.packetcallback  )
			{
				std::vector<unsigned char> SendKeyEvent;
				SendKeyEvent.push_back( 0x50 );
				// header custom packets
				SendKeyEvent.push_back( 0xFF );
				// size custom packets 
				SendKeyEvent.push_back( 0 );
				SendKeyEvent.push_back( 0 );
				SendKeyEvent.push_back( 0 );
				SendKeyEvent.push_back( 0 );
				// packet type
				int packettype = 'FRAM';
				SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&packettype, ( ( unsigned char * )&packettype ) + 4 );
				*( int* )&SendKeyEvent[ 2 ] += 4;
				// data
				int locpid = GetLocalPlayerId( );
				SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&locpid, ( ( unsigned char * )&locpid ) + 4 );
				*( int* )&SendKeyEvent[ 2 ] += 4;
				SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&s.FrameCode, ( ( unsigned char * )&s.FrameCode ) + 4 );
				*( int* )&SendKeyEvent[ 2 ] += 4;
				SendKeyEvent.insert( SendKeyEvent.end( ), ( unsigned char * )&LastEventId, ( ( unsigned char * )&LastEventId ) + 4 );
				*( int* )&SendKeyEvent[ 2 ] += 4;
				SendPacket( ( BYTE* )&SendKeyEvent[ 0 ], SendKeyEvent.size( ) );
				SendKeyEvent.clear( );
			}
			break;
		}
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif


	return 0;
}


void FrameDefHelperInitialize( )
{
	CWar3Frame::SetGlobalEventCallback( DotaHelperEventEndCallback );
	CWar3Frame::Init( GameVersion, GameDll );
	CWar3Frame::InitCallbackHook( );
	NewCallBackFuncName = RCString( );
	NewCallBackTriggerHandle = 0;
}

void FrameDefHelperUninitialize( )
{
	CFrameBufferList.clear( );
	CWar3Frame::UninitializeAllFrames( TRUE );
	CWar3Frame::UninitializeCallbackHook( );
	NewCallBackFuncName = RCString( );
	NewCallBackTriggerHandle = 0;
}

string GetCallbackFuncName( CWar3Frame * frame )
{
	if ( !frame )
		return string( );
	for ( auto s : CFrameBufferList )
	{
		if ( s.frame == frame )
		{
			return s.callbackfuncname;
		}
	}
	return string( );
}


void __stdcall CFrame_SetCustomValue( CWar3Frame*frame, int id, int value )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( id >= 1 && id <= 10 )
		frame->SetFrameCustomValue( value, id );
	else
		frame->SetFrameCustomValue( value, 1 );
}

int __stdcall CFrame_GetCustomValue( CWar3Frame*frame, int id )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( id >= 1 && id <= 10 )
		return frame->GetFrameCustomValue( id );
	return frame->GetFrameCustomValue( 1 );
}

int __stdcall CFrame_GetFrameAddress( CWar3Frame*frame )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	return frame->FrameAddr;
}

// Load custom FrameDef.toc file ( .txt file with fdf path list)
void __stdcall CFrame_LoadFramesListFile( const char * path_to_listfile, BOOL forcereload )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	CWar3Frame::LoadFrameDefFiles( path_to_listfile, forcereload );
}


// Create frame from fdf file.  relativeframe can be NULL for using 'current frame'
// show - show frame or create invisibled.
// frameid - load same file but with another id
CWar3Frame * __stdcall CFrame_CreateNewFrame( const char * FrameName, int relativeframeaddr, BOOL show, int FrameId )
{

	CWar3Frame::InitCallbackHook( );

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	CWar3Frame * returnframe = new CWar3Frame( FrameName, relativeframeaddr, show, FrameId );

	if ( !returnframe->CheckIsOk( ) )
	{
		delete returnframe;
		return NULL;
	}
	returnframe->SetCallbackFunc( DotaHelperFrameCallback );
	return returnframe;
}

// Load exists frame (get name from any .fdf files) with FrameId (use 0 if frame not created with another id)
CWar3Frame * __stdcall CFrame_LoadFrame( const char * FrameName, int FrameId )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	CWar3Frame * returnframe = new CWar3Frame( );
	if ( !returnframe->Load( FrameName, FrameId ) )
	{
		delete returnframe;
		return NULL;
	}
	returnframe->SetCallbackFunc( DotaHelperFrameCallback );
	return returnframe;
}

// Set frametype for frame . Possibe in new version i can add autodetect frametype
void __stdcall CFrame_SetFrameType( CWar3Frame * frame, CFrameType FrameType )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( string( __func__ + string( ":" ) + std::to_string( ( int )FrameType ) ).c_str( ) );
#endif
	if ( !frame )
		return;
	frame->SetFrameType( FrameType );
}

// Set model for frame now support SPRITE frametype
void __stdcall CFrame_SetFrameModel( CWar3Frame * frame, const char * modelpath )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->SetModel( modelpath );
}

void __stdcall CFrame_StartCustomAnimate( CWar3Frame * frame, int anim_id )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->StartAnimate( anim_id );
}

void __stdcall CFrame_StopCustomAnimate( CWar3Frame * frame )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->StopAnimate( );
}

void __stdcall CFrame_SetCustomAnimateOffset( CWar3Frame * frame, float offset )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->anim_offset = offset;
}


// Set texture for frame, now support only SPRITE and some other frametype(USE FRAMETYPE_ITEM FOR TEST)
void __stdcall CFrame_SetFrameTexture( CWar3Frame * frame, const char * texturepath, const char * borderpath, BOOL tiled )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->SetTexture( texturepath, borderpath, tiled );
}

// Set text for frame, support various frame types
void __stdcall CFrame_SetFrameText( CWar3Frame * frame, const char * text )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( string( __func__ + string( ":" ) + text ).c_str( ) );
#endif
	if ( !frame )
		return;
	frame->SetText( text );
}

// Set frame position (Relative main frame)
void __stdcall CFrame_SetAbsolutePosition( CWar3Frame * frame, CFramePosition originpos, float AbsoluteX, float AbsoluteY )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->SetFrameAbsolutePosition( originpos, AbsoluteX, AbsoluteY );
}

// Set frame relative position 
void __stdcall CFrame_SetRelativePosition( CWar3Frame * frame, CFramePosition originpos, int relframeaddr, CFramePosition dstpos, float RelativeX, float RelativeY )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->SetFrameRelativePosition( originpos, relframeaddr, dstpos, RelativeX, RelativeY );
}


// Remove frame from memory and game callback
void __stdcall CFrame_Destroy( CWar3Frame * frame )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->DestroyThisFrame( );

	for ( unsigned int i = 0; i < CFrameBufferList.size( ); i++ )
	{
		if ( CFrameBufferList[ i ].frame == frame )
		{
			CFrameBufferList.erase( CFrameBufferList.begin( ) + i );
			if ( i > 0 )
				i--;
			else
				break;
		}
	}

	delete frame;
}

// Add callback func for selected frame with selected callbackeventid
// skipothercallback can be TRUE or FALSE (possibly useless)
void __stdcall CFrame_AddCallack( CWar3Frame * frame, const char * callbackfuncname, unsigned int callbackeventid, BOOL skipothercallback )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	CFrameBuffer tmpFrameBuf = CFrameBuffer( );
	tmpFrameBuf.frame = frame;
	tmpFrameBuf.callbackfuncname = callbackfuncname;
	tmpFrameBuf.packetcallback = false;
	tmpFrameBuf.callbackeventid = callbackeventid;
	tmpFrameBuf.FrameCode = 0;
	CFrameBufferList.push_back( tmpFrameBuf );
	frame->RegisterEventCallback( callbackeventid );
	//frame->SkipOtherEvents = skipothercallback;
}

void __stdcall CFrame_AddCallackPacket( CWar3Frame * frame, int FrameCode,  unsigned int callbackeventid, BOOL skipothercallback )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	CFrameBuffer tmpFrameBuf = CFrameBuffer( );
	tmpFrameBuf.frame = frame;
	tmpFrameBuf.callbackfuncname = "";
	tmpFrameBuf.packetcallback = true;
	tmpFrameBuf.callbackeventid = callbackeventid;
	tmpFrameBuf.FrameCode = FrameCode;
	CFrameBufferList.push_back( tmpFrameBuf );
	frame->RegisterEventCallback( callbackeventid );
	//frame->SkipOtherEvents = skipothercallback;
}


void __stdcall CFrame_Enable( CWar3Frame * frame, BOOL enabled )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return;
	frame->Enable( enabled );
}

BOOL __stdcall CFrame_IsEnabled( CWar3Frame * frame )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaChatLog( __func__ );
#endif
	if ( !frame )
		return FALSE;
	return frame->IsEnabled( );
}

void __stdcall CFrame_SetScale( CWar3Frame * frame, CFrameBackdropType backtype, BOOL FillToFrame, float scalex, float scaley )
{
	if ( !frame )
		return;
	frame->FillToParentFrame( backtype, FillToFrame );
	frame->SetFrameScale( backtype, scalex, scaley );
}