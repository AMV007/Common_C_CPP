#include "stdafx.h"

#include <stdlib.h>
#include <string.h>

#include "c_file.h"
#include "Compatability.h"

#ifndef _WIN32
#undef 	INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE NULL
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

c_file::c_file(const TCHAR * FileName, const BOOL Write, const BOOL Read,  const BOOL CreateNew)
{
	hFile=INVALID_HANDLE_VALUE;

	fileOpenForRead = false;
	fileOpenForWrite = false;

	FilePos.QuadPart=0;
	if(FileName!=NULL) Open(FileName,Write,Read,CreateNew);
}

c_file::~c_file()
{
	Close();
}

BOOL __fastcall c_file::Opened()
{
	if(hFile==INVALID_HANDLE_VALUE) return false;
	return true;
}

BOOL __fastcall c_file::Open(const TCHAR * FileName, const BOOL Write, const BOOL Read, const BOOL CreateNew)
{
	Close();	// FilePos.QuadPart=0;
	if(FileName==NULL) return false;	

	fileOpenForRead = Read;
	fileOpenForWrite = Write;

#ifdef _WIN32
	int GenericFlag =0;
	if(Read) GenericFlag|=GENERIC_READ;
	if(Write) GenericFlag|=GENERIC_WRITE;

	int FileShareFlag=0;
	if(Read) FileShareFlag|=FILE_SHARE_READ;
	if(Write) FileShareFlag|=FILE_SHARE_WRITE;

	int CreateFlag=0;
	if(Read) CreateFlag|=OPEN_EXISTING;
	if(Write) CreateFlag|=OPEN_ALWAYS;
	if(CreateNew) CreateFlag = CREATE_ALWAYS;

	hFile=CreateFile(FileName, GenericFlag, FileShareFlag, NULL, CreateFlag, 0, NULL);
#else

	if(CreateNew)
	{
		if(Read)hFile=fopen(FileName,"a+");
		if(Write)hFile=fopen(FileName,"w");
	}
	else
	{
		if(Read)hFile=fopen(FileName,"r");
		if(Write)hFile=fopen(FileName,"w+");
	}
#endif
	if(hFile==INVALID_HANDLE_VALUE) return false;
	
	fFileName=my_wa_strdup(FileName);
	return true;
}

void __fastcall c_file::Close()
{
	if(hFile!=INVALID_HANDLE_VALUE)
	{
#ifdef _WIN32
		CloseHandle(hFile);
#else
		fclose(hFile);
#endif
		hFile=INVALID_HANDLE_VALUE;
		free(fFileName);
	}
	FilePos.QuadPart=0;
}

DWORD __fastcall c_file::Read(BYTE * buf, const __int32 BufLen)
{
	if(hFile==INVALID_HANDLE_VALUE||!fileOpenForRead) return 0;

	DWORD BytesReaded=0;
	BOOL res=TRUE;
#ifdef _WIN32
	res=ReadFile(hFile,buf,BufLen, &BytesReaded,NULL);
#else
	BytesReaded=fread(buf,1,BufLen,hFile);
#endif

	FilePos.QuadPart+=BytesReaded;

	if(res)	return BytesReaded;
	else return 0;	
}

BYTE __fastcall c_file::ReadByte(void)
{
	BYTE Data=0;
	Read((BYTE *)&Data,sizeof(Data)); 
	return Data;	
}

WORD __fastcall c_file::ReadWORD(void)
{
	WORD Data=0;
	Read((BYTE *)&Data,sizeof(Data)); 
	return Data;	
}

DWORD __fastcall c_file::ReadDWORD(void)
{
	DWORD Data=0;
	Read((BYTE *)&Data,sizeof(Data)); 
	return Data;	
}

DWORD __fastcall c_file::Write(const BYTE * buf, const __int32 BufLen)
{
	if(hFile==INVALID_HANDLE_VALUE||!fileOpenForWrite) return 0;

	DWORD BytesWrited=0;
	BOOL res=TRUE;
#ifdef _WIN32
	res=WriteFile(hFile,buf,BufLen, &BytesWrited,NULL);
#else
	BytesWrited=fwrite(buf,1,BufLen,hFile);
#endif
	FilePos.QuadPart+=BytesWrited;

	if(res)	return BytesWrited;
	else return 0;
}

BOOL __fastcall c_file::WriteByte(BYTE Data)
{
	return Write((BYTE *)&Data,sizeof(Data));
}

BOOL __fastcall c_file::WriteWORD(WORD Data)
{
	return Write((BYTE *)&Data,sizeof(Data));
}

BOOL __fastcall c_file::WriteDWORD(DWORD Data)
{
	return Write((BYTE *)&Data,sizeof(Data));
}

BOOL __fastcall c_file::ReadLine(TCHAR * buf, __int32 BufLen)
{
	if(hFile==INVALID_HANDLE_VALUE||!fileOpenForRead) return false;
	
	TCHAR TempBuf;
	TCHAR * pBuf=buf;
	DWORD NumReadBytes=0;

	for(int i=0;i<(BufLen-1);i++)
	{
		NumReadBytes=Read((BYTE *)&TempBuf,sizeof(TCHAR));
		FilePos.QuadPart+=NumReadBytes;

		if((NumReadBytes==0&&i==0) // end of file and not enough info
            )
            return false;

		if(TempBuf=='\r') continue;
		if( TempBuf=='\n'
            ||NumReadBytes==0 // end of file
            )
		{
			*pBuf=0; // end string symbol
			return true;
		}
		*pBuf=TempBuf;
		pBuf+=sizeof(TCHAR);
	}

	return false;
}
BOOL __fastcall c_file::WriteLine(const TCHAR * buf, __int32 BufLen)
{
	if(!fileOpenForWrite) return false;

	if(BufLen==0) BufLen=my_wa_strlen(buf);

	if(!Write((PBYTE)buf, BufLen)) return false;
	
	return Write((BYTE *)TEXT("\n"), sizeof(TCHAR));
}

#if(defined(_WIN32)&&!defined(_WIN32_WCE)) 
BOOL __fastcall c_file::SetPos64(__int64 Pos, E_C_FILE_POS type)
{
	LARGE_INTEGER MoveDist;
	MoveDist.QuadPart=Pos;
	return SetFilePointerEx(hFile, MoveDist, &FilePos,type);
}

__int64 __fastcall c_file::GetPos64(void)
{
	return FilePos.QuadPart; 
}
#endif

BOOL __fastcall c_file::SetPos(int Pos, E_C_FILE_POS type)
{
	LONG HighValPtr=0;
	DWORD res=0;
#ifdef _WIN32
	res=SetFilePointer(hFile,Pos,&HighValPtr,type);
#else

	if(!fseek(hFile,Pos,type))
	{
		fpos_t pos;
		if(!fgetpos (hFile,&pos))
		{
			res=pos.__pos;
		}else res=INVALID_SET_FILE_POINTER;
	}else res=INVALID_SET_FILE_POINTER;
#endif

	if(res!=INVALID_SET_FILE_POINTER)
	{
		FilePos.LowPart=res;	
		FilePos.HighPart=HighValPtr;
	}
	return (res!=INVALID_SET_FILE_POINTER);
}
DWORD __fastcall c_file::GetPos(void)
{
	return FilePos.LowPart;
}

DWORD __fastcall c_file::GetFileSize32()
{
#ifdef _WIN32
	return GetFileSize(hFile,0);
#else
	struct stat filestatus;
	stat( fFileName, &filestatus );
	return filestatus.st_size;
#endif
}
