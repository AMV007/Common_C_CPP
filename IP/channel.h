#ifndef __CHANNEL__
#define __CHANNEL__

// deprecated class, use instead sock_base

#include <Winsock2.h>
//#define DATAR_SIZE  (32*1024)
//#define DATAW_SIZE  (32*1024)

#pragma comment( lib, "ws2_32.lib" )

#define CHANNEL_NO_ERROR                        0
#define CHANNEL_ERROR_ASYNC_TRANSFER            -1
#define CHANNEL_ERROR_GET_HOST_NAME_INIT        -2
#define CHANNEL_ERROR_SET_SOCK_OPT              -3
#define CHANNEL_ERROR_GET_SOCK_NAME             -4
#define CHANNEL_ERROR_CREATE_SOCKET             -5
#define CHANNEL_ERROR_CONNECT_CLIENT_SOCKET     -7
#define CHANNEL_ERROR_BIND_SERVER_SOCKET        -8
#define CHANNEL_ERROR_ACCEPT_SOCKET             -9
#define CHANNEL_ERROR_LISTEN_SOCKET             -10
#define CHANNEL_ERROR_GET_HOST_NAME             -11

#define CHANNEL_ERROR_ASYNC_TRANSFER_SOCKET_RECV    -12
#define CHANNEL_ERROR_ASYNC_TRANSFER_SOCKET_RECVANY -13
#define CHANNEL_ERROR_ASYNC_TRANSFER_SOCKET_SEND    -14

#define CHANNEL_ERROR_ASYNC_TRANSFER_RECV_OVER      -15
#define CHANNEL_ERROR_ASYNC_TRANSFER_RECVANY_OVER   -16
#define CHANNEL_ERROR_ASYNC_TRANSFER_SEND_OVER      -17

#define CHANNEL_ERROR_ASYNC_TRANSFER_RECV_SZ        -18
#define CHANNEL_ERROR_ASYNC_TRANSFER_RECVANY_SZ     -19
#define CHANNEL_ERROR_ASYNC_TRANSFER_SEND_SZ        -20

#define CHANNEL_ERROR_ASYNC_TRANSFER_RECV_PEN       -21
#define CHANNEL_ERROR_ASYNC_TRANSFER_RECVANY_PEN    -22
#define CHANNEL_ERROR_ASYNC_TRANSFER_SEND_PEN       -23


#define SOCKET_CONNECT_TIMEOUT_MS   3000
#define SOCKET_ACCEPT_TIMEOUT_MS    3000

typedef struct
{
    SOCKET sock;

    SOCKADDR_IN peer;

    WSAOVERLAPPED ovr, ovw;
    WSABUF bufr,bufw;
    volatile bool ReadInUse;
    volatile bool WriteInUse;

    bool Initialized;

    void __fastcall Init()
    {   
        sock=INVALID_SOCKET;

	    memset(&ovr, 0, sizeof(WSAOVERLAPPED));
    	memset(&ovw, 0, sizeof(WSAOVERLAPPED));

    	ovr.hEvent=WSA_INVALID_EVENT;
	    ovw.hEvent=WSA_INVALID_EVENT;

        ReadInUse=WriteInUse=false;

        Initialized=false;
    }

    bool __fastcall CreateEvent_s()
    {
        if(ovr.hEvent==WSA_INVALID_EVENT)
        {
            ovr.hEvent=WSACreateEvent();
        }
        if(ovw.hEvent==WSA_INVALID_EVENT)
        {
           ovw.hEvent=WSACreateEvent();
        }                              		

		if( ovr.hEvent!=WSA_INVALID_EVENT &&
			ovw.hEvent!=WSA_INVALID_EVENT)
		{
            return true;
		}
        return false;
    }

    void __fastcall Close()
    {
        if(sock!=INVALID_SOCKET)
        {
            DWORD flag1=0,flag2=0;
            DWORD sz=0;
            WSASetEvent(ovr.hEvent);
            WSASetEvent(ovw.hEvent);
            if(!WSAGetOverlappedResult(sock, &ovr, &sz, FALSE, &flag1)||
                !WSAGetOverlappedResult(sock, &ovw, &sz, FALSE, &flag2))
            {
                CancelIo((HANDLE)sock);
            }
            shutdown(sock, SD_BOTH);
            closesocket(sock);
            sock=INVALID_SOCKET;
        }

        ReadInUse=WriteInUse=false;

        Initialized=false;
    }

    void __fastcall FreeResources()
    {
        if(ovr.hEvent!=WSA_INVALID_EVENT)
        {
            WSACloseEvent(ovr.hEvent);
            ovr.hEvent=WSA_INVALID_EVENT;
        }
        if(ovw.hEvent!=WSA_INVALID_EVENT)
        {
            WSACloseEvent(ovw.hEvent);
            ovw.hEvent=WSA_INVALID_EVENT;
        }
    }
}s_SockParam;

