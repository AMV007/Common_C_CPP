/*
 * c_sock_base class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

/*
Base class for working with sockets
based on berkley functions
*/
#pragma once

#ifndef __H_C_COSCK__
#define __H_C_COSCK__

//#include <Winsock2.h>
#include <Windows.h>
#pragma comment( lib, "ws2_32.lib" )


//-PORTS------------------------------------------------------------------------------------
// Common port assignments
typedef enum {
  C_SOCK_ECHO_PORT       = 7,   // Echo port
  C_SOCK_FTPDATA_PORT    = 20,  // FTP data port
  C_SOCK_FTP_PORT        = 21,  // FTP port
  C_SOCK_SSH_PORT        = 22,  // Secure shell port
  C_SOCK_TELNET_PORT     = 23,  // Telnet port
  C_SOCK_SMTP_PORT       = 25,  // Simple mail transfer protocol port
  C_SOCK_TIME_PORT       = 37,  // Time server
  C_SOCK_NAME_PORT       = 42,  // Name server
  C_SOCK_NAMESERVER_PORT = 53,  // Domain name server
  C_SOCK_TFTP_PORT       = 69,  // Trivial file transfer port
  C_SOCK_FINGER_PORT     = 79,  // Finger port
  C_SOCK_HTTP_PORT       = 80,  // HTTP port
  C_SOCK_POP_PORT        = 109, // Postoffice protocol 1
  C_SOCK_POP2_PORT       = 109, // Postoffice protocol 2
  C_SOCK_POP3_PORT       = 110, // Postoffice protocol 3
  C_SOCK_SFTP_PORT       = 115, // Simple file transfer protocol
  C_SOCK_NNTP_PORT       = 119, // Network news transfer protocol
  C_SOCK_NTP_PORT        = 123, // Network time protocol
  C_SOCK_IMAP_PORT       = 143, // Internet message access protocol
  C_SOCK_SNMP_PORT       = 161, // SNMP port
  C_SOCK_LDAP_PORT       = 389, // Lightweight directory access protocol
  C_SOCK_HTTPS_PORT      = 443, // HTTPS port
  C_SOCK_SYSLOG_PORT     = 514, // UNIX syslog port
  C_SOCK_PRINTER_PORT    = 515, // UNIX line printer (lp) port
  C_SOCK_SLDAP_PORT      = 636, // Secure LDAP
  C_SOCK_RSYNC_PORT      = 873, // UNIX rsync port
  C_SOCK_SIMAP_PORT      = 993, // Secure IMAP
  C_SOCK_SPOP3_PORT      = 995, // Secure POP3
  C_SOCK_WSMAN_PORT      = 5985, // Secure POP3
}_E_C_COSK_PORTS;

//-ERRORS------------------------------------------------------------------------------------

#define C_SOCK_NO_ERROR								0

#define C_SOCK_BASE_BASE_ERROR_NUM					(0)
#define C_SOCK_OVER_BASE_ERROR_NUM					(-100)
#define FTP_CH_ERROR_BASE							(-1000)
#define SMTP_CH_ERROR_BASE							(-2000)
#define POP3_CH_ERROR_BASE							(-3000)

#define C_SOCK_ERROR_GET_HOST_NAME_INIT				(C_SOCK_BASE_BASE_ERROR_NUM  -1 )
#define C_SOCK_ERROR_SET_SOCK_OPT					(C_SOCK_BASE_BASE_ERROR_NUM  -2 )
#define C_SOCK_ERROR_GET_SOCK_NAME					(C_SOCK_BASE_BASE_ERROR_NUM  -3 )
#define C_SOCK_ERROR_CREATE_SOCKET					(C_SOCK_BASE_BASE_ERROR_NUM  -4 )
#define C_SOCK_ERROR_CONNECT_CLIENT_SOCKET			(C_SOCK_BASE_BASE_ERROR_NUM  -5 )
#define C_SOCK_ERROR_BIND_SERVER_SOCKET				(C_SOCK_BASE_BASE_ERROR_NUM  -6 )
#define C_SOCK_ERROR_ACCEPT_SOCKET					(C_SOCK_BASE_BASE_ERROR_NUM  -7 )
#define C_SOCK_ERROR_LISTEN_SOCKET					(C_SOCK_BASE_BASE_ERROR_NUM  -8 )
#define C_SOCK_ERROR_GET_HOST_NAME					(C_SOCK_BASE_BASE_ERROR_NUM  -9 )

#define C_SOCK_ERROR_RECV					        (C_SOCK_BASE_BASE_ERROR_NUM -10 )
#define C_SOCK_ERROR_RECVANY					    (C_SOCK_BASE_BASE_ERROR_NUM -11 )
#define C_SOCK_ERROR_SEND					        (C_SOCK_BASE_BASE_ERROR_NUM -12 )

