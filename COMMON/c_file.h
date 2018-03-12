#ifndef __H_C_FILE__
#define __H_C_FILE__

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "std_types.h"

#endif

typedef enum
{
	C_FILE_FILE_BEGIN=0,
	C_FILE_FILE_CURRENT=1,
	C_FILE_FILE_END=2
}E_C_FILE_POS;

class c_file
{
private:
#ifdef _WIN32
	HANDLE
#else
	FILE *
#endif
		hFile;

	LARGE_INTEGER FilePos;

	volatile BOOL fileOpenForRead;
	volatile BOOL fileOpenForWrite;
public :

	TCHAR *fFileName;

	c_file(const TCHAR * FileName=NULL, const BOOL Write=false, const BOOL Read=true, const BOOL CreateNew=false);
	~c_file();
	
	virtual BOOL __fastcall Opened();
	virtual BOOL __fastcall Open(const TCHAR * FileName, const BOOL Write=false, const BOOL Read=true, const BOOL CreateNew=false);
	virtual void __fastcall Close(); // constructor analog, possibility to start - stop thread

	virtual DWORD __fastcall Read(BYTE * buf, const int BufLen);	
	virtual BYTE  __fastcall ReadByte(void);	
	virtual WORD  __fastcall ReadWORD(void);	
	virtual DWORD __fastcall ReadDWORD(void);	

	virtual DWORD __fastcall Write(const BYTE * buf, const int BufLen);		
	virtual BOOL  __fastcall WriteByte(BYTE Data);		
	virtual BOOL  __fastcall WriteWORD(WORD Data);		
	virtual BOOL  __fastcall WriteDWORD(DWORD Data);		
	
	virtual BOOL __fastcall ReadLine(TCHAR * buf, int BufLen=0);
	virtual BOOL __fastcall WriteLine(const TCHAR * buf, int BufLen=0);

	// file position
	virtual BOOL SetPosBeginFile()	{return SetPos(0,C_FILE_FILE_BEGIN);};
	virtual BOOL SetPosEndFile()	{return SetPos(0,C_FILE_FILE_END);};

#if(defined(_WIN32)&&!defined(_WIN32_WCE)) 
	virtual BOOL __fastcall SetPos64(__int64 Pos, E_C_FILE_POS type=C_FILE_FILE_BEGIN);
	virtual __int64 __fastcall GetPos64(void);
#endif

	virtual BOOL __fastcall SetPos(int Pos, E_C_FILE_POS type=C_FILE_FILE_BEGIN);
	virtual DWORD __fastcall GetPos(void);

    // file size
    virtual DWORD __fastcall GetFileSize32();
#if(defined(_WIN32)&&!defined(_WIN32_WCE)) 
    virtual __int64 __fastcall GetFileSize64()
    {
        LARGE_INTEGER res;
        GetFileSizeEx(hFile,&res);
        return res.QuadPart;
    };
#endif
};

#endif //__H_C_FILE__
