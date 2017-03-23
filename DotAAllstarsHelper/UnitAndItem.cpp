#include "Main.h"


pGetHeroInt GetHeroInt;
// Получить владельца юнита
int __stdcall GetUnitOwnerSlot( int unitaddr )
{
	if ( unitaddr > 0 )
		return *( int* )( unitaddr + 88 );
	return 15;
}



// Является ли юнит героем
BOOL __stdcall IsHero( int unitaddr )
{
	if ( unitaddr > 0 )
	{
		unsigned int ishero = *( unsigned int* )( unitaddr + 48 );
		ishero = ishero >> 24;
		ishero = ishero - 64;
		return ishero < 0x19;
	}
	return FALSE;
}


// Является ли юнит зданием
BOOL __stdcall IsTower( int unitaddr )
{
	if ( unitaddr > 0 )
	{
		unsigned int istower = *( unsigned int* )( unitaddr + 0x5C );
		return ( istower & 0x10000 ) > 0;
	}
	return FALSE;
}


BOOL __stdcall IsUnitInvulnerable( int unitaddr )
{
	return ( *( unsigned int* )( unitaddr + 0x20 ) & 8 ) ;
}

// Проверяет юнит или не юнит
BOOL __stdcall IsNotBadUnit( int unitaddr )
{
	if ( unitaddr > 0 )
	{
		int xaddraddr = ( int )&UnitVtable;

		if ( *( BYTE* )xaddraddr != *( BYTE* )unitaddr )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 1 ) != *( BYTE* )( unitaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 2 ) != *( BYTE* )( unitaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 3 ) != *( BYTE* )( unitaddr + 3 ) )
			return FALSE;

		unsigned int unitflag = *( unsigned int* )( unitaddr + 0x20 );
		unsigned int unitflag2 = *( unsigned int* )( unitaddr + 0x5C );

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
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "IsEnemy:Start" );
#endif
	if ( UnitAddr > 0 && IsNotBadUnit( UnitAddr ) )
	{
		int unitownerslot = GetUnitOwnerSlot( ( int )UnitAddr );

		if ( GetLocalPlayerId( ) == unitownerslot )
		{
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( "IsEnemy:SamePlayer1" );
#endif
			return FALSE;
		}

		if ( unitownerslot <= 15 && unitownerslot >= 0 )
		{
			unsigned int Player1 = Player( unitownerslot );
			unsigned int Player2 = Player( GetLocalPlayerId( ) );

			if ( Player1 == Player2 )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( "IsEnemy:SamePlayer2" );
#endif
				return FALSE;
			}
			if ( Player1 == 0 || Player2 == 0 )
			{
#ifdef DOTA_HELPER_LOG
				AddNewLineToDotaHelperLog( "IsEnemy:BadPlayer" );
#endif
				return FALSE;
			}

			BOOL retval = ( ( ( IsPlayerEnemy )( GameDll + IsPlayerEnemyOffset ) )( Player1, Player2 ) );
#ifdef DOTA_HELPER_LOG
			AddNewLineToDotaHelperLog( "IsEnemy:Okay" );
#endif
			return retval;
		}
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "IsEnemy:BadUnit" );
#endif
	return FALSE;
}


// Проверяет предмет или не предмет
BOOL __stdcall IsNotBadItem( int itemaddr )
{
	if ( itemaddr > 0 )
	{
		int xaddraddr = ( int )&ItemVtable;

		if ( *( BYTE* )xaddraddr != *( BYTE* )itemaddr )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 1 ) != *( BYTE* )( itemaddr + 1 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 2 ) != *( BYTE* )( itemaddr + 2 ) )
			return FALSE;
		else if ( *( BYTE* )( xaddraddr + 3 ) != *( BYTE* )( itemaddr + 3 ) )
			return FALSE;

		return TRUE;
	}

	return FALSE;
}





int GetSelectedUnitCountBigger( int slot )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "GetSelectedUnitCountBigger" );
#endif
	int plr = GetPlayerByNumber( slot );
	if ( plr > 0 )
	{
		int PlayerData1 = *( int* )( plr + 0x34 );
		if ( PlayerData1 > 0 )
		{
			int unitcount = *( int * )( PlayerData1 + 0x10 );
			int unitcount2 = *( int * )( PlayerData1 + 0x1D4 );

			if ( unitcount > unitcount2 )
				return unitcount;
			else
				return unitcount2;
		}
	}

	return NULL;
}


