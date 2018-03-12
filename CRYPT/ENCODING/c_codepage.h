#ifndef __H_C_CODEPAGE__
#define __H_C_CODEPAGE__
#include <windows.h> // from BOOL type


class c_codepage
{
private:
	
public :    
	c_codepage(){};
	~c_codepage(){};

	static __int32 GetCodePage(char * CodePageName, __int32 NameLen=0);
	static __int32 GetRequiredBufferSizeToConvert(int CodePageNumber, char * Data, __int32 DataLen);
	static BOOL Convert(int CodePageNumber,  char * DataOut, __int32  DataOutLen, char * DataIn, __int32 DataInLen);    
};
	

#endif //__H_C_CODEPAGE__