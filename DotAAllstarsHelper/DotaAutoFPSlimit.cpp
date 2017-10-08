#include "Main.h"


unsigned int( __thiscall  * DestroyUnitHpBar )( int HpBarAddr );


BOOL FPS_LIMIT_ENABLED = FALSE;

ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
int numProcessors;
unsigned int CPU_cores;
HANDLE Warcraft3_Process = NULL;
p_SetMaxFps _SetMaxFps = NULL;

int __stdcall SetMaxFps( int fps )
{
	_SetMaxFps( fps );
	return fps;
}

void InitThreadCpuUsage( ) {
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;


	numProcessors = std::thread::hardware_concurrency( );
	if ( numProcessors == 0 )
	{
		GetSystemInfo( &sysInfo );
		numProcessors = sysInfo.dwNumberOfProcessors;
	}

	GetSystemTimeAsFileTime( &ftime );
	memcpy( &lastCPU, &ftime, sizeof( FILETIME ) );


	GetProcessTimes( Warcraft3_Process, &ftime, &ftime, &fsys, &fuser );
	memcpy( &lastSysCPU, &fsys, sizeof( FILETIME ) );
	memcpy( &lastUserCPU, &fuser, sizeof( FILETIME ) );
}

double GetWar3CpuUsage( ) {
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;
	GetSystemTimeAsFileTime( &ftime );
	memcpy( &now, &ftime, sizeof( FILETIME ) );
	GetProcessTimes( Warcraft3_Process, &ftime, &ftime, &fsys, &fuser );
	memcpy( &sys, &fsys, sizeof( FILETIME ) );
	memcpy( &user, &fuser, sizeof( FILETIME ) );
	percent = ( double )( ( sys.QuadPart - lastSysCPU.QuadPart ) +
		( user.QuadPart - lastUserCPU.QuadPart ) );
	percent /= ( now.QuadPart - lastCPU.QuadPart );
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;
	return percent * 100.0;
}


#define MAX_WAR3_FPS 80
#define MIN_WAR3_FPS 24
int CurrentFps = 64;
void UpdateFPS( )
{
	double currentcpuusage = GetWar3CpuUsage( );
	if ( currentcpuusage > 88.0 )
	{
		if ( CurrentFps > MIN_WAR3_FPS )
		{
			CurrentFps -= 4;
			if ( *InGame )
				_SetMaxFps( CurrentFps );
			sprintf_s( MyFpsString, 512, "%s%i.0 CPU:%.1f", "|nFPS: %.1f/", CurrentFps, currentcpuusage );
		}
	}
	else if ( currentcpuusage < 70.0 )
	{
		if ( CurrentFps < MAX_WAR3_FPS )
		{
			CurrentFps += 4;
			if ( *InGame )
				_SetMaxFps( CurrentFps );
			sprintf_s( MyFpsString, 512, "%s%i.0 CPU:%.1f", "|nFPS: %.1f/", CurrentFps, currentcpuusage );
		}
	}
}



int __stdcall EnableAutoFPSlimit( BOOL enable )
{
	FPS_LIMIT_ENABLED = enable;
	if ( !FPS_LIMIT_ENABLED )
		_SetMaxFps( 200 );
	return enable;
}


//map<int, BOOL> NeedDrawBarForUnit;

BOOL __stdcall UnitNeedDrawBar( int unitaddr )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__,__LINE__ );
#endif
	if ( unitaddr && IsNotBadUnit( unitaddr ) )
	{
		if ( GetUnitOwnerSlot( unitaddr ) >= 15 && IsUnitInvulnerable( unitaddr ) )
		{
			//if ( NeedDrawBarForUnit[ unitaddr ] )
			return FALSE;
			/*else
				NeedDrawBarForUnit[ unitaddr ] = TRUE;*/
		}
		//else
		//	NeedDrawBarForUnit[ unitaddr ] = FALSE;

		return TRUE;
	}
	return FALSE;
}

pDrawBarForUnit DrawBarForUnit_org;
pDrawBarForUnit DrawBarForUnit_ptr;

BOOL FPSfix1Enabled = FALSE;

void __fastcall DrawBarForUnit_my( int unitaddr )
{
	if ( !FPSfix1Enabled )
	{
		DrawBarForUnit_ptr( unitaddr );
	}
	else /*if ( UnitNeedDrawBar( unitaddr ) )*/
	{
		BOOL needremove = FALSE;
		int hpbaraddr = *( int* )( unitaddr + 0x50 );
		if ( hpbaraddr)
		{
			if ( !IsNotBadUnit( unitaddr ) || ( IsTower( unitaddr ) && IsUnitInvulnerable( unitaddr ) ) )
			{
				needremove = TRUE;
				*( int * )( hpbaraddr + 8 ) = 0;
			}
		}

		DrawBarForUnit_ptr( unitaddr );

		if ( needremove )
		{
			*( int* )( unitaddr + 0x50 ) = 0;
		}
	}
}

int __stdcall EnableFPSfix1( BOOL enable )
{
	FPSfix1Enabled = enable;
	return 0;
}