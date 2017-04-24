#include "Main.h"


GetTownUnitCount_p GetTownUnitCount_org;
GetTownUnitCount_p GetTownUnitCount_ptr;



int __cdecl Wc3MemoryRW( int * addr, int value, BOOL write )
{
#ifdef DOTA_HELPER_LOG

	char readmemdat[ 256 ];
	char modulename[ 256 ];
	HMODULE h = GetModuleFromAddress( ( int )addr );
	if ( !h )
	{
		sprintf_s( readmemdat, "%s memory. Addr: %X", write ? "Write" : "Read", ( unsigned int )addr );
	}
	else
	{
		GetModuleFileNameA( h, modulename, 256 );
		std::string modulefilename = fs::path( modulename ).stem( ).string( );
		sprintf_s( readmemdat, "%s memory. Addr: %s+%X", write ? "Write" : "Read", modulefilename.c_str( ), ( ( unsigned int )addr - ( unsigned int )h ) );
	}
	AddNewLineToJassNativesLog( readmemdat );
#endif


	if ( write )
	{
		*addr = value;
	}
	else
	{
		return *addr;
	}
	return 0;
}

struct ModuleInfoStr
{
	HMODULE addr;
	int size;
};

std::vector<ModuleInfoStr> Modules;

HMODULE GetModuleFromAddress( int addr )
{
	HMODULE hModule = NULL;
#ifdef DOTA_HELPER_LOG

	for ( ModuleInfoStr m : Modules )
	{
		if ( addr >= ( int )m.addr && addr <= ( ( int )m.addr + m.size ) )
			return m.addr;
	}

#endif
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		( LPCTSTR )addr,
		&hModule );

#ifdef DOTA_HELPER_LOG
	if ( hModule )
	{
		MODULEINFO mi;
		GetModuleInformation( GetCurrentProcess( ), hModule, &mi, sizeof( MODULEINFO ) );
		ModuleInfoStr tmp;
		tmp.addr = hModule;
		tmp.size = mi.SizeOfImage;
		Modules.push_back( tmp );
	}
#endif


	return hModule;
}