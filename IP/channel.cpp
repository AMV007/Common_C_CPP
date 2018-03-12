//#ifdef  _MSC_VER
#include "stdafx.h"
//#endif
#include "channel.h"
#include <algorithm>
// deprecated class !!!!!!!!!!!!!!!!!!!! , use instead it c_sock_base
//---------------------------------------------------------------------------
//                        tchannel constructor
//---------------------------------------------------------------------------

TChannel::TChannel(void)
{
	p_accept.Init();
	p_connect_bind.Init();

    SetLastError_s(CHANNEL_NO_ERROR);

	localHostName=NULL;
	localHostInfo=NULL;

	InitializeWSASystem();
}
//---------------------------------------------------------------------------
//                       tchannel destructor
//---------------------------------------------------------------------------
TChannel::~TChannel(void)
{
	Close();

    p_accept.FreeResources();
    p_connect_bind.FreeResources();

	if(localHostName!=NULL)
	{
		delete localHostName;
	}
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
long TChannel::StartupProcessed=0;
BOOL TChannel::InitializeWSASystem(void)
{
	if(StartupProcessed) return false;

	WSADATA wsaData;
	if (!WSAStartup(MAKEWORD( 2, 2 ), &wsaData))
	{
		if (LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE( wsaData.wVersion ) != 2 )
		{
			// Tell the user that we could not find a usable
			// WinSock DLL.                                  */
			WSACleanup( );
			return false;
		}
	}
	else
	{
		return false;
	}

	InterlockedIncrement(&StartupProcessed);
	return true;
}

BOOL TChannel::ReleaseWSASystem(void)
{
	long InternalStartupProcessed=0;
	InterlockedExchange(&InternalStartupProcessed,StartupProcessed);
	InterlockedDecrement(&StartupProcessed);

	if(InternalStartupProcessed>1)
	{		
		return false;
	}
	if(WSACleanup() != 0) return false;
	return true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

BOOL __fastcall TChannel::CloseAccept()
{
	p_accept.Close();

	return true;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::Close(void)
{
	CloseAccept();

	p_connect_bind.Close();

	return true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::InitSocket_con_bind(DWORD addr, WORD port, char* addrName, __int32 protocol, __int32 s_type)
{
	// сначала Close а потом SetLastError
	Close();
	SetLastError_s(CHANNEL_NO_ERROR);
    memset(&p_connect_bind.peer,0,sizeof(SOCKADDR_IN));

	if(addrName) {
		// Get the server's Internet address
		hostent * hostnm = gethostbyname(addrName);
		if(hostnm == NULL) {
			SetLastError_s(CHANNEL_ERROR_GET_HOST_NAME_INIT);
			return false;
		}
		// Put the server information into the client structure.
		p_connect_bind.peer.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
	}
	else
	{
		if(addr==0)
		{
			p_connect_bind.peer.sin_addr.s_addr = INADDR_ANY; // Use my IP address
		}
	}

	p_connect_bind.peer.sin_family=AF_INET;
	p_connect_bind.peer.sin_port=htons(port);


	//----------------

	if(protocol==IPPROTO_TCP||protocol==IPPROTO_IP)
	{
		s_type=SOCK_STREAM;
	}
	else if (protocol==IPPROTO_UDP)
	{
		s_type=SOCK_DGRAM;
	}
	else
	{
		s_type=SOCK_STREAM;
	}

	p_connect_bind.sock=WSASocket(AF_INET, s_type,protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
	if(p_connect_bind.sock==INVALID_SOCKET)
	{
		SetLastError_s(CHANNEL_ERROR_CREATE_SOCKET);
		return false;
	}

	p_connect_bind.Initialized=true;
	return true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::Connect_s()
{
	if(!p_connect_bind.Initialized)return false;

	DWORD begintime=GetTickCount();
	while ((GetTickCount()-begintime)<SOCKET_CONNECT_TIMEOUT_MS)
	{
		if (WSAConnect(p_connect_bind.sock, (sockaddr*)&p_connect_bind.peer, sizeof(sockaddr), NULL, NULL, NULL, NULL)==SOCKET_ERROR)
		{
			int Value=WSAGetLastError();
			if (!(Value==WSAETIMEDOUT||Value==WSAENETUNREACH||Value==WSAECONNREFUSED))
			{
				SetLastError_s(CHANNEL_ERROR_CONNECT_CLIENT_SOCKET);
				break;
			}
			Sleep(2);
		}
        else if(p_connect_bind.CreateEvent_s())
        {
            return true;
        }
	}

    Close();
    return false;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::Bind_s()
{
	if(!p_connect_bind.Initialized)return false;

	DWORD begintime=GetTickCount();
	while ((GetTickCount()-begintime)<SOCKET_CONNECT_TIMEOUT_MS)
	{
		if (bind(p_connect_bind.sock, (sockaddr*)&p_connect_bind.peer, sizeof(p_connect_bind.peer))==SOCKET_ERROR)
		{
			SetLastError_s(CHANNEL_ERROR_BIND_SERVER_SOCKET);
			break;
		}
        else if(p_connect_bind.CreateEvent_s())
        {
            return true;
        }
	}

    Close();
    return false;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::Accept_s()
{
    if(p_connect_bind.sock==INVALID_SOCKET) return false;

	CloseAccept();
    p_accept.Init();

	int accept_peer_size=sizeof(SOCKADDR_IN);

	DWORD begintime=GetTickCount();
	while (p_accept.sock==INVALID_SOCKET&&((GetTickCount()-begintime)<SOCKET_ACCEPT_TIMEOUT_MS))
	{
		//p_accept.sock=accept(p_connect_bind.sock, (sockaddr*)&p_accept.peer, &accept_peer_size);
		p_accept.sock= WSAAccept(p_connect_bind.sock, (sockaddr*)&p_accept.peer, &accept_peer_size,NULL,NULL);
		if(p_accept.sock==INVALID_SOCKET)
		{
			int Value=WSAGetLastError();
			if (!(Value==WSAECONNRESET
				||Value==WSAEMFILE
				))
			{
				SetLastError_s(CHANNEL_ERROR_ACCEPT_SOCKET);
				break;
			}
			Sleep(2);
		}
        else
        {
            p_accept.CreateEvent_s();
            return true;
        }
	}

    CloseAccept();
    return false;
}
//---------------------------------------------------------------------------
// поскольку может быть разрыв соединения
//---------------------------------------------------------------------------
INT __fastcall TChannel::RecvUni(s_SockParam * SockParam, BYTE *buf, DWORD size, DWORD timeout)
{
	if (SockParam->sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_SOCKET_RECV);
		return -1;
	}

	while (SockParam->ReadInUse)Sleep(5);
	SockParam->ReadInUse=true;

	DWORD SizeRecieved=0;
	DWORD tbegin = GetTickCount();
	while(size&&((GetTickCount()-tbegin)<timeout))
	{
		SockParam->bufr.len=size;
		SockParam->bufr.buf=(PCHAR)buf;

		DWORD sz=0;
		DWORD flag=0;
		int res=WSARecv(SockParam->sock, &SockParam->bufr, 1, &sz, &flag, &SockParam->ovr, NULL);
		if(res==SOCKET_ERROR)
		{
			if (WSAGetLastError()==WSA_IO_PENDING)
			{
				if(WSAWaitForMultipleEvents(1, &SockParam->ovr.hEvent, TRUE, timeout, FALSE)==WSA_WAIT_EVENT_0)
				{
					flag=0;
					if (!WSAGetOverlappedResult(SockParam->sock, &SockParam->ovr, &sz, FALSE, &flag))
					{
                        SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_RECV_OVER);
						Close();
						break;
					}
				}
				else
				{
					CancelIo((HANDLE)SockParam->sock);
					flag=0;
					WSAGetOverlappedResult(SockParam->sock, &SockParam->ovr, &sz, FALSE, &flag);
				}
			}
			else
			{
                SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_RECV_PEN);
				Close();
				break;
			}
		}
		else if (res==0&&sz==0)
		{
            SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_RECV_SZ);
			Close();
			break;
		}
		size-=sz;
		SizeRecieved+=sz;
		buf+=sz;
	}

	SockParam->ReadInUse=false;
	return SizeRecieved;
}

//---------------------------------------------------------------------------
// для непостоянного размера
//---------------------------------------------------------------------------
INT __fastcall TChannel::RecvAnyUni(s_SockParam * SockParam, BYTE *buf, DWORD size)
{
	if (SockParam->sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_SOCKET_RECVANY);
		return -1;
	}
	while (SockParam->ReadInUse)Sleep(5);
	SockParam->ReadInUse=true;

	SockParam->bufr.len=size;
	SockParam->bufr.buf=(PCHAR)buf;

	DWORD RecvSize=0;
	DWORD flag=0;
	int res=WSARecv(SockParam->sock, &SockParam->bufr, 1, &RecvSize, &flag, &SockParam->ovr, NULL);
	if(res==SOCKET_ERROR)
	{
		if (WSAGetLastError()==WSA_IO_PENDING)
		{
			if(WSAWaitForMultipleEvents(1, &SockParam->ovr.hEvent, TRUE, INFINITE, FALSE)==WSA_WAIT_EVENT_0)
			{
				flag=0;
				if (!WSAGetOverlappedResult(SockParam->sock, &SockParam->ovr, &RecvSize, FALSE, &flag))
				{
                    SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_RECVANY_OVER);
					Close();
				}
			}
			else
			{
				CancelIo((HANDLE)SockParam->sock);
				flag=0;
				WSAGetOverlappedResult(SockParam->sock, &SockParam->ovr, &RecvSize, FALSE, &flag);
			}
		}
		else
		{
            SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_RECVANY_PEN);
			Close();
		}
	}
	else if (res!=0)
	{
        SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_RECVANY_SZ);
		Close();
	}


	SockParam->ReadInUse=false;
	return RecvSize;
}
//---------------------------------------------------------------------------
// Sends Data through the socket
//---------------------------------------------------------------------------
INT __fastcall TChannel::SendUni(s_SockParam * SockParam, BYTE *buf, DWORD size, DWORD timeout)
{
	if (SockParam->sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_SOCKET_SEND);
		return -1;
	};

	while (SockParam->WriteInUse)Sleep(5);
	SockParam->WriteInUse=true;

	SockParam->bufw.len=size;
	SockParam->bufw.buf=(PCHAR)buf;

	DWORD sz=0;
	int res=WSASend(SockParam->sock, &SockParam->bufw, 1, &sz, 0, &SockParam->ovw, NULL);
	if(res==SOCKET_ERROR)
	{
		if (WSAGetLastError()==WSA_IO_PENDING)
		{
			DWORD flgs=0;
			if(WSAWaitForMultipleEvents(1, &SockParam->ovw.hEvent, FALSE, timeout, FALSE)==WSA_WAIT_EVENT_0)
			{
				if (!WSAGetOverlappedResult(SockParam->sock, &SockParam->ovw, &sz, FALSE, &flgs))
				{
                    SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_SEND_OVER);
					Close();
				}
			}
			else
			{
				CancelIo((HANDLE)SockParam->sock);
				WSAGetOverlappedResult(SockParam->sock, &SockParam->ovw, &sz, FALSE, &flgs);
			}
		}
		else
		{
            SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_SEND_PEN);
			Close();
		}
	}
	else if (res==0&&sz==0)
	{
        SetLastError_s(CHANNEL_ERROR_ASYNC_TRANSFER_SEND_SZ);
		Close();
	};

	SockParam->WriteInUse=false;

	return sz;
}

