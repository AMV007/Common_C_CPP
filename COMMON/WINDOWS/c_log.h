#pragma once

#ifndef TLOG_CLASS
#define TLOG_CLASS

#include <windows.h>
#include <stdio.h>

#define DEFAULTLOGFILENAME "app.log"
// флаги ведения лога
#define LOG_FLAG_DEBUG_LEVEL0   (1<<0)
#define LOG_FLAG_DEBUG_LEVEL1   (1<<1)
#define LOG_FLAG_DEBUG_LEVEL2   (1<<2)
#define LOG_FLAG_DEBUG_LEVEL3   (1<<3)
#define LOG_FLAG_DEBUG_LEVEL4   (1<<4)
#define LOG_FLAG_DEBUG_LEVEL5   (1<<5)
#define LOG_FLAG_DEBUG_LEVEL6   (1<<6)
#define LOG_FLAG_DEBUG_LEVEL7   (1<<7)
#define LOG_FLAG_DEBUG_MASK     (0xff)

#define LOG_FLAG_SPEC_DEBUG1    (1<<8)
#define LOG_FLAG_SPEC_DEBUG2    (1<<9)
#define LOG_FLAG_SPEC_DEBUG3    (1<<10)
#define LOG_FLAG_SPEC_DEBUG4    (1<<11)
#define LOG_FLAG_SPEC_MASK      (0xf00)

#define LOG_FLAG_FILE           (1<<12)
#define LOG_FLAG_SCREEN         (1<<13)
#define LOG_FLAG_CALLBACK       (1<<14)
#define LOG_FLAG_SCREEN_FILE    (LOG_FLAG_SCREEN|LOG_FLAG_FILE)
#define LOG_FLAG_SHOW_MASK      (0xf000)

#ifdef __BORLANDC__
#include <classes.hpp>
#include <inifiles.hpp>
#else
// not implemented, just some ovverrido for compile
#include <string>
#include "..\\c_ListPtr.h"
typedef std::string AnsiString;
typedef c_ListPtr TStringList;
#endif //__BORLANDC__

typedef void (*LogCallBackType) (AnsiString LogMessage, PVOID CallBackParam);

//---------------------------------------------------------------------------
class c_log
{
 private :
    HANDLE access;
    FILE  *file;
    TStringList *slist;
    LogCallBackType LogCallBack;
    __int32 log_mask;


    PVOID callBackParam;

 public :
    c_log(AnsiString fname="", __int32 DebugLevel=LOG_FLAG_DEBUG_LEVEL0);
    ~c_log(void);
    //
    bool Open(AnsiString fname="");
    bool Close(void);
    //
    bool AddMessage(AnsiString msg, __int32 log_flag=(LOG_FLAG_DEBUG_LEVEL0|LOG_FLAG_SPEC_DEBUG1|LOG_FLAG_FILE|LOG_FLAG_SCREEN|LOG_FLAG_CALLBACK));
    AnsiString GetMessage(void);
    __int32 GetMessages(TStringList * Messages);
    __int32 GetCount(void);

    void SetLogCallBack(LogCallBackType LogCallBackSet, PVOID CallBackParam=NULL);
};
//---------------------------------------------------------------------------

//#endif //__BCC32__

#endif //#ifndef TLOG_CLASS
