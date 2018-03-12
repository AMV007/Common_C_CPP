// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the Common_C_CPP_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// Common_C_CPP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef COMMON_C_CPP_EXPORTS
#define COMMON_C_CPP_API __declspec(dllexport)
#else
#define COMMON_C_CPP_API __declspec(dllimport)
#endif


#define  Quote_It( It )  #It
#define  Common_C_CPP_Path( __X )  Quote_It(  \
	E:\\PRJ\\LIB\\Common_C_CPP\\__X )