//---------------------------------------------------------------------------
class TChannel {
 private :

    s_SockParam p_connect_bind;
    s_SockParam p_accept;

    BOOL InitializeWSASystem(void);
    BOOL ReleaseWSASystem(void);
    static long StartupProcessed;

    __int32 LastError;
    __int32 LastWSAError;

    char * localHostName;

    // не удалять и не освобождать
    hostent * localHostInfo;

    __int32 __fastcall RecvUni(s_SockParam * SockParam,    BYTE *buf, DWORD size, DWORD timeout=INFINITE);
    __int32 __fastcall RecvAnyUni(s_SockParam * SockParam, BYTE *buf, DWORD size);
    __int32 __fastcall SendUni(s_SockParam * SockParam,    BYTE *buf, DWORD size, DWORD timeout=INFINITE);
    BOOL __fastcall WaitForRecieveDataUni(s_SockParam * SockParam, __int32 seconds, __int32 useconds);

  public :

    TChannel(void);
    ~TChannel(void);

    virtual BOOL __fastcall InitSocket_con_bind(DWORD addr=0, WORD port=80, char* addrName=NULL, __int32 protocol=IPPROTO_IP, __int32 s_type=SOCK_STREAM);

    virtual BOOL __fastcall Connect_s();

    virtual BOOL __fastcall Bind_s();

    virtual BOOL __fastcall Accept_s();

    virtual BOOL __fastcall Listen(int max_connections=1);

    virtual __int32 __fastcall Recv(BYTE *buf, DWORD size, DWORD timeout=INFINITE);
    virtual __int32 __fastcall RecvAny(BYTE *buf, DWORD size);
    virtual BOOL __fastcall WaitForRecieveData(int seconds, __int32 useconds);
    virtual __int32 __fastcall Send(BYTE *buf, DWORD size, DWORD timeout=INFINITE);

    virtual __int32 __fastcall Recv_ac(BYTE *buf, DWORD size, DWORD timeout=INFINITE);
    virtual __int32 __fastcall RecvAny_ac(BYTE *buf, DWORD size);
    virtual BOOL __fastcall WaitForRecieveData_ac(int seconds, __int32 useconds);
    virtual __int32 __fastcall Send_ac(BYTE *buf, DWORD size, DWORD timeout=INFINITE);
    
    virtual BOOL __fastcall Close(void);
    virtual BOOL __fastcall CloseAccept(void);

    virtual BOOL __fastcall SetSockOpt(int level, __int32 optName,
						 const void *optVal, __int32 optLen);

    virtual BOOL __fastcall GetSockName();
    virtual BOOL __fastcall GetHostName(char *sbuf, __int32 sbuflen);
    virtual hostent * __fastcall GetHostInformation(char *hostname=NULL);

    virtual SOCKADDR_IN * __fastcall GetConBindIn_AddrPtr()
    {
        return &p_connect_bind.peer;
    }

    virtual BOOL __fastcall Connected_Binded()
    {
        if (p_connect_bind.sock==INVALID_SOCKET||p_connect_bind.sock==NULL)
            return false;
        return true;
    }

    virtual BOOL __fastcall Accepted()
    {
        if (p_accept.sock==INVALID_SOCKET||p_accept.sock==NULL)
            return false;
        return true;
    }

    virtual __int32 __fastcall GetLastError_s()
    {
        return  LastError;
    }

    virtual __int32 __fastcall WSAGetLastError_s()
    {
        return LastWSAError;
    }

    virtual void __fastcall SetLastError_s(int SetError)
    {
        // чтобы первая ошибка не затиралась
        if(LastError!=CHANNEL_NO_ERROR&&SetError!=CHANNEL_NO_ERROR) return;

        LastError=SetError;
        if(LastError==CHANNEL_NO_ERROR) LastWSAError=CHANNEL_NO_ERROR;
        else LastWSAError=WSAGetLastError();
    }
};
//---------------------------------------------------------------------------
#endif
