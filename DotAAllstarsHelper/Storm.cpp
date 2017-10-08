#include "Main.h"
#include "Storm.h"
#include "FunctionTemplate.h"
#include <new>
#include <cstdlib>
#include <vector>

namespace Storm {

	struct StormLeakHelper
	{
		void * mem;
		int retaddr;
	};

	vector<StormLeakHelper> LeakHelperList;

	int AllocationCount = 0;
	bool StormAvailable = false;
	HMODULE StormModule = NULL;
	PROTOTYPE_FileArchiveOpen FileOpenArchive = NULL;
	PROTOTYPE_FileArchiveClose FileCloseArchive = NULL;
	PROTOTYPE_StringGetHash StringGetHash = NULL;
	void* AddrMemAlloc;
	void* AddrMemFree;
	void* AddrMemGetSize;
	void* AddrMemReAlloc;
	void* AddrFileOpenFileEx;
	void* AddrFileGetFileSize;
	void* AddrFileReadFile;
	void* AddrFileCloseFile;
	void* AddrFileGetLocale;

	void Init( HMODULE module ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		if ( !module )
			return;

		StormModule = module;
		FileOpenArchive = ( PROTOTYPE_FileArchiveOpen )GetProcAddress( module, ( LPCSTR )266 );
		FileCloseArchive = ( PROTOTYPE_FileArchiveClose )GetProcAddress( module, ( LPCSTR )252 );
		StringGetHash = ( PROTOTYPE_StringGetHash )GetProcAddress( module, ( LPCSTR )590 );
		AddrMemAlloc = ( void* )GetProcAddress( module, ( LPCSTR )401 );
		AddrMemFree = ( void* )GetProcAddress( module, ( LPCSTR )403 );
		AddrMemGetSize = ( void* )GetProcAddress( module, ( LPCSTR )404 );
		AddrMemReAlloc = ( void* )GetProcAddress( module, ( LPCSTR )405 );
		AddrFileOpenFileEx = ( void* )GetProcAddress( module, ( LPCSTR )268 );
		AddrFileGetFileSize = ( void* )GetProcAddress( module, ( LPCSTR )265 );
		AddrFileReadFile = ( void* )GetProcAddress( module, ( LPCSTR )269 );
		AddrFileCloseFile = ( void* )GetProcAddress( module, ( LPCSTR )253 );
		AddrFileGetLocale = ( void* )GetProcAddress( module, ( LPCSTR )294 );
		StormAvailable = true;
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	}

	void Cleanup( ) {

	}

	
	//294	SFileGetLocale() 
	LANGID FileGetLocale( ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );


		if ( !StormAvailable )
			MessageBoxA( 0, "Storm not initialized", "Error1", 0 );
#endif

		return aero::generic_c_call<uint16_t>( AddrFileGetLocale );
	}

	//TODO Debug版本加上调试信息
	void* MemAlloc( uint32_t size ) {



#ifdef DOTA_HELPER_LOG
		auto retaddr = _ReturnAddress( );
		StormLeakHelper tmpleak = StormLeakHelper( );
		tmpleak.retaddr = ( int )retaddr;

		if ( !StormAvailable )
			MessageBoxA( 0, "Storm not initialized", "Error1", 0 );
#endif
		void * retval = aero::generic_std_call<void*>(
			AddrMemAlloc,
			size,
			"",
			0,
			0
			);
#ifdef DOTA_HELPER_LOG
		tmpleak.mem = retaddr;
		LeakHelperList.push_back( tmpleak );
#endif
		return retval;
	}

	void ShowAllLeaks( )
	{
		std::string outleaks;
		char tmps[ 50 ];
	
		for ( auto s : LeakHelperList )
		{
			sprintf_s( tmps, "%X:%X\r\n", s.retaddr, s.mem );
			outleaks = outleaks + tmps;
		}
		MessageBoxA( 0, outleaks.c_str( ), "Leaks:", 0 );
	}

	void* MemFree( void* addr ) {
		AllocationCount--;
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
		for ( unsigned int  i = 0; i < LeakHelperList.size( ); i++ )
		{
			if ( LeakHelperList[ i ].mem == addr )
			{
				LeakHelperList.erase( LeakHelperList.begin( ) + i );
				break;
			}
		}
	
#endif
		
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( (__func__ + (string)". Addr:" + to_string((int)addr)).c_str( ), __LINE__ );


		if ( !StormAvailable )
			MessageBoxA( 0, "Storm not initialized", "Error1", 0 );
#endif

		if ( !addr )
			return 0;

		return aero::generic_std_call<void*>(
			AddrMemFree,
			addr,
			"",
			0,
			0
			);
	}

	int MemGetSize( void* addr ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );


		if ( !StormAvailable )
			MessageBoxA( 0, "Storm not initialized", "Error1", 0 );