#define C_SOCK_ERROR_RECV_SZ					    (C_SOCK_BASE_BASE_ERROR_NUM -13 )
#define C_SOCK_ERROR_RECVANY_SZ					    (C_SOCK_BASE_BASE_ERROR_NUM -14 )
#define C_SOCK_ERROR_SEND_SZ				        (C_SOCK_BASE_BASE_ERROR_NUM -15 )

#define C_SOCK_ERROR_RECV_PARAM				        (C_SOCK_BASE_BASE_ERROR_NUM -16 )
#define C_SOCK_ERROR_RECVANY_PARAM				    (C_SOCK_BASE_BASE_ERROR_NUM -17 )
#define C_SOCK_ERROR_SEND_PARAM				        (C_SOCK_BASE_BASE_ERROR_NUM -18 )

#define C_SOCK_ERROR_SET_BLOCK_PARAM		        (C_SOCK_BASE_BASE_ERROR_NUM -19 )
#define C_SOCK_ERROR_BLOCK_WRITE_TIMEOUT	        (C_SOCK_BASE_BASE_ERROR_NUM -20 )
#define C_SOCK_ERROR_BLOCK_READ_TIMEOUT				(C_SOCK_BASE_BASE_ERROR_NUM -21 )

#define C_SOCK_ERROR_NON_BLOCK_CONNECT     			(C_SOCK_BASE_BASE_ERROR_NUM -22 )
#define C_SOCK_ERROR_NON_BLOCK_CONNECT_WAIT			(C_SOCK_BASE_BASE_ERROR_NUM -23 )
#define C_SOCK_ERROR_NON_BLOCK_CONNECT_WAIT2		(C_SOCK_BASE_BASE_ERROR_NUM -24 )
#define C_SOCK_ERROR_CONNECT_TIMEOUT       			(C_SOCK_BASE_BASE_ERROR_NUM -25 )

#define C_SOCK_ERROR_GET_HOST_INFO       			(C_SOCK_BASE_BASE_ERROR_NUM -26 )

#define C_SOCK_DEF_CON_BIND_ACC_TIMEOUT         (3000)

typedef enum
{
	t_Unknown_Socket=0,
	t_Connect_Socket=	(1<<0),
	t_Bind_Socket=		(1<<1),	
	t_Listen_Socket=	(1<<2),
	t_Accept_Socket=	(1<<3)
}e_Socket_Status;


//-SOCK_INTERFACE------------------------------------------------------------------------------------
class Ic_sock
{	
private:
	virtual void __fastcall SetError(int SetError) = 0;	
	friend class c_sock_compl;

public:	
	SOCKADDR_IN sock_addr;
	int Socket_Status;
	SOCKET sock;	
    
    virtual ~Ic_sock(){}; // necessary

	virtual BOOL InitializeWSASystem(void) = 0;
    virtual BOOL ReleaseWSASystem(void) = 0;		// not need

	virtual BOOL __fastcall Create(	DWORD addr=0, WORD port=80,
											char* addrName=NULL, BOOL NonBlockingConn = false,
											int protocol=IPPROTO_TCP, 
											int s_type=SOCK_STREAM) = 0;

    virtual void __fastcall SetSockTimeout(DWORD ConnectTimeout_ms=0, DWORD BindTimout_ms=0, DWORD AcceptTimeout_ms=0) = 0;

	virtual BOOL __fastcall Connect_s() = 0;	
    virtual BOOL __fastcall Bind_s() = 0;
    virtual BOOL __fastcall Accept_s(SOCKET ListenSocket) = 0;
	virtual BOOL __fastcall Listen_s(int max_connections=1) = 0;
	virtual BOOL __fastcall Close(void) = 0;

	virtual __int32 __fastcall Recv(BYTE *buf, DWORD size, DWORD timeout=INFINITE) = 0;
    virtual __int32 __fastcall RecvAny(BYTE *buf, DWORD size) = 0;
    virtual __int32 __fastcall Send(BYTE *buf, DWORD size, DWORD timeout=INFINITE) = 0;
	virtual BOOL __fastcall WaitForWriteData(int timeout_ms) = 0;	
	virtual BOOL __fastcall WaitForRecieveData(int timeout_ms) =0;

	virtual BOOL __fastcall SetSockOpt(int level, __int32 optName,const void *optVal, __int32 optLen) = 0;
	virtual BOOL __fastcall GetSockName() = 0;
    virtual BOOL __fastcall GetHostName(char *sbuf, __int32 sbuflen) = 0;
	virtual hostent * __fastcall GetHostInformation(char *hostname=NULL) = 0;
	virtual char * __fastcall GetDomainName() = 0;

