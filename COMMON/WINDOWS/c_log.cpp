#include "stdafx.h"
//---------------------------------------------------------------------------
#include "c_log.h"
//---------------------------------------------------------------------------


#ifdef __BORLANDC__
//---------------------------------------------------------------------------
c_log::c_log(AnsiString fname, __int32 DebugLevel)
{
	access=CreateMutex(NULL,FALSE,NULL);
	slist=new TStringList;
	log_mask=DebugLevel;
	//
	file=NULL;
	LogCallBack=NULL;
	if(fname!="") Open(fname);
}
//---------------------------------------------------------------------------
c_log::~c_log(void)
{
	Close();
	delete slist;
	CloseHandle(access);
}
//---------------------------------------------------------------------------
bool c_log::Open(AnsiString fname)
{
	Close();
    if(fname=="")fname=GetCurrentDir()+"\\"+DEFAULTLOGFILENAME;
	file=fopen(fname.c_str(), "wb");
	return file==NULL ? false : true;
}
//---------------------------------------------------------------------------
bool c_log::Close(void)
{
	if(WaitForSingleObject(access, INFINITE)==WAIT_OBJECT_0)
	{
		if(file!=NULL) { fclose(file); file=NULL; }
		ReleaseMutex(access);
	}
	return true;
}
//---------------------------------------------------------------------------
bool c_log::AddMessage(AnsiString msg, __int32 log_flag)
{
	if((log_flag&log_mask)==log_mask)
	{
		if(WaitForSingleObject(access, 5000)==WAIT_OBJECT_0)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			msg=Format("[%0.2d:%0.2d:%0.2d], flag:%d ", ARRAYOFCONST((st.wHour, st.wMinute, st.wSecond,log_flag)))+msg;
			if(log_flag&LOG_FLAG_SCREEN)
			{
                if(slist->Count>(64*1024))
                { // чтобы не было переполнения памяти
                    slist->Delete(0);
                }
				slist->Add(msg);
			}
			if(log_flag&LOG_FLAG_FILE && file!=NULL)
			{
				fprintf(file, "%s\n", msg.c_str());
				fflush(file);
			}
			if(log_flag&LOG_FLAG_CALLBACK&&LogCallBack!=NULL)
			{
				LogCallBack(msg,callBackParam);
			}
			ReleaseMutex(access);
		} else return false;
	}
	return true;
}
//---------------------------------------------------------------------------
AnsiString c_log::GetMessage(void)
{
	AnsiString astr;
	if(WaitForSingleObject(access, 5000)==WAIT_OBJECT_0)
	{
		if(slist->Count)
		{
			astr=slist->Strings[0];
			slist->Delete(0);
		}
		ReleaseMutex(access);
	}
	return astr;
}
//---------------------------------------------------------------------------
int c_log::GetMessages(TStringList * Messages)
{
	if(Messages==NULL) return 0;
	Messages->Clear();

	int NumProceedMessages = slist->Count;
	for(int i=0;i<NumProceedMessages;i++)
	{
		Messages->Add(slist->Strings[0]);
		slist->Delete(0);
	}
	return NumProceedMessages;
}

//---------------------------------------------------------------------------
int c_log::GetCount(void)
{
	int count=0;
	if(WaitForSingleObject(access, 5000)==WAIT_OBJECT_0)
	{
		count=slist->Count;
		ReleaseMutex(access);
	}
	return count;
}
#endif //__BORLANDC__
//---------------------------------------------------------------------------
void c_log::SetLogCallBack(LogCallBackType LogCallBackSet, PVOID CallBackParam)
{
	LogCallBack=LogCallBackSet;
    callBackParam=CallBackParam;
}
