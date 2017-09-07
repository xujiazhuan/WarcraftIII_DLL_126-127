#include "Main.h"
#include "Storm.h"


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
		fopen_s( &f, "dumpallbadstr.txt", "w" );


	std::vector<int> foundoffsets;

	for ( i = 0; i < memsize; i++ )
	{
		int currentmemoffset = firstoffset[ i ];
		while ( currentmemoffset > 0 )
		{
			int currentstrrepoffset = currentmemoffset + 0x8C;
			if ( std::find( foundoffsets.begin( ), foundoffsets.end( ), currentstrrepoffset ) == foundoffsets.end( ) )
			{
				foundoffsets.push_back( currentstrrepoffset );
				const char * currentnameoffset = ( const char * )( currentmemoffset + 0x70 );


				if ( *( int* )( currentstrrepoffset ) == strrepvtable )
				{

					StringRep * curstr = ( StringRep * )currentstrrepoffset;

					while ( ( int )curstr->prev > 0 && ( int )curstr->prev->vtable == strrepvtable )
					{
						curstr = curstr->prev;
					}



					while ( ( int )curstr->vtable == strrepvtable && ( int )curstr->next > 0 )
					{
						uint32_t strhash = Storm::StringGetHash( curstr->text );
						if ( strhash != curstr->hash )
						{
							if ( dump && f )
								fprintf_s( f, "Found bad string[#1-%X]:%s\n", (unsigned int )curstr,curstr->text );
							stringcount++;
						}
					
						curstr = curstr->next;
					}


				}
				else if ( memcmp( currentnameoffset, "String2H", 8 ) == 0 )
				{
					int curstr2hoffset = currentmemoffset + 0x94;
					String2H * curstr = ( String2H * )curstr2hoffset;
					String2H * firststr = curstr;

					while ( ( int )curstr > 0 )
					{

						uint32_t strhash = Storm::StringGetHash( curstr->text );
						if ( strhash != curstr->hash )
						{
							if ( dump && f )
								fprintf_s( f, "Found bad string[#2-%X]:%s\n", ( unsigned int )curstr, curstr->text );
							stringcount++;
						}

						if ( ( int )curstr->next <= 0 )
							break;
						curstr = curstr->next;
						if ( firststr == curstr )
							break;

					}


				}
			}
			currentmemoffset = *( int* )currentmemoffset;
		}
	}

	if ( dump )
		fclose( f );

	foundoffsets.clear( );

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


	std::vector<int> foundoffsets;

	for ( i = 0; i < memsize; i++ )
	{
		int currentmemoffset = firstoffset[ i ];
		while ( currentmemoffset > 0 )
		{
			int currentstrrepoffset = currentmemoffset + 0x8C;
			const char * currentnameoffset = ( const char * )( currentmemoffset + 0x70 );

			if ( std::find( foundoffsets.begin( ), foundoffsets.end( ), currentstrrepoffset ) == foundoffsets.end( ) )
			{
				foundoffsets.push_back( currentstrrepoffset );

				if ( *( int* )( currentstrrepoffset ) == strrepvtable )
				{

					StringRep * curstr = ( StringRep * )currentstrrepoffset;

					while ( ( int )curstr->prev > 0 && ( int )curstr->prev->vtable == strrepvtable )
					{
						curstr = curstr->prev;
					}



					while ( ( int )curstr->vtable == strrepvtable && ( int )curstr->next > 0 )
					{
						if ( dump && f )
							fprintf_s( f, "%s\n", curstr->text );
						stringcount++;
						curstr = curstr->next;
					}


				}
				else if ( memcmp( currentnameoffset, "String2H", 8 ) == 0 )
				{
					int curstr2hoffset = currentmemoffset + 0x94;
					String2H * curstr = ( String2H * )curstr2hoffset;
					String2H * firststr = curstr;

					while ( ( int )curstr > 0 )
					{

						if ( dump && f )
							fprintf_s( f, "%s\n", curstr->text );
						stringcount++;


						if ( ( int )curstr->next <= 0 )
							break;
						curstr = curstr->next;
						if ( firststr == curstr )
							break;

					}


				}
			}
			currentmemoffset = *( int* )currentmemoffset;
		}
	}

	if ( dump )
		fclose( f );

	foundoffsets.clear( );

	return stringcount;
}

