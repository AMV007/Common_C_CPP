/*
 * c_sock_base class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

//#ifdef  _MSC_VER
#include "stdafx.h"
//#endif
#include "c_sock_base.h"
#include <stdlib.h>
#include "..\\COMMON\\Compatability.h"


c_sock_base::c_sock_base(void)
{
	sock=INVALID_SOCKET; // for close
	Close();

	connectTimeout_ms=
		bindTimeout_ms=
		acceptTimeout_ms=C_SOCK_DEF_CON_BIND_ACC_TIMEOUT;

	localHostName=NULL;
	localDomainInfo=NULL;

	c_sock_base::ClearError();	
	InitializeWSASystem();
}

c_sock_base::~c_sock_base(void)
{
	Close();
	if(localHostName!=NULL)
	{
		delete localHostName;
		localHostName=NULL;
	}
	if(localDomainInfo!=NULL) 
	{
		delete localDomainInfo;
		localDomainInfo=NULL;
	}
	//localHostInfo - not delete !
}

BOOL __fastcall c_sock_base::Close()
{
	if(sock!=INVALID_SOCKET)
	{
		//BOOL SockOptDntLinger=true;
		//SetSockOpt(SOL_SOCKET,SO_DONTLINGER,&SockOptDntLinger,sizeof(SockOptDntLinger));

		shutdown(sock, SD_BOTH);
		closesocket(sock);
		sock=INVALID_SOCKET;
	}

	Initialized=false;
	ReadInUse=WriteInUse=false;
	nonBlocking=0;
	Socket_Status=t_Unknown_Socket;

	return true;
}


//---------------------------------------------------------------------------
// WSA system
//---------------------------------------------------------------------------
volatile long c_sock_base::WSAInit=0;
BOOL c_sock_base::InitializeWSASystem(void)
{
	if(WSAInit) return false;
#ifdef _WIN32
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
    InterlockedIncrement(&WSAInit);
#else
    WSAInit = true;
#endif
	
	return true;
}

BOOL c_sock_base::ReleaseWSASystem(void)
{
	long InternalStartupProcessed=0;
#ifdef _WIN32
	InterlockedExchange(&InternalStartupProcessed,WSAInit);
	InterlockedDecrement(&WSAInit);

	if(InternalStartupProcessed>1)
	{		
		return false;
	}
	if(WSACleanup() != 0) return false;
#endif
	return true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void __fastcall c_sock_base::SetSockTimeout(DWORD ConnectTimeout_ms, DWORD BindTimout_ms, DWORD AcceptTimeout_ms)
{
	connectTimeout_ms=ConnectTimeout_ms;
	bindTimeout_ms=BindTimout_ms;
	acceptTimeout_ms=AcceptTimeout_ms;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall c_sock_base::Create_Internal(DWORD addr, WORD port,
											 char* addrName,
											 __int32 *protocol, 
											 __int32 *s_type)
{
	// сначала Close а потом c_sock_base::SetError
	Close();
	c_sock_base::ClearError();
	memset(&sock_addr,0,sizeof(SOCKADDR_IN));

	if(addrName) {

		bool addrameIsIP = false;

		DWORD AddrIP=atoi(addrName);
		if(AddrIP>0&&AddrIP<=255)
		{
			AddrIP<<=24;
			int IPSegCount=1;
			int addrlen=strlen(addrName);
			int LastPsetPos=0;
			for (int i = 0; i <addrlen; i++)
			{
				if(addrName[i]!='.') continue;

				if((i-LastPsetPos)>3) break;

				int TempRes=atoi(&addrName[i+1]);
				if(TempRes>255||TempRes<0) break;

				AddrIP|=(TempRes<<((4-(++IPSegCount))*8));
				LastPsetPos=i+1;

				if(IPSegCount==4)
				{
					addrameIsIP=true;
					break;
				}
			}
		}


		if(addrameIsIP)
		{
			sock_addr.sin_addr.s_addr=htonl(AddrIP);
		}
		else
		{
			// Get the server's Internet address
			hostent * hostnm = gethostbyname(addrName);
			if(hostnm == NULL) {
				c_sock_base::SetError(C_SOCK_ERROR_GET_HOST_NAME_INIT);
				return false;
			}
			// Put the server information into the client structure.
			sock_addr.sin_addr.s_addr = *((unsigned long *)hostnm->h_addr);
		}
	}
	else
	{
		if(addr==0)
		{
			sock_addr.sin_addr.s_addr = INADDR_ANY; // Use my IP address
		}
	}

	sock_addr.sin_family=AF_INET;
	sock_addr.sin_port=htons(port);

	//----------------

	if(*protocol==IPPROTO_TCP||*protocol==IPPROTO_IP)
	{
		*s_type=SOCK_STREAM;
	}
	else if (*protocol==IPPROTO_UDP)
	{
		*s_type=SOCK_DGRAM;
	}
	else
	{
		*s_type=SOCK_STREAM;
	}	

	return true;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL __fastcall c_sock_base::Create(DWORD addr, WORD port,
									char* addrName, 
									BOOL NonBlockingConn,
									int protocol, 
									int s_type)
{
	if(!Create_Internal(addr, port,addrName,&protocol,&s_type)) return false;


	sock=socket(AF_INET, s_type,protocol);
	if(sock==INVALID_SOCKET)
	{
		c_sock_base::SetError(C_SOCK_ERROR_CREATE_SOCKET);
		return false;
	}	

	nonBlocking=NonBlockingConn;
	if(nonBlocking)
	{	// default - blocking, but 		
		// If iMode = 0, blocking is enabled; 
		// If iMode != 0, non-blocking mode is enabled.				
		if(ioctlsocket(sock, FIONBIO, &nonBlocking)!=0)
		{
			c_sock_base::SetError(C_SOCK_ERROR_SET_BLOCK_PARAM);			
			return false;
		}		
	}	

	Initialized=true;
	return true;
}

BOOL __fastcall c_sock_base::Connect_s()
{
	if(!Initialized)return false;
	bool res=false;

	DWORD begintime=GetTickCount();
	while (true)
	{
		if (connect(sock, (sockaddr*)&sock_addr, sizeof(sockaddr))==SOCKET_ERROR)
		{
			int Value=WSAGetLastError();
			if(nonBlocking)
			{ // blocking socket, we must wait until connection
				if(Value==WSAEWOULDBLOCK)
				{
					if(WaitForWriteData(connectTimeout_ms))
					{
						res=true;
					}
					else c_sock_base::SetError(C_SOCK_ERROR_NON_BLOCK_CONNECT_WAIT);
				}
				else if (Value==WSAENOBUFS)
				{// system not have ehough resources to open new socket
					if((GetTickCount()-begintime)<connectTimeout_ms)
					{
						Sleep(100);
						continue;
					}
					else c_sock_base::SetError(C_SOCK_ERROR_NON_BLOCK_CONNECT_WAIT2);
				}
				else c_sock_base::SetError(C_SOCK_ERROR_NON_BLOCK_CONNECT);

				break;
			}

			if (!(Value==WSAENETUNREACH||Value==WSAECONNREFUSED))
			{
				c_sock_base::SetError(C_SOCK_ERROR_CONNECT_CLIENT_SOCKET);
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
			c_sock_base::SetError(C_SOCK_ERROR_CONNECT_TIMEOUT);
			break;
		}
	};


	if(res)
	{
		if(nonBlocking)
		{
			nonBlocking=0;
			ioctlsocket(sock, FIONBIO, &nonBlocking); // disable blocking
		}

		Socket_Status|=t_Connect_Socket;
	}
	else
	{
		c_sock_base::SetError(C_SOCK_ERROR_CONNECT_CLIENT_SOCKET);
		Close();
	}

	return res;
}
BOOL __fastcall c_sock_base::Bind_s()
{
	if(!Initialized)return false;

	bool res=false;
	DWORD begintime=GetTickCount();
	do
	{
		if (bind(sock, (sockaddr*)&sock_addr, sizeof(sock_addr))==SOCKET_ERROR)
		{

			//break;
		}
		else
		{

			res=true;
			break;
		}
	}while ((GetTickCount()-begintime)<bindTimeout_ms);

	if(res)
	{
		if(nonBlocking)
		{
			nonBlocking=0;
			ioctlsocket(sock, FIONBIO, &nonBlocking); // disable blocking
		}
		Socket_Status|=t_Bind_Socket;
	}
	else
	{
		c_sock_base::SetError(C_SOCK_ERROR_BIND_SERVER_SOCKET);
		Close();
	}
	return res;
}
BOOL __fastcall c_sock_base::Accept_s(SOCKET ListenSocket)
{
	if(ListenSocket==INVALID_SOCKET) return false;

	Close();

	bool res=false;
	int accept_peer_size=sizeof(SOCKADDR_IN);

	DWORD begintime=GetTickCount();
	do
	{

		//p_accept.sock=accept(sock, (sockaddr*)&p_accept.peer, &accept_peer_size);
		sock= accept(ListenSocket, (sockaddr*)&sock_addr, &accept_peer_size);
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
		c_sock_base::SetError(C_SOCK_ERROR_ACCEPT_SOCKET);
		Close();
	}
	return res;
}

INT __fastcall c_sock_base::Recv(BYTE *buf, DWORD size, DWORD timeout)
{
	if (sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		c_sock_base::SetError(C_SOCK_ERROR_RECV_PARAM);
		return -1;
	}

	while (ReadInUse)Sleep(5);
	ReadInUse=true;

	DWORD SizeRecieved=0;
	DWORD tbegin = GetTickCount();
	while(size&&((GetTickCount()-tbegin)<timeout))
	{		
		int res=recv(sock, (char *)buf, size,0);
		if(res>0)
		{
			size-=res;
			SizeRecieved+=res;
			buf+=res;

		}
		else if (res==0)
		{
			if(res==0)c_sock_base::SetError(C_SOCK_ERROR_RECV_SZ);
			break;
		}
		else //if(res<0)
		{
			c_sock_base::SetError(C_SOCK_ERROR_RECV);
			Close();
			break;
		}					
	}


	ReadInUse=false;
	return SizeRecieved;
}

INT __fastcall c_sock_base::RecvAny(BYTE *buf, DWORD size)
{
	if (sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		c_sock_base::SetError(C_SOCK_ERROR_RECVANY_PARAM);
		return -1;
	}

	while (ReadInUse)Sleep(5);
	ReadInUse=true;	

	int res=recv(sock, (char *)buf, size,0);
	if(res<0)
	{
		c_sock_base::SetError(C_SOCK_ERROR_RECVANY);
		Close();
	}				

	if(res==0)c_sock_base::SetError(C_SOCK_ERROR_RECVANY_SZ);

	ReadInUse=false;
	return res;
}

INT __fastcall c_sock_base::Send(BYTE *buf, DWORD size, DWORD timeout)
{
	if (sock==INVALID_SOCKET)
	{
		Sleep(1); // чтобы проц не загружался при частом вызове
		c_sock_base::SetError(C_SOCK_ERROR_SEND_PARAM);
		return -1;
	};

	while (WriteInUse)Sleep(5);
	WriteInUse=true;

	WaitForWriteData(timeout); // wait for socket ready for write data

	int res=send(sock, (char *)buf, size,0);
	if(res<0)
	{
		c_sock_base::SetError(C_SOCK_ERROR_SEND);
		Close();
	}	

	if(res==0)c_sock_base::SetError(C_SOCK_ERROR_SEND_SZ);

	WriteInUse=false;

	return res;
}

//---------------------------------------------------------------------------
// Function used to multiplex reads without polling. Returns false if a
// reply time is longer then the timeout values.
//---------------------------------------------------------------------------

BOOL __fastcall c_sock_base::WaitForWriteData(int timeout_ms)
{	
	if(sock == INVALID_SOCKET) return 0;	//CWS - socket was disconnected?

	struct timeval timeout;
	timeout.tv_sec = timeout_ms/1000;
	timeout.tv_usec = timeout_ms%1000;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	// This function calls select() giving it the file descriptor of
	// the socket. The kernel reports back to this function when the file
	// descriptor has woken it up.
	//CWS  return select(s+1, &fds, 0, 0, &timeout);

	int res=select(sock+1, 0, &fds, 0, &timeout);
	if(res==1)
	{
		return true;
	}
	if(res==0) c_sock_base::SetError(C_SOCK_ERROR_BLOCK_WRITE_TIMEOUT);	

	return false;
}

BOOL __fastcall c_sock_base::WaitForRecieveData(int timeout_ms)
{
	if(sock == INVALID_SOCKET) return 0;	//CWS - socket was disconnected?

	struct timeval timeout;
	timeout.tv_sec = timeout_ms/1000;
	timeout.tv_usec = timeout_ms%1000;

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	// This function calls select() giving it the file descriptor of
	// the socket. The kernel reports back to this function when the file
	// descriptor has woken it up.
	//CWS  return select(s+1, &fds, 0, 0, &timeout);

	int res=select(sock+1, &fds, 0, 0, &timeout);
	if(res==1) return true;	
	if(res==0) c_sock_base::SetError(C_SOCK_ERROR_BLOCK_READ_TIMEOUT);	

	return false;
}

//---------------------------------------------------------------------------
// Sets the current socket option for the specified option level or name.
//---------------------------------------------------------------------------
BOOL __fastcall c_sock_base::SetSockOpt(int level, __int32 optName,
										const void *optVal,  __int32 optLen)
{
	if (sock==INVALID_SOCKET) return false;

	if(setsockopt(sock, level, optName, (const char *)optVal,
		optLen)!=0)
	{
		c_sock_base::SetError(C_SOCK_ERROR_SET_SOCK_OPT);
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------
// Retrieves the current name for this objects socket descriptor.
//---------------------------------------------------------------------------
BOOL __fastcall c_sock_base::GetSockName()
{
	if (sock==INVALID_SOCKET) return false;

	int namelen = sizeof(sockaddr);
	if(getsockname(sock, (struct sockaddr *)&sock_addr, &namelen)!=0)
	{
		c_sock_base::SetError(C_SOCK_ERROR_GET_SOCK_NAME);
	}
	return true;
}

//---------------------------------------------------------------------------
// Listen for connections if configured as a server.
// The "max_connections" variable determines how many
// pending connections the queue will hold. Returns -1
// if an error occurs.
//---------------------------------------------------------------------------
BOOL __fastcall c_sock_base::Listen_s(int max_connections)
{
	if (sock==INVALID_SOCKET) return false;

	if(listen(sock, max_connections)!=0)
	{
		c_sock_base::SetError(C_SOCK_ERROR_LISTEN_SOCKET);
		return false;
	}
	else
	{
		Socket_Status|=t_Listen_Socket;
	}
	return true;
}

//---------------------------------------------------------------------------
// Pass back the host name of this machine in the "sbuf" variable.
//---------------------------------------------------------------------------
BOOL __fastcall c_sock_base::GetHostName(char *sbuf, __int32 sbuflen)
{
	if(sbuf==NULL) return false;

	if(gethostname(sbuf, sbuflen)!=0)
	{
		c_sock_base::SetError(C_SOCK_ERROR_GET_HOST_NAME);
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
hostent * __fastcall c_sock_base::GetHostInformation(char *hostname)
{
	if(hostname==NULL)
	{   // for localhost - optimization
		// (we are not planning, that local host name will be change during programm running)
		if(localHostName==NULL)
		{
			localHostName = new char[MAX_HOST_NAME_LEN];
			if(localHostName!=NULL)
			{
				memset(localHostName,0,MAX_HOST_NAME_LEN);
				if(!GetHostName(localHostName, MAX_HOST_NAME_LEN)) return NULL;
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

	if(!hostinfo) c_sock_base::SetError(C_SOCK_ERROR_GET_HOST_INFO);

	return hostinfo;
}

//------------------------------------------------------------------------
// Pass back the domain name of this machine 
// Return NULL if an error occurs.
// do not delete return value, it will be deleted in class delete
//-----------------------------------------------------------------------
char * __fastcall c_sock_base::GetDomainName()
{  
	if(localDomainInfo!=NULL) return localDomainInfo;
	
	localDomainInfo=new char[MAX_DOMAIN_NAME_LEN];
	if(localDomainInfo==NULL) return NULL;			
	
	hostent *hostinfo = GetHostInformation(NULL); // host information from localhost
	if(!hostinfo)  return NULL; 	

	strcpy_loc(localDomainInfo, MAX_DOMAIN_NAME_LEN, hostinfo->h_name);
	int i; __int32 len = strlen(localDomainInfo);
	for(i = 0; i < len; i++) {
		if(localDomainInfo[i] == '.') break;
	}
	if(++i < len) {
		len -= i;
		memmove(localDomainInfo, localDomainInfo+i, len);
		localDomainInfo[len] = 0; // Null terminate the string
	}
	
	return localDomainInfo;
}
