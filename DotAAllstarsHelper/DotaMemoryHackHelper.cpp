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

Ordinal590_p Ordinal590_org;

int __stdcall ScanJassStringForErrors( StringRep * firstStr )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	StringRep * currentStr = firstStr;
	int stringfound = 0;
	int maxstr = 500000;

	FILE * f;
	fopen_s( &f, "dumpbadstrings.txt", "w" );

	while ( (int)currentStr > 0 && firstStr != currentStr->next )
	{

		uint32_t strhash = Ordinal590_org( ( unsigned char * )currentStr->text );
		if ( strhash != currentStr->hash )
		{
			fprintf_s( f, "Found bad string:%s", currentStr->text );
			stringfound++;
		}

		currentStr = currentStr->next;
		if ( maxstr-- < 0 )
			break;
	}
	fclose( f );

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	return stringfound;
}




int __stdcall GetJassStringCount( StringRep * firstStr )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	StringRep * currentStr = firstStr;
	int stringfound = 0;
	int maxstr = 500000;

	while ( (int)currentStr > 0 && firstStr != currentStr->next )
	{
		stringfound++;
		currentStr = currentStr->next;
		if ( maxstr-- < 0 )
			break;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	return stringfound;
}