#endif
		return aero::generic_std_call<uint32_t>(
			AddrMemGetSize,
			addr,
			"",
			0
			);
	}

	void* MemReAlloc( void* addr, uint32_t size ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );


		if ( !StormAvailable )
			MessageBoxA( 0, "Storm not initialized", "Error1", 0 );
#endif
		return aero::generic_std_call<void*>(
			AddrMemReAlloc,
			addr,
			size,
			"",
			0,
			0
			);
	}

	HANDLE FileOpenFileEx( HANDLE hMpq, const char *szFileName, uint32_t dwSearchScope, HANDLE *phFile ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );


		if ( !StormAvailable )
			MessageBoxA( 0, "Storm not initialized", "Error1", 0 );
#endif
		return aero::generic_std_call<HANDLE>(
			AddrFileOpenFileEx,
			hMpq,
			szFileName,
			dwSearchScope,
			phFile
			);
	}

	uint32_t FileGetFileSize( HANDLE hFile, uint32_t *pdwFileSizeHigh ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );

		if ( !StormAvailable )
			MessageBoxA( 0, "Storm not initialized", "Error1", 0 );
#endif
		return aero::generic_std_call<uint32_t>(
			AddrFileGetFileSize,
			hFile,
			pdwFileSizeHigh
			);
	}

	bool FileReadFile( HANDLE hFile, void* lpBuffer, uint32_t dwToRead, uint32_t* pdwRead, LPOVERLAPPED lpOverlapped ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return aero::generic_std_call<bool>(
			AddrFileReadFile,
			hFile,
			lpBuffer,
			dwToRead,
			pdwRead,
			lpOverlapped
			);
	}

	bool FileCloseFile( HANDLE hFile ) {
#ifdef DOTA_HELPER_LOG
		AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
		return aero::generic_std_call<bool>(
			AddrFileCloseFile,
			hFile
			);
	}

}

/*
#ifndef _DEBUG
struct MemInfo {
	const char* FILE;
	int LINE;
};

std::map<void*, MemInfo> MemInfoMap;

void *operator new(size_t size) {
	void* rv;

	if (!Storm::StormAvailable) {
		rv = malloc(size);
		//OutputDebug("NonStormAlloc: 0x%X (%u)", rv, size);

		return rv;
	}

	if (size == 0)
		size = 1;

	while (1) {
		if ((rv = Storm::MemAlloc(size)) != NULL) {
			return rv;
		}

		// 分配不成功，找出当前出错处理函数
		new_handler globalhandler = std::set_new_handler(0);
		std::set_new_handler(globalhandler);

		if (globalhandler)
			(*globalhandler)();
		else throw std::bad_alloc();
	}
}

void operator delete(void *rawmemory) {
	if (rawmemory == 0)
		return;

	if (!Storm::StormAvailable) {
		free(rawmemory);
		//OutputDebug("NonStormFree: 0x%X", rawmemory);
		return;
	}

	Storm::MemFree(rawmemory);
	return;
}
#endif
*/