int GetSelectedUnit( int slot )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( "GetSelectedUnit" );
#endif
	int plr = GetPlayerByNumber( slot );
	if ( plr > 0 )
	{
		int PlayerData1 = *( int* )( plr + 0x34 );
		if ( PlayerData1 > 0 )
		{
			return *( int * )( PlayerData1 + 0x1e0 );
		}
	}

	return NULL;
}





int ReadObjectAddrFromGlobalMat( unsigned int a1, unsigned int a2 )
{
	BOOL found1;
	int result;
	int AddrType1;
	int v5;

	if ( !( a1 >> 31 ) )
	{
		if ( a1 < *( unsigned int * )( *( int* )pGameClass1 + 28 ) )
		{
			found1 = *( int * )( *( int * )( *( int* )pGameClass1 + 12 ) + 8 * a1 ) == -2;
			if ( !found1 )
				return 0;
			if ( a1 >> 31 )
			{
				AddrType1 = *( int * )( *( int * )( *( int* )pGameClass1 + 44 ) + 8 * a1 + 4 );
				result = *( unsigned int * )( AddrType1 + 24 ) != a2 ? 0 : AddrType1;
			}
			else
			{
				v5 = *( int * )( *( int * )( *( int* )pGameClass1 + 12 ) + 8 * a1 + 4 );
				result = *( unsigned int * )( v5 + 24 ) != a2 ? 0 : v5;
			}
			return result;
		}
		return 0;
	}
	if ( ( a1 & 0x7FFFFFFF ) >= *( unsigned int * )( *( int* )pGameClass1 + 60 ) )
		return 0;
	found1 = *( int * )( *( int * )( *( int* )pGameClass1 + 44 ) + 8 * a1 ) == -2;
	if ( !found1 )
		return 0;
	if ( a1 >> 31 )
	{
		AddrType1 = *( int * )( *( int * )( *( int* )pGameClass1 + 44 ) + 8 * a1 + 4 );
		result = *( unsigned int * )( AddrType1 + 24 ) != a2 ? 0 : AddrType1;
	}
	else
	{
		v5 = *( int * )( *( int * )( *( int* )pGameClass1 + 12 ) + 8 * a1 + 4 );
		result = *( unsigned int * )( v5 + 24 ) != a2 ? 0 : v5;
	}
	return result;
}


int GetObjectDataAddr( int addr )
{
	int mataddr;
	int result; // eax@3

	mataddr = ReadObjectAddrFromGlobalMat( *( unsigned int * )addr, *( unsigned int * )( addr + 4 ) );

	if ( !mataddr || *( int * )( mataddr + 32 ) )
		result = 0;
	else
		result = *( int * )( mataddr + 84 );
	return result;
}


vector<int> ReturnAbils;

int * FindUnitAbils( int unitaddr, unsigned int * count, int abilcode, int abilbasecode )
{
	if ( !ReturnAbils.empty( ) )
		ReturnAbils.clear( );
	*count = 0;
	if ( unitaddr > 0 )
	{
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__ );
#endif
		int pAddr1 = unitaddr + 0x1DC;
		int pAddr2 = unitaddr + 0x1E0;

		if ( ( int )( *( unsigned int * )( pAddr1 ) & *( unsigned int * )( pAddr2 ) ) != -1 )
		{
			int pData = GetObjectDataAddr( pAddr1 );

			while ( pData > 0 )
			{
				int pData2 = *( int* )( pData + 0x54 );
				if ( pData2 > 0 )
				{
					if ( abilcode != 0 && *( int* )( pData2 + 0x34 ) == abilcode )
					{
						if ( abilbasecode != 0 && *( int* )( pData2 + 0x30 ) == abilbasecode )
						{
							ReturnAbils.push_back( pData );
						}
						else if ( abilbasecode == 0 )
						{
							ReturnAbils.push_back( pData );
						}
					}
					else if ( abilcode == 0 )
					{
						if ( abilbasecode != 0 && *( int* )( pData2 + 0x30 ) == abilbasecode )
						{
							ReturnAbils.push_back( pData );
						}
						else if ( abilbasecode == 0 )
						{
							ReturnAbils.push_back( pData );
						}
					}
				}
				pData = GetObjectDataAddr( pData + 0x24 );
			}

			*count = ReturnAbils.size( );
		}
	}

	return &ReturnAbils[ 0 ];
}
