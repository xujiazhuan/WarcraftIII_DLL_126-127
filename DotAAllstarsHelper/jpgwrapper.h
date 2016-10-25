#ifndef JpgWrapper
#define JpgWrapper
#pragma warning(disable:4668)
#pragma warning(disable:4820)
#pragma warning(disable:4710)
#pragma warning(disable:4100)
#pragma warning(disable:4514)

#include <cstdio>
#include <cstring>
#include <csetjmp>

#pragma comment(lib,"legacy_stdio_definitions.lib")
#pragma comment(lib,"turbojpeg-static.lib")
#include <jpeglib.h>


#include "buffer.h"

GLOBAL( bool )ConvertToJpg( Buffer &source, Buffer &target, int width, int height, int bytespp, int quality, bool bottomUp );
GLOBAL( bool )DecompressJpg( Buffer &source, Buffer &target, int &width, int &height, int &bytespp );
#endif


#if !defined(__LIBBLP_BYTEORDER__)
#define __LIBBLP_BYTEORDER__ 1

#ifdef __BIG_ENDIAN__
#ifdef __APPLE__

#include <CoreFoundation/CFByteOrder.h>

#define _blp_swap_int16(x) CFSwapInt16(x)
#define _blp_swap_int32(x) CFSwapInt32(x)

#else
#error unsupported operating system
#endif

#else

#define _blp_swap_int16(x) (x)
#define _blp_swap_int32(x) (x)

#endif

#endif // __LIBBLP_BYTEORDER__ 