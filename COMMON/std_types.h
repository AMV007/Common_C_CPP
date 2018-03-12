
/*
 * Define the base type, architecture-wise
 * STD_LIMIT_INT8 - just only 8 bit int restriction
 * STD_LIMIT_INT16 - just only 16 bit int restriction
 * STD_LIMIT_INT32 - just only 32 bit int restriction
 */
#ifndef __H_STD_TYPES_MY__
#define __H_STD_TYPES_MY__

#if defined(_MSC_VER) && defined(_M_IX86)
	#undef(STD_LIMIT_INT32)
	#undef(STD_LIMIT_INT16)
	#undef(STD_LIMIT_INT8)
#endif

// For linux datatypes
#ifndef WIN32
// LINUX
	#include "sys/types.h"
	#ifndef DWORD	
	// windows.h
		typedef unsigned long DWORD;
		typedef short WCHAR;
		typedef void * HANDLE;
		#define MAX_PATH PATH_MAX
		typedef unsigned char BYTE;
		typedef BYTE* PBYTE;
		typedef unsigned short WORD;
		typedef unsigned int BOOL;
		typedef unsigned int UINT;
		typedef char TCHAR;
		typedef unsigned long UINT_PTR;
		typedef long	LONG;
		typedef long long LONGLONG;

		typedef int __int32;
		typedef long long __int64;

		#define FALSE 	0
		#define TRUE	1

		#define INVALID_HANDLE_VALUE ((unsigned int)(-1))

		#define TEXT(x)	x
		typedef char* LPCSTR;

		typedef union _LARGE_INTEGER {
		  struct {
		    DWORD LowPart;
		    LONG  HighPart;
		  } ;
		  struct {
		    DWORD LowPart;
		    LONG  HighPart;
		  } u;
		  LONGLONG QuadPart;
		} LARGE_INTEGER, *PLARGE_INTEGER;

	#endif
	#define __fastcall
#else
// WINDOWS
	typedef t_int8  __int8
	typedef t_uint8  unsigned __int8

	#ifndef(STD_LIMIT_INT8)
		typedef t_int16 __int16
		typedef t_uint16 unsigned __int16

		#ifndef(STD_LIMIT_INT16)
			typedef t_int32 __int32
			typedef t_uint32 unsigned __int32

			#ifndef (STD_LIMIT_INT32)
				typedef t_int64 __int64	
				typedef t_uint64 unsigned __int64
			#endif //#ifndef (STD_LIMIT_INT32)
		#endif //#ifndef(STD_LIMIT_INT16)
	#endif //#ifndef(STD_LIMIT_INT8)

	typedef t_float32 float
	typedef t_float64 double
#endif //WIN32

#endif //__H_STD_TYPES_MY__