	virtual BOOL __fastcall IsAlive() = 0;    
	virtual BOOL __fastcall Connected() = 0;    
	virtual BOOL __fastcall Binded() = 0;    
	virtual BOOL __fastcall Listened() = 0;    
	virtual BOOL __fastcall Accepted() = 0;    

	virtual void __fastcall	ClearError() = 0;	
	virtual __int32 __fastcall GetError() = 0;
	virtual __int32 __fastcall GetWSAError() = 0;	
};

//-BASE_SOCK_CLASS------------------------------------------------------------------------------------

#define MAX_HOST_NAME_LEN       256  // Maximum string name length
#define MAX_DOMAIN_NAME_LEN       256  // Maximum string name length

// for nested classes - FTP, SMTP,POP3 ...
#define SOCK_COMMAND_BUF_SIZE		(1*1024) // standard buffer for commands
#define SOCK_REPLY_BUF_SIZE			(8*1024) // standard buffer for reply

class c_sock_base: public Ic_sock
{
private:

	char * localHostName;
	char * localDomainInfo;

	volatile static long WSAInit;

	int LastWSAError;
	int LocalError;	  

	virtual void __fastcall SetError(int SetError)
    {
        // чтобы первая ошибка не затиралась
        if(LocalError!=C_SOCK_NO_ERROR&&SetError!=C_SOCK_NO_ERROR) return;

        LocalError=SetError;
        if(LocalError==C_SOCK_NO_ERROR) LastWSAError=C_SOCK_NO_ERROR;
        else LastWSAError=WSAGetLastError();
    }

	friend class c_sock_over;

protected:

	BOOL Initialized;
	u_long nonBlocking; // not blocking socket

	// multithreaded try
	volatile bool ReadInUse;
    volatile bool WriteInUse;

    DWORD connectTimeout_ms;
    DWORD bindTimeout_ms;
    DWORD acceptTimeout_ms;

	BOOL __fastcall Create_Internal(DWORD addr, WORD port,
											char* addrName, 
											int *protocol, 
											int *s_type);
public:
	// constructors
	c_sock_base(void);
	~c_sock_base(void);

	BOOL InitializeWSASystem(void);
    BOOL ReleaseWSASystem(void);		// not need

    virtual void __fastcall SetSockTimeout(DWORD ConnectTimeout_ms=0, DWORD BindTimout_ms=0, DWORD AcceptTimeout_ms=0);
	virtual BOOL __fastcall Create(	DWORD addr=0, WORD port=80,
											char* addrName=NULL, BOOL NonBlockingConn = false,
											int protocol=IPPROTO_TCP, 
											int s_type=SOCK_STREAM);

	virtual BOOL __fastcall Connect_s();
    virtual BOOL __fastcall Bind_s();
    virtual BOOL __fastcall Accept_s(SOCKET ListenSocket);
	BOOL __fastcall Listen_s(int max_connections=1);	
	virtual BOOL __fastcall Close(void);	

	virtual __int32 __fastcall Recv(BYTE *buf, DWORD size, DWORD timeout=INFINITE);
    virtual __int32 __fastcall RecvAny(BYTE *buf, DWORD size);
    virtual __int32 __fastcall Send(BYTE *buf, DWORD size, DWORD timeout=INFINITE);
	BOOL __fastcall WaitForWriteData(int timeout_ms);	
	BOOL __fastcall WaitForRecieveData(int timeout_ms);

	BOOL __fastcall SetSockOpt(int level, __int32 optName, const void *optVal, __int32 optLen);
	BOOL __fastcall GetSockName();
    BOOL __fastcall GetHostName(char *sbuf, __int32 sbuflen);
	hostent * __fastcall GetHostInformation(char *hostname=NULL);
	char * __fastcall GetDomainName();

	virtual BOOL __fastcall IsAlive()
	{
		if (sock==INVALID_SOCKET||sock==NULL)return false;
		else return true;
	}

	virtual BOOL __fastcall Connected(){return (IsAlive()&&(Socket_Status|t_Connect_Socket));};
	virtual BOOL __fastcall Binded(){return (IsAlive()&&(Socket_Status|t_Bind_Socket));};
	virtual BOOL __fastcall Listened(){return (IsAlive()&&(Socket_Status|t_Listen_Socket));};
	virtual BOOL __fastcall Accepted(){return (IsAlive()&&(Socket_Status|t_Accept_Socket));};

	virtual void __fastcall	ClearError() {c_sock_base::SetError(C_SOCK_NO_ERROR);};
	virtual __int32 __fastcall GetError()		{return  LocalError;};
	virtual __int32 __fastcall GetWSAError()	{return LastWSAError;};	
};

#endif __H_C_COSCK__