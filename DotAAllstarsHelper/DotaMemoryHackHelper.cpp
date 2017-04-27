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



int __stdcall ScanJassStringForErrors( BOOL dump )
{

	int strrepvtable = GameDll + 0x954658;

	if ( GameVersion == 0x27a )
	{
		strrepvtable = GameDll + 0x952F30;
	}


	int * firstoffset = ( int * )( StormDll + 0x055370 );

	if ( GameVersion == 0x27a )
	{
		firstoffset = ( int * )( StormDll + 0x056F80 );
	}

	int memsize = 0x100;
	int i = 0;
	int stringcount = 0;
	FILE * f;

	if ( dump )
		fopen_s( &f, "dumpbadstr.txt", "w" );

	for ( i = 0; i < memsize; i++ )
	{
		int currentmemoffset = firstoffset[ i ];
		while ( currentmemoffset > 0 )
		{
			int currentstrrepoffset = currentmemoffset + 0x8C;


			if ( *( int* )( currentstrrepoffset ) == strrepvtable )
			{

				StringRep * curstr = ( StringRep * )currentstrrepoffset;

				while ( ( int )curstr->prev > 0 && ( int )curstr->prev->vtable == strrepvtable )
				{
					curstr = curstr->prev;
				}


				while ( ( int )curstr->vtable == strrepvtable && ( int )curstr->next > 0 )
				{
					uint32_t strhash = Ordinal590_org( ( unsigned char * )curstr->text );
					if ( strhash != curstr->hash )
					{
						if ( dump )
							fprintf_s( f, "Found bad string:%s\n", curstr->text );
						stringcount++;
					}
					curstr = curstr->next;
				}

			}
			currentmemoffset = *( int* )currentmemoffset;
		}
	}

	if ( dump )
		fclose( f );


	return stringcount;
}


int __stdcall GetJassStringCount( BOOL dump )
{

	int strrepvtable = GameDll + 0x954658;

	if ( GameVersion == 0x27a )
	{
		strrepvtable = GameDll + 0x952F30;
	}


	int * firstoffset = ( int * )( StormDll + 0x055370 );

	if ( GameVersion == 0x27a )
	{
		firstoffset = ( int * )( StormDll + 0x056F80 );
	}



	int memsize = 0x100;
	int i = 0;
	int stringcount = 0;
	FILE * f;

	if ( dump )
		fopen_s( &f, "dumpallstr.txt", "w" );

	for ( i = 0; i < memsize; i++ )
	{
		int currentmemoffset = firstoffset[ i ];
		while ( currentmemoffset > 0 )
		{
			int currentstrrepoffset = currentmemoffset + 0x8C;

			if ( *( int* )( currentstrrepoffset ) == strrepvtable )
			{

				StringRep * curstr = ( StringRep * )currentstrrepoffset;

				while ( ( int )curstr->prev > 0 && ( int )curstr->prev->vtable == strrepvtable )
				{
					curstr = curstr->prev;
				}



				while ( ( int )curstr->vtable == strrepvtable && ( int )curstr->next > 0 )
				{
					if ( dump )
						fprintf_s( f, "%s\n", curstr->text );
					stringcount++;
					curstr = curstr->next;
				}


			}
			currentmemoffset = *( int* )currentmemoffset;
		}
	}

	if ( dump )
		fclose( f );


	return stringcount;
}