//---------------------------------------------------------------------------
// Function used to multiplex reads without polling. Returns false if a
// reply time is longer then the timeout values.
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::WaitForRecieveDataUni(s_SockParam * SockParam, __int32 seconds, __int32 useconds)
{
	if(SockParam->sock == INVALID_SOCKET) return 0;	//CWS - socket was disconnected?

	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = useconds;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(SockParam->sock, &fds);

	// This function calls select() giving it the file descriptor of
	// the socket. The kernel reports back to this function when the file
	// descriptor has woken it up.
	//CWS  return select(s+1, &fds, 0, 0, &timeout);

	int res=select(SockParam->sock+1, &fds, 0, 0, &timeout);
	if(res!=1)
	{
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------
INT __fastcall TChannel::Recv(BYTE *buf, DWORD size, DWORD timeout)
{
	return RecvUni(&p_connect_bind,buf, size, timeout);
}
INT __fastcall TChannel::RecvAny(BYTE *buf, DWORD size)
{
	return RecvAnyUni(&p_connect_bind,buf, size);
}       
BOOL __fastcall TChannel::WaitForRecieveData(int seconds, __int32 useconds)
{
	return WaitForRecieveDataUni(&p_connect_bind, seconds, useconds);
}
INT __fastcall TChannel::Send(BYTE *buf, DWORD size, DWORD timeout)
{
	return SendUni(&p_connect_bind,buf, size, timeout);
}
//---------------------------------------------------------------------------
INT __fastcall TChannel::Recv_ac(BYTE *buf, DWORD size, DWORD timeout)
{
	return RecvUni(&p_accept,buf, size, timeout);
}
INT __fastcall TChannel::RecvAny_ac(BYTE *buf, DWORD size)
{
	return RecvAnyUni(&p_accept,buf, size);
}
BOOL __fastcall TChannel::WaitForRecieveData_ac(int seconds, __int32 useconds)
{
	return WaitForRecieveDataUni(&p_accept, seconds, useconds);
}
INT __fastcall TChannel::Send_ac(BYTE *buf, DWORD size, DWORD timeout)
{
	return SendUni(&p_accept,buf, size, timeout);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Sets the current socket option for the specified option level or name.
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::SetSockOpt(int level, __int32 optName,
						  const void *optVal,  __int32 optLen)
{
	if (p_connect_bind.sock==INVALID_SOCKET) return false;

	if(setsockopt(p_connect_bind.sock, level, optName, (const char *)optVal,
		optLen)!=0)
	{
		SetLastError_s(CHANNEL_ERROR_SET_SOCK_OPT);
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------
// Retrieves the current name for this objects socket descriptor.
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::GetSockName()
{
	if (p_connect_bind.sock==INVALID_SOCKET) return false;

	int namelen = sizeof(sockaddr);
	if(getsockname(p_connect_bind.sock, (struct sockaddr *)&p_connect_bind.peer, &namelen)!=0)
	{
		SetLastError_s(CHANNEL_ERROR_GET_SOCK_NAME);
	}
	return true;
}

//---------------------------------------------------------------------------
// Listen for connections if configured as a server.
// The "max_connections" variable determines how many
// pending connections the queue will hold. Returns -1
// if an error occurs.
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::Listen(int max_connections)
{
	if (p_connect_bind.sock==INVALID_SOCKET) return false;

	if(listen(p_connect_bind.sock, max_connections)!=0)
	{
		SetLastError_s(CHANNEL_ERROR_LISTEN_SOCKET);
	}
	return true;
}

//---------------------------------------------------------------------------
// Pass back the host name of this machine in the "sbuf" variable.
//---------------------------------------------------------------------------
BOOL __fastcall TChannel::GetHostName(char *sbuf, __int32 sbuflen)
{
	if(sbuf==NULL) return false;

	if(gethostname(sbuf, sbuflen)!=0)
	{
		SetLastError_s(CHANNEL_ERROR_GET_HOST_NAME);
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------
// Function used to obtain hostname information about a specified host. 
// The source of this information is dependent on the calling function's
// platform configuration which should be a DNS, local host table, and/or
// NIS database. Returns a pointer to a gxsHostNameInfo data structure
// if information is available or a null value if the hostname cannot be
// found. NOTE: The calling function must free the memory allocated
// for gxsHostNameInfo data structure upon each successful return.
//---------------------------------------------------------------------------
hostent * __fastcall TChannel::GetHostInformation(char *hostname)
{
	bool GetLocal=false;
	if(hostname==NULL)
	{   // для локальных - оптимизация
		GetLocal=true;
		if(localHostInfo!=NULL)
			return localHostInfo;

		if(localHostName==NULL)
		{
			localHostName = new char[256];
            if(localHostName!=NULL)
            {
			    memset(localHostName,0,256);
			    if(!GetHostName(localHostName, 256)) return NULL;
            }
            else return NULL;
		}
		hostname=localHostName;
	}

	in_addr hostia;
	hostent *hostinfo;

	hostia.s_addr = inet_addr(hostname);

	if(hostia.s_addr == INADDR_NONE) { // Look up host by name
		hostinfo = gethostbyname(hostname);
	}
	else {  // Look up host by IP address
		hostinfo = gethostbyaddr((const char *)&hostia,
			sizeof(IN_ADDR), AF_INET);
	}

	if(GetLocal)
	{ // для локальных - оптимизация
		localHostInfo=hostinfo;
	}

	return hostinfo;
}
