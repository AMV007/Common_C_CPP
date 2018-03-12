#pragma once

/*
 this class - just connection via proxy for HTTP
 not implemented via Visual Studio
*/
#include "channel.h"
#include "..\\COMMON\\c_List.h"

#define ERROR_SOCKS4_ERROR1                  -1005
#define ERROR_SOCKS4_ERROR2                  -1006
#define ERROR_SOCKS4_ERROR3                  -1007
#define ERROR_SOCKS4_ERROR4                  -1008
#define ERROR_SOCKS4_ERROR5                  -1009

#define ERROR_SOCKS5_ERROR1                  -1010
#define ERROR_SOCKS5_ERROR2                  -1011
#define ERROR_SOCKS5_ERROR3                  -1012
#define ERROR_SOCKS5_ERROR4                  -1013
#define ERROR_SOCKS5_ERROR5                  -1014
#define ERROR_SOCKS5_ERROR6                  -1015
#define ERROR_SOCKS5_ERROR7                  -1016
#define ERROR_SOCKS5_ERROR8                  -1017
#define ERROR_SOCKS5_ERROR9                  -1018
#define ERROR_SOCKS5_ERROR10                 -1019

#ifdef __BORLANDC__
    #include <classes.hpp>
#elif  _MSC_VER
	#include <string>	
	#include "..\\COMMON\\c_ListPtr.h"
	typedef std::string AnsiString;
	typedef c_ListPtr TStringList;
#endif

#define	SOCKS_INTERVAL	1000

typedef enum
{
	PTYPE_NO_PROXY=0,
	PTYPE_HTTP=1,
	PTYPE_SOCKS_VER4=4,
	PTYPE_SOCKS_VER5=5
}e_ProxyType;

typedef struct
{   
    DWORD IP;
    WORD Port;	
    e_ProxyType ProxyType;
    AnsiString Login;
    AnsiString Pass;
}s_ProxySettings;
//---------------------------------------------------------------------------
class TPChannel : public TChannel
{
 private :
    BOOL __fastcall ConnectSocks4(AnsiString HostName);
    BOOL __fastcall ConnectSocks5(AnsiString HostName);

    AnsiString CurrHostName;

    TStringList * HostNames;
    c_List<int> hostIP;

    WORD DestPort;
    __int32 DestIPAddress;

  public :

    __int32 LastError;
    s_ProxySettings ProxySettings;

    void __fastcall SetProxy(s_ProxySettings * NewProxySettings);

    BOOL __fastcall Connect(AnsiString Address, WORD Port, s_ProxySettings * NewProxySettings=NULL);
    void __fastcall Disconnect();

    // working function
    AnsiString __fastcall GetHostName(AnsiString WebAddress, AnsiString * RelativePath=NULL);
    AnsiString __fastcall ConvertToASCII(PCHAR Data, __int32 DataLength);

    TPChannel(void);
    ~TPChannel(void);
};
//---------------------------------------------------------------------------



