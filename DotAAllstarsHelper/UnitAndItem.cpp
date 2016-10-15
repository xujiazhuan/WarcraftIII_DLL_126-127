#include "Main.h"


pGetHeroInt GetHeroInt;
// Получить владельца юнита
int __stdcall GetUnitOwnerSlot( int unitaddr )
{
	if ( unitaddr )
		return *( int* ) ( unitaddr + 88 );
	return 0;
}



// Является ли юнит героем
BOOL __stdcall IsHero( int unitaddr )
{
	if ( unitaddr )
	{
		UINT ishero = *( UINT* ) ( unitaddr + 48 );
		ishero = ishero >> 24;
		ishero = ishero - 64;
		return ishero < 0x19;
	}
	return FALSE;
}


// Является ли юнит зданием
BOOL __stdcall IsTower( int unitaddr )
{
	if ( unitaddr )
	{
		UINT istower = *( UINT* ) ( unitaddr + 0x5C );
		return ( istower & 0x10000 ) > 0;
	}
	return FALSE;
}


// Проверяет юнит или не юнит
BOOL __stdcall IsNotBadUnit( int unitaddr )
{
	if ( unitaddr > 0 )
	{
		int xaddraddr = ( int ) &UnitVtable;

		if ( *( BYTE* ) xaddraddr != *( BYTE* ) unitaddr )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 1 ) != *( BYTE* ) ( unitaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 2 ) != *( BYTE* ) ( unitaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 3 ) != *( BYTE* ) ( unitaddr + 3 ) )
			return FALSE;

		unsigned int unitflag = *( unsigned int* ) ( unitaddr + 0x20 );
		unsigned int unitflag2 = *( unsigned int* ) ( unitaddr + 0x5C );

		if ( unitflag & 1u )
			return FALSE;

		if ( !( unitflag & 2u ) )
			return FALSE;

		if ( unitflag2 & 0x100u )
			return FALSE;

		return TRUE;
	}

	return FALSE;
}


// Проверяет враг юнит локальному игроку или нет
BOOL __stdcall IsEnemy( int UnitAddr )
{
	if ( !UnitAddr )
		return TRUE;

	int unitownerslot = GetUnitOwnerSlot( ( int ) UnitAddr );
	if ( unitownerslot <= 15 && unitownerslot >= 0 )
	{
		UINT Player1 = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( unitownerslot );
		UINT Player2 = ( ( GetPlayerByID ) ( GameDll + GetPlayerByIDOffset ) )( GetLocalPlayerId( ) );
		return ( ( ( IsPlayerEnemy ) ( GameDll + IsPlayerEnemyOffset ) )( Player1, Player2 ) );
	}
	return TRUE;
}





// Проверяет предмет или не предмет
BOOL __stdcall IsNotBadItem( int itemaddr )
{
	if ( itemaddr > 0 )
	{
		int xaddraddr = ( int ) &ItemVtable;

		if ( *( BYTE* ) xaddraddr != *( BYTE* ) itemaddr )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 1 ) != *( BYTE* ) ( itemaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 2 ) != *( BYTE* ) ( itemaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* ) ( xaddraddr + 3 ) != *( BYTE* ) ( itemaddr + 3 ) )
			return FALSE;

		return TRUE;
	}

	return FALSE;
}

