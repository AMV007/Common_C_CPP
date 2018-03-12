/*
 * c_sock_compl class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/
#include "stdafx.h"

#include "c_sock_compl.h"

c_sock_compl::c_sock_compl(BOOL use_overlapped)
{
	conn_bind_sock=NULL;	
	accept_sock=NULL;
	
	ChangeOverType(use_overlapped);	
}

c_sock_compl::~c_sock_compl(void)
{
	RemoveResources();
}

void c_sock_compl::RemoveResources()
{
	if(conn_bind_sock!=NULL)
    {
		delete conn_bind_sock;
        conn_bind_sock=NULL;
    }
	if(accept_sock!=NULL)
    {
		delete accept_sock;
        accept_sock=NULL;
    }
}

void c_sock_compl::ChangeOverType(BOOL use_overlapped)
{
	RemoveResources();

	if(use_overlapped)
	{
		conn_bind_sock = new c_sock_over();
		accept_sock = new c_sock_over();
	}
	else
	{
		conn_bind_sock = new c_sock_base();
		accept_sock = new c_sock_base();
	}
}


BOOL __fastcall c_sock_compl::Create(DWORD addr, WORD port, char* addrName,BOOL NonBlockingConn, __int32 protocol, __int32 s_type)
{
	return conn_bind_sock->Create(addr, port, addrName, NonBlockingConn, protocol, s_type);
}

BOOL __fastcall c_sock_compl::Connect_s()
{
	return conn_bind_sock->Connect_s();
}

BOOL __fastcall c_sock_compl::Bind_s()
{
	return conn_bind_sock->Bind_s();
}	

BOOL __fastcall c_sock_compl::Accept_s()
{
	if((conn_bind_sock->Socket_Status|(t_Bind_Socket|t_Listen_Socket))
		!=(t_Bind_Socket|t_Listen_Socket) ) return false;
	return accept_sock->Accept_s(conn_bind_sock->sock);
}


BOOL __fastcall c_sock_compl::Listen_s(int max_connections)
{
	return conn_bind_sock->Listen_s(max_connections);
}

BOOL __fastcall c_sock_compl::Close(void)
{
	BOOL res=conn_bind_sock->Close();	
	return (accept_sock->Close()&&res);
}

BOOL __fastcall c_sock_compl::SetSockOpt(int level, __int32 optName, const void *optVal, __int32 optLen)
{
	return conn_bind_sock->SetSockOpt(level, optName, optVal, optLen);
}

BOOL __fastcall c_sock_compl::GetSockName()
{
	return conn_bind_sock->GetSockName();
}

BOOL __fastcall c_sock_compl::GetHostName(char *sbuf, __int32 sbuflen)
{
	return conn_bind_sock->GetHostName(sbuf, sbuflen);
}

hostent * __fastcall c_sock_compl::GetHostInformation(char *hostname)
{
	return conn_bind_sock->GetHostInformation(hostname);
}