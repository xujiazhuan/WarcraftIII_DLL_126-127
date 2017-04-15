#include "Main.h"
#include "ViewAllySkill.h"


IsNeedDrawUnit2 IsNeedDrawUnit2org;
IsNeedDrawUnit2 IsNeedDrawUnit2ptr;

IsDrawSkillPanel IsDrawSkillPanel_org = NULL;
IsDrawSkillPanel IsDrawSkillPanel_ptr;
IsDrawSkillPanelOverlay IsDrawSkillPanelOverlay_org = NULL;
IsDrawSkillPanelOverlay IsDrawSkillPanelOverlay_ptr;

BOOL ShowSkillPanelForObservers = FALSE;





signed int __fastcall  IsDrawSkillPanel_my( void *UnitAddr, int addr1 )
{
	signed int result;
	int GETOID;
	int OID;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	if ( addr1 )
	{
		GETOID = *( int * )( addr1 + 444 );
		if ( GETOID > 0 )
			OID = *( int * )( GETOID + 8 );
		else
			OID = 852290;
		// Сначала вызвать оригинальную функцию
		if ( ( ( IsNeedDrawUnitOrigin )( GameDll + IsNeedDrawUnitOriginOffset ) )( UnitAddr ) )
		{
			( ( DrawSkillPanel )( GameDll + DrawSkillPanelOffset ) )( UnitAddr, OID );
		}
		else if ( IsNotBadUnit( ( int )UnitAddr ) )
		{
			// Затем дополнительную которая отрисует скилы всем союзным героям.
			if ( IsEnemy( ( int )UnitAddr ) == FALSE )
			{
				if ( IsHero( ( int )UnitAddr ) )
					( ( DrawSkillPanel )( GameDll + DrawSkillPanelOffset ) )( UnitAddr, OID );
			}
			else if ( ShowSkillPanelForObservers && IsLocalPlayerObserver( ) )
			{
				( ( DrawSkillPanel )( GameDll + DrawSkillPanelOffset ) )( UnitAddr, OID );
			}
		}
		result = 1;
	}
	else
	{
		result = 0;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ + string( "2" ) );
#endif
	return result;
}

signed int __fastcall  IsDrawSkillPanelOverlay_my( void *UnitAddr, int addr1 )
{
	signed int result; // eax@2
	int GETOID; // eax@3
	int OID; // esi@4
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	if ( addr1 )
	{
		GETOID = *( int * )( addr1 + 444 );
		if ( GETOID > 0 )
			OID = *( int * )( GETOID + 8 );
		else
			OID = 852290;
		// Сначала вызвать оригинальную функцию
		if ( ( ( IsNeedDrawUnitOrigin )( GameDll + IsNeedDrawUnitOriginOffset ) )( UnitAddr ) )
		{
			( ( DrawSkillPanelOverlay )( GameDll + DrawSkillPanelOverlayOffset ) )( UnitAddr, OID );
		}
		else if ( IsNotBadUnit( ( int )UnitAddr ) )
		{
			// Затем дополнительную которая отрисует скилы всем союзным героям.
			if ( IsEnemy( ( int )UnitAddr ) == FALSE )
			{
				if ( IsHero( ( int )UnitAddr ) )
					( ( DrawSkillPanelOverlay )( GameDll + DrawSkillPanelOverlayOffset ) )( UnitAddr, OID );
			}
			else if ( ShowSkillPanelForObservers && IsLocalPlayerObserver( ) )
			{
				( ( DrawSkillPanelOverlay )( GameDll + DrawSkillPanelOverlayOffset ) )( UnitAddr, OID );
			}
		}
		result = 1;
	}
	else
	{
		result = 0;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ + string( "2" ) );
#endif
	return result;
}

int __fastcall IsNeedDrawUnit2_my( int UnitAddr, int unused/* converted from thiscall to fastcall*/ )
{
	/*int retaddr = ( int ) _ReturnAddress( ) - GameDll;


	if ( retaddr + 2000 < IsNeedDrawUnit2offsetRetAddress && retaddr > IsNeedDrawUnit2offsetRetAddress )
	{*/
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__ );
#endif
	if ( IsEnemy( UnitAddr ) == FALSE )
	{
		if ( IsHero( ( int )UnitAddr ) )
			return 1;
	}

	if ( ShowSkillPanelForObservers && IsLocalPlayerObserver( ) )
	{
		return 1;
	}

	//}

	return IsNeedDrawUnit2ptr( UnitAddr );
}