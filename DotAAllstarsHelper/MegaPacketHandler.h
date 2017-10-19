#pragma once


extern int PacketClassPtr;
extern int pGAME_SendPacket;
void SendPacket( unsigned char * packetData, unsigned int  size );


void UninitializePacketHandler(   );
void InitializePacketHandler( int GameVersion );