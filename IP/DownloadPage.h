#pragma once
#include "pchannel.h"


//#pragma comment( lib, "zlib1.lib" )
//---------------------------------------------------------------------------

#define ERROR_CONNECT_HOST                  -1001
#define ERROR_CONNECT_PROXY_AUTHENTICATION  -1002
#define ERROR_GZIPHEADER                    -1003
#define ERROR_GZIP_LEN                      -1004
#define ERROR_GATEWAY_TIMEOUT               -1005


class TDownloadPage {
 private :
    TPChannel Channel;

    AnsiString LastPage;
    __int32 LastMaxSize;
    AnsiString HTTPProxyAuth;

    PBYTE TempData;

    AnsiString __fastcall ConvertToASCII(PCHAR Data, __int32 DataLength);
  public :
    __int32 LastError;

    AnsiString __fastcall DownloadPage(AnsiString WebAddress,int MaxSize=-1, BOOL USEGZip=false, WORD Port=80, AnsiString PostParam="");
    AnsiString __fastcall RestartDownloadLastPage();

    void __fastcall SetProxy(s_ProxySettings * NewProxySettings);
    void __fastcall HangUp();
    // конструктор
    TDownloadPage(void);
    ~TDownloadPage(void);
};
//---------------------------------------------------------------------------

