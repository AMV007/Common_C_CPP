#include "stdafx.h"

#include <windows.h>
#include <stdio.h>
#include "..\\Compatability.h"

//#define LOG_ANSI_ONLY

#ifndef LOG_FILE
// with line number
#define STRING2(x) #x // need here, not header or preprocessor can't understand
#define STRING(x) STRING2(x)
#pragma message (__FILE__ "[" STRING(__LINE__) "] : warning !!! LOG_FILE not defined !!!")
#undef STRING
#undef STRING2
#endif

#if (defined(_DEBUG)||defined(ADD_LOG_ALLWAYS))

TCHAR LogFileName[MAX_PATH]={0};
bool LogFileSet = false;
bool SetLogFileName(TCHAR * NewFileName)
{
	LogFileSet = false;	
	if(my_wa_strlen(NewFileName)>=MAX_PATH) return false;
	my_wa_sprintf(LogFileName, TEXT("%s"),NewFileName);		
	LogFileSet = true;
	return true;
}

void ResetLog()
{
#ifdef LOG_FILE
	if(!LogFileSet) SetLogFileName(TEXT(LOG_FILE));	
#endif
	if(!LogFileSet) return;
	HANDLE hfile=CreateFile(LogFileName, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_WRITE|FILE_SHARE_READ, 0, TRUNCATE_EXISTING , FILE_ATTRIBUTE_NORMAL, 0);						
	if (hfile!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(hfile);
	}
}

BOOL FirstMessage = TRUE;
BOOL  AddLogInUse=FALSE;// simple multithread
void AddLogInternal(TCHAR * Message,...)
{
	while(AddLogInUse)Sleep(1);
	AddLogInUse=TRUE;
#ifdef LOG_FILE
	if(!LogFileSet) SetLogFileName(TEXT(LOG_FILE));	
#endif
	if(!LogFileSet)
	{
		AddLogInUse = FALSE;
		return;
	}
	
	if(FirstMessage)
	{	// printing session begin	
		FirstMessage = FALSE;
		AddLogInUse=FALSE; // just exception
		SYSTEMTIME LocalTime;
		GetLocalTime(&LocalTime);
		AddLogInternal(TEXT("-----Begin------%.2d:%.2d:%.2d %.2d.%.2d.%.4d -"),
							LocalTime.wHour,LocalTime.wMinute,LocalTime.wSecond,
							LocalTime.wDay,LocalTime.wMonth,LocalTime.wYear							
							);				
		AddLogInUse=TRUE;
	}

	va_list args;  
	va_start(args,Message);

#ifndef LOG_ANSI_ONLY
	// Add TCHAR to log
	HANDLE hFile=CreateFile(LogFileName, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL, 0);						
	if (hFile!=INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if (SetFilePointer(hFile,0,0,FILE_END)!=0xFFFFFFFF)
		{	
			// if message size exceed array
			TCHAR * TempMessageData = new TCHAR[my_wa_strlen(Message)+512];
			my_wa_vsprintf(TempMessageData,Message,args);

			int TempMessageLen = my_wa_strlen(TempMessageData);
			TempMessageData[TempMessageLen++]='\n';			

			WriteFile(hFile,TempMessageData,TempMessageLen*sizeof(TCHAR),&BytesWritten,NULL); 
			if(TempMessageData)delete TempMessageData;
		}
		CloseHandle(hFile);
	}

#else //LOG_ANSI_ONLY
	// add only ansi to log

	FILE * hFile = my_wa_fopen(LogFileName,TEXT("a"));

	if(hFile!=NULL)
	{

		my_wa_vfprintf(hFile,Message,args);		

		fprintf(hFile,"\n");		
		fclose(hFile);
	}

#endif //LOG_ANSI_ONLY

	va_end(args);
	AddLogInUse = FALSE;

}

#endif