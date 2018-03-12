/*
 * c_sock_over class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/
#include "stdafx.h"
#include "c_sock_over.h"

c_sock_over::c_sock_over(void)
{
	memset(&ovr, 0, sizeof(WSAOVERLAPPED));
    memset(&ovw, 0, sizeof(WSAOVERLAPPED));

	ovr.hEvent=WSACreateEvent();
	ovw.hEvent=WSACreateEvent();
	BlockEvent=WSA_INVALID_EVENT;	     
}

c_sock_over::~c_sock_over(void)
{
	Close();

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

	if(BlockEvent!=WSA_INVALID_EVENT)
	{
		WSACloseEvent(BlockEvent);
		BlockEvent=WSA_INVALID_EVENT;
	}	
}

BOOL __fastcall c_sock_over::TestEvents()
{
	if(	ovr.hEvent==WSA_INVALID_EVENT||
		ovw.hEvent==WSA_INVALID_EVENT||
		(nonBlocking&&(BlockEvent==WSA_INVALID_EVENT))) return false;

	if(	ovr.hEvent==NULL||
		ovw.hEvent==NULL||
		(nonBlocking&&(BlockEvent==NULL))) return false;
	
	return true;
}

BOOL __fastcall c_sock_over::Close(void)
{
	c_sock_base::Close(); // base member function

	DWORD flag=0, sz=0;

	if(ovr.hEvent!=WSA_INVALID_EVENT)
	{
		WSASetEvent(ovr.hEvent);
		if(!WSAGetOverlappedResult(sock, &ovr, &sz, FALSE, &flag))
		{
			CancelIo((HANDLE)sock);
		}
		WSAResetEvent(ovr.hEvent);
	}

	if(ovw.hEvent!=WSA_INVALID_EVENT)
	{
		WSASetEvent(ovw.hEvent);
		if(!WSAGetOverlappedResult(sock, &ovw, &sz, FALSE, &flag))
		{
			CancelIo((HANDLE)sock);
		} 
		WSAResetEvent(ovw.hEvent);
	}

	if(BlockEvent!=WSA_INVALID_EVENT)
	{
		WSAEventSelect(sock,BlockEvent,0); // disable block event
		WSASetEvent(BlockEvent);		
		WSAResetEvent(BlockEvent);
	}
	return true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall c_sock_over::Create(DWORD addr, WORD port,
											char* addrName, 
											BOOL NonBlockingConn,
											int protocol, 
											int s_type)
{
	if(!Create_Internal(addr, port,addrName,&protocol,&s_type)) return false;

	
	sock=WSASocket(AF_INET, s_type,protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(sock==INVALID_SOCKET)
	{
		c_sock_over::SetError(C_SOCK_ERROR_CREATE_SOCKET);
		return false;
	}	

	nonBlocking=NonBlockingConn;	
	if(nonBlocking)
	{	
		if(BlockEvent==WSA_INVALID_EVENT) BlockEvent=WSACreateEvent();		
		if(BlockEvent==NULL) return false; // error alloc mem
		
		if(WSAEventSelect(sock,BlockEvent,FD_CONNECT)!=0)
        {            
            return false;
        }
	}	

	Initialized=true;
	return true;
}

BOOL __fastcall c_sock_over::Connect_s()
{
	if(!Initialized||!TestEvents())return false;

    bool res=false;

	DWORD begintime=GetTickCount();
	do
	{
		if (WSAConnect(sock, (sockaddr*)&sock_addr, sizeof(sockaddr), NULL, NULL, NULL, NULL)==SOCKET_ERROR)
		{
			int Value=WSAGetLastError();

			if(nonBlocking)
			{ // blocking socket, we must wait until connection
                if(Value==WSAEWOULDBLOCK)
                {
                    if(WSAWaitForMultipleEvents(1, &BlockEvent, TRUE, connectTimeout_ms, FALSE)==WSA_WAIT_EVENT_0)
                    {
                        res=true;
                    }
                    else c_sock_over::SetError(C_SOCK_ERROR_NON_BLOCK_CONNECT_WAIT);
                }
                else if (Value==WSAENOBUFS)
                {// system not have ehough resources to open new socket
                    if((GetTickCount()-begintime)<connectTimeout_ms)
                    {
                        Sleep(100);
                        continue;
                    }
                    else c_sock_over::SetError(C_SOCK_ERROR_NON_BLOCK_CONNECT_WAIT2);
                }
                else c_sock_over::SetError(C_SOCK_ERROR_NON_BLOCK_CONNECT);
				break;
			}

			if (!(Value==WSAENETUNREACH||Value==WSAECONNREFUSED))
			{
				c_sock_over::SetError(C_SOCK_ERROR_CONNECT_CLIENT_SOCKET);
				break;
			}
			Sleep(10);
		}
		else
        {
            res=true;
            break;
        }

        if((GetTickCount()-begintime)>=connectTimeout_ms)
        {
            c_sock_over::SetError(C_SOCK_ERROR_CONNECT_TIMEOUT);
            break;
        }
	}while (true);

    if(res)
    {
        Socket_Status|=t_Connect_Socket;
    }
    else
    {
    	c_sock_over::SetError(C_SOCK_ERROR_CONNECT_CLIENT_SOCKET);
    	Close();
    }

    if(nonBlocking) WSAEventSelect(sock,BlockEvent,0); // disable nonblock

    return res;
}

BOOL __fastcall c_sock_over::Accept_s(SOCKET ListenSocket)
{	
	if(ListenSocket==INVALID_SOCKET||!TestEvents()) return false;

	Close();

	int accept_peer_size=sizeof(SOCKADDR_IN);
    bool res=false;
	DWORD begintime=GetTickCount();
	do
	{
		//p_accept.sock=accept(sock, (sockaddr*)&p_accept.peer, &accept_peer_size);
		sock= WSAAccept(ListenSocket, (sockaddr*)&sock_addr, &accept_peer_size,NULL,NULL);
		if(sock==INVALID_SOCKET)
		{
			int Value=WSAGetLastError();
			if (!(Value==WSAECONNRESET
				||Value==WSAEMFILE
				))
			{
				break;
			}
			Sleep(10);
		}
        else 
        {
			res=true;
            break;
        }
	}while ((GetTickCount()-begintime)<acceptTimeout_ms);

   	if(res)
    {
        if(nonBlocking)
        {
            nonBlocking=0;
            ioctlsocket(sock, FIONBIO, &nonBlocking); // disable blocking
        }
        Socket_Status|=t_Accept_Socket;
    }
    else
    {
        c_sock_over::SetError(C_SOCK_ERROR_ACCEPT_SOCKET);
    	Close();
    }
    return res;
}

INT __fastcall c_sock_over::Recv(BYTE *buf, DWORD size, DWORD timeout)
{
	if (sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		c_sock_over::SetError(C_SOCK_ERROR_RECV_PARAM);
		return -1;
	}

	while (ReadInUse)Sleep(5);
	ReadInUse=true;

	DWORD SizeRecieved=0;
	DWORD tbegin = GetTickCount();
    do
	{
		bufr.len=size;
		bufr.buf=(PCHAR)buf;

		DWORD sz=0;
		DWORD flag=0;
		int res=WSARecv(sock, &bufr, 1, &sz, &flag, &ovr, NULL);
		if(res==SOCKET_ERROR)
		{
			if (WSAGetLastError()==WSA_IO_PENDING)
			{
				if(WSAWaitForMultipleEvents(1, &ovr.hEvent, TRUE, timeout, FALSE)==WSA_WAIT_EVENT_0)
				{
					flag=0;
					if (!WSAGetOverlappedResult(sock, &ovr, &sz, FALSE, &flag))
					{
                        c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_RECV_OVER);
						Close();
						break;
					}
				}
				else
				{
					CancelIo((HANDLE)sock);
					flag=0;
					WSAGetOverlappedResult(sock, &ovr, &sz, FALSE, &flag);
				}
			}
			else
			{
                c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_RECV_PEN);
				Close();
				break;
			}
		}
		else if (res==0&&sz==0)
		{
            c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_RECV_SZ);
			Close();
			break;
		}
		size-=sz;
		SizeRecieved+=sz;
		buf+=sz;
	}while(size&&((GetTickCount()-tbegin)<timeout));

	ReadInUse=false;
	return SizeRecieved;
}
INT __fastcall c_sock_over::RecvAny(BYTE *buf, DWORD size)
{
	if (sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		c_sock_over::SetError(C_SOCK_ERROR_RECVANY_PARAM);
		return -1;
	}
	while (ReadInUse)Sleep(5);
	ReadInUse=true;

	bufr.len=size;
	bufr.buf=(PCHAR)buf;

	DWORD RecvSize=0;
	DWORD flag=0;
	int res=WSARecv(sock, &bufr, 1, &RecvSize, &flag, &ovr, NULL);
	if(res==SOCKET_ERROR)
	{
		if (WSAGetLastError()==WSA_IO_PENDING)
		{
			if(WSAWaitForMultipleEvents(1, &ovr.hEvent, TRUE, INFINITE, FALSE)==WSA_WAIT_EVENT_0)
			{
				flag=0;
				if (!WSAGetOverlappedResult(sock, &ovr, &RecvSize, FALSE, &flag))
				{
                    c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_RECVANY_OVER);
					Close();
				}
			}
			else
			{
				CancelIo((HANDLE)sock);
				flag=0;
				WSAGetOverlappedResult(sock, &ovr, &RecvSize, FALSE, &flag);
			}
		}
		else
		{
            c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_RECVANY_PEN);
			Close();
		}
	}
	else if (res!=0)
	{
        c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_RECVANY_SZ);
		Close();
	}


	ReadInUse=false;
	return RecvSize;
}
INT __fastcall c_sock_over::Send(BYTE *buf, DWORD size, DWORD timeout)
{
	if (sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		c_sock_over::SetError(C_SOCK_ERROR_SEND_PARAM);
		return -1;
	};

	while (WriteInUse)Sleep(5);
	WriteInUse=true;

	bufw.len=size;
	bufw.buf=(PCHAR)buf;

	DWORD sz=0;
	int res=WSASend(sock, &bufw, 1, &sz, 0, &ovw, NULL);
	if(res==SOCKET_ERROR)
	{
		if (WSAGetLastError()==WSA_IO_PENDING)
		{
			DWORD flgs=0;
			if(WSAWaitForMultipleEvents(1, &ovw.hEvent, FALSE, timeout, FALSE)==WSA_WAIT_EVENT_0)
			{
				if (!WSAGetOverlappedResult(sock, &ovw, &sz, FALSE, &flgs))
				{
                    c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_SEND_OVER);
					Close();
				}
			}
			else
			{
				CancelIo((HANDLE)sock);
				WSAGetOverlappedResult(sock, &ovw, &sz, FALSE, &flgs);
			}
		}
		else
		{
            c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_SEND_PEN);
			Close();
		}
	}
	else if (res==0&&sz==0)
	{
        c_sock_over::SetError(C_SOCK_ERROR_ASYNC_TRANSFER_SEND_SZ);
		Close();
	};

	WriteInUse=false;

	return sz;
}