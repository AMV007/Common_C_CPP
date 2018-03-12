/*
 * c_sock_compl class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/
/*
Complex class for working with sockets
2 sockets, usually 1 - bind, listen, 2 - accept
using in FTP connsctions
in constructor have choose to create - overlapped windows or berkley sockets
*/

#pragma once 
#ifndef __H_C_SOCK_COMPLEX__
#define __H_C_SOCK_COMPLEX__ 

#include "c_sock_base.h"
#include "c_sock_over.h"

class c_sock_compl
{
private:
	void RemoveResources();	

	void virtual __fastcall SetError(int SetError)
    {
		if(accept_sock->IsAlive())		
			accept_sock->SetError(SetError);		
		else 
			conn_bind_sock->SetError(SetError);		
    }

public:
	Ic_sock * conn_bind_sock;
	Ic_sock * accept_sock;

	c_sock_compl(BOOL use_overlapped=FALSE);	// if not NULL changesocktype on create
	~c_sock_compl(void);

	void ChangeOverType(BOOL use_overlapped);

	BOOL __fastcall Create(DWORD addr=0, WORD port=80, char* addrName=NULL, BOOL NonBlockingConn=FALSE, __int32 protocol=IPPROTO_TCP, __int32 s_type=SOCK_STREAM);

	BOOL __fastcall Connect_s();
    BOOL __fastcall Bind_s();
    BOOL __fastcall Accept_s();
	BOOL __fastcall Listen_s(int max_connections=1);		
	
	BOOL __fastcall Close(void);	

	BOOL __fastcall SetSockOpt(int level, __int32 optName, const void *optVal, __int32 optLen);
	BOOL __fastcall GetSockName();
	BOOL __fastcall GetHostName(char *sbuf, __int32 sbuflen);
	hostent * __fastcall GetHostInformation(char *hostname=NULL);

	void __fastcall ClearError(){c_sock_compl::SetError(C_SOCK_NO_ERROR);};

	int __fastcall GetError()
	{
		int LocalError=accept_sock->GetError();
		if(LocalError==C_SOCK_NO_ERROR) return conn_bind_sock->GetError();					
		return  LocalError;
	}

	int __fastcall GetWSAError()	
	{
		int LocalError=accept_sock->GetError();
		if(LocalError==C_SOCK_NO_ERROR) return conn_bind_sock->GetWSAError();					
		return  accept_sock->GetWSAError();
	}	
};

#endif //__H_C_SOCK_COMPLEX__