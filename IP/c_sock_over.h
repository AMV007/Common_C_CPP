/*
 * c_sock_over class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

/*
Windows extended class for working with TCPIP sockets
based on windows WSA extended functions
*/

#pragma once 

#ifndef __H_C_SOCK_OVER__
#define __H_C_SOCK_OVER__

#include "c_sock_base.h"

#define C_SOCK_ERROR_ASYNC_TRANSFER_RECV_OVER		(C_SOCK_OVER_BASE_ERROR_NUM  -1 )
#define C_SOCK_ERROR_ASYNC_TRANSFER_RECVANY_OVER	(C_SOCK_OVER_BASE_ERROR_NUM  -2 )
#define C_SOCK_ERROR_ASYNC_TRANSFER_SEND_OVER		(C_SOCK_OVER_BASE_ERROR_NUM  -3 )

#define C_SOCK_ERROR_ASYNC_TRANSFER_RECV_SZ			(C_SOCK_OVER_BASE_ERROR_NUM  -4 )
#define C_SOCK_ERROR_ASYNC_TRANSFER_RECVANY_SZ		(C_SOCK_OVER_BASE_ERROR_NUM  -5 )
#define C_SOCK_ERROR_ASYNC_TRANSFER_SEND_SZ			(C_SOCK_OVER_BASE_ERROR_NUM  -6 )

#define C_SOCK_ERROR_ASYNC_TRANSFER_RECV_PEN		(C_SOCK_OVER_BASE_ERROR_NUM  -7 )
#define C_SOCK_ERROR_ASYNC_TRANSFER_RECVANY_PEN		(C_SOCK_OVER_BASE_ERROR_NUM  -8 )
#define C_SOCK_ERROR_ASYNC_TRANSFER_SEND_PEN		(C_SOCK_OVER_BASE_ERROR_NUM  -9 )

class c_sock_over: public c_sock_base
{	
private:
	WSABUF bufr,bufw; 
	WSAOVERLAPPED ovr, ovw;	

	WSAEVENT BlockEvent;	// event fot blocking option

	BOOL __fastcall TestEvents();		

public:
	c_sock_over(void);
	~c_sock_over(void);	
	
	BOOL __fastcall Create(	DWORD addr=0, WORD port=80, 
											char* addrName=NULL, BOOL NonBlockingConn = false,
											int protocol=IPPROTO_TCP, 
											int s_type=SOCK_STREAM);											

	BOOL __fastcall Connect_s();
    BOOL __fastcall Accept_s(SOCKET ListenSocket);

	INT __fastcall Recv(BYTE *buf, DWORD size, DWORD timeout=INFINITE);
    __int32 __fastcall RecvAny(BYTE *buf, DWORD size);
    __int32 __fastcall Send(BYTE *buf, DWORD size, DWORD timeout=INFINITE);
	
	BOOL __fastcall Close(void);		
};

#endif __H_C_SOCK_OVER__