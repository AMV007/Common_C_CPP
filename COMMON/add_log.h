#ifndef __H__ADD_LOG__
#define __H__ADD_LOG__

#if (defined(_DEBUG)||defined(ADD_LOG_ALLWAYS))
// log only for debug
bool SetLogFileName(TCHAR * NewFileName);
void ResetLog();

#define CONV_STRING2(x) #x
#define CONV_STRING(x) CONV_STRING2(x)

#ifdef _MSC_VER // Microsoft predefenitions
#define CONV_FILE_LINE(x)		TEXT(__FUNCTION__)TEXT("[")TEXT(CONV_STRING(__LINE__))TEXT("]:")TEXT(x)
#define CONV_FILE_LINE_ERR(x)	TEXT(__FUNCTION__)TEXT("[")TEXT(CONV_STRING(__LINE__))TEXT("] Error:")TEXT(x)
#else // non Microsoft
#define CONV_FILE_LINE(x)		TEXT(__FILE__)TEXT("[")TEXT(CONV_STRING(__LINE__))TEXT("]:")TEXT(x)
#define CONV_FILE_LINE_ERR(x)	TEXT(__FILE__)TEXT("[")TEXT(CONV_STRING(__LINE__))TEXT("] Error:")TEXT(x)
#endif

#ifdef _WIN32_WCE
#define  AddLog(x,...) {DEBUGMSG(TRUE,(CONV_FILE_LINE(x)TEXT("\n"),__VA_ARGS__));\
						AddLogInternal(CONV_FILE_LINE(x),__VA_ARGS__);}

#define  AddLogErr(x,...) {DEBUGMSG(TRUE,(CONV_FILE_LINE_ERR(x)TEXT("\n"),__VA_ARGS__));\
							AddLogInternal(CONV_FILE_LINE_ERR(x),__VA_ARGS__);}
#else
#define  AddLog(x,...) {AddLogInternal(CONV_FILE_LINE(x),__VA_ARGS__);}
#define  AddLogErr(x,...) {AddLogInternal(CONV_FILE_LINE(x),__VA_ARGS__);}
#endif
void AddLogInternal(TCHAR * Message, ...);

#else

#define SetLogFileName(NewFileName)	{}
#define AddLog(...)					{}
#define AddLogErr(...)				{}
#define ResetLog(x)					{}

#endif

#endif //__H__ADD_LOG__