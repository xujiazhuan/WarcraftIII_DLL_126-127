#include "Main.h"


int PacketClassPtr;
int pGAME_SendPacket;
struct Packet
{
	int PacketClassPtr;
	BYTE* PacketData;
	DWORD _1;				//+08, zero
	DWORD _2;				//+0C, ??
	DWORD Size;
	DWORD _3;				//+14, 0xFFFFFFFF
};


typedef void *( __fastcall * GAME_SendPacket_p ) ( Packet* packet, DWORD );
GAME_SendPacket_p GAME_SendPacket = NULL;


void SendPacket( BYTE* packetData, DWORD size )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
	// @warning: this function thread-unsafe, do not use it in other thread.
	// note: this is very useful function, in fact this function
	// does wc3 ingame action, so you can use it for anything you want,
	// including unit commands and and gameplay commands,
	// i suppose its wc3 single action W3GS_INCOMING_ACTION (c) wc3noobpl.

	Packet packet;
	memset( &packet, 0, sizeof( Packet ) );
	packet.PacketClassPtr = PacketClassPtr; // Packet Class
	packet.PacketData = packetData;
	packet.Size = size;
	packet._3 = 0xFFFFFFFF;
	if ( !GAME_SendPacket )
		GAME_SendPacket = ( GAME_SendPacket_p )( pGAME_SendPacket );
	GAME_SendPacket( &packet, 0 );
}
