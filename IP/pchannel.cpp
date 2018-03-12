#include "stdafx.h"
#include "pchannel.h"
#include <stdio.h>

#ifdef __BORLANDC__
TPChannel ::TPChannel(void)
{
	CurrHostName="";
	HostNames = new TStringList();	
    ProxySettings.Login="";
    ProxySettings.IP=0;
    ProxySettings.Port=0;
    ProxySettings.ProxyType=PTYPE_NO_PROXY;
    ProxySettings.Pass="";
}
TPChannel ::~TPChannel(void)
{
	Disconnect();
	if(HostNames!=NULL) delete HostNames;	
}

AnsiString __fastcall TPChannel::ConvertToASCII(PCHAR Data, __int32 DataLength)
{
	if(DataLength==0) return "";
	AnsiString res=Data;	
	return res;
}

// RelativePath - выделяем относительный путь от имени хоста, нам нужно только имя хоста
AnsiString __fastcall TPChannel::GetHostName(AnsiString WebAddress, AnsiString * RelativePath)
{
	AnsiString HostName;
	if(RelativePath!=NULL) *RelativePath="/";

	int BeginHost=WebAddress.AnsiPos("//");
	if(BeginHost>0)
	{
		WebAddress =WebAddress.SubString(BeginHost+2, WebAddress.Length()-BeginHost-1);
	}

	int EndHost=WebAddress.AnsiPos("/");
	if(EndHost==0)
	{
		HostName =WebAddress;
	}
	else
	{
		HostName =WebAddress.SubString(0,EndHost-1);
		if(RelativePath!=NULL)
		{
			if(EndHost<WebAddress.Length()-1)
			{
				*RelativePath=WebAddress.SubString(EndHost,WebAddress.Length()-EndHost+1);
			}
		}
	}
	return HostName;
}

void __fastcall TPChannel::SetProxy(s_ProxySettings * NewProxySettings)
{
    ProxySettings.IP=NewProxySettings->IP;
    ProxySettings.Port=NewProxySettings->Port;
    ProxySettings.ProxyType=NewProxySettings->ProxyType;
    ProxySettings.Login=NewProxySettings->Login;
    ProxySettings.Pass=NewProxySettings->Pass;
    if(ProxySettings.IP==(DWORD)-1)ProxySettings.IP=0;
}

BOOL __fastcall TPChannel::ConnectSocks4(AnsiString HostName)
{  
	int Len=ProxySettings.Login.Length()+9;
	PBYTE TempData = new BYTE[Len];
	TempData[0]=PTYPE_SOCKS_VER4;
	TempData[1]=1;
	*(PWORD)(&TempData[2])=htons(DestPort);
	*(PDWORD)(&TempData[4])=htonl(DestIPAddress);
	TempData[8]=0;
	if(ProxySettings.Login!="")
	{
		memcpy(&TempData[8],ProxySettings.Login.c_str(), ProxySettings.Login.Length());
	}
	//Sleep(SOCKS_INTERVAL); // надо ли ?
	if(Send(TempData, Len, 5000))
	{
		memset(TempData, 0 , 8 );							//Set it to 0
		RecvAny(TempData, 8 );		//Get the reply packet

		if ( TempData[0] == 0&&TempData[1] == 90)
		{
			if(TempData!=NULL) delete [] TempData;
			return true;
		}

		switch ( TempData[1] )								//Error codes
		{
		case 91:
			LastError=ERROR_SOCKS4_ERROR1; //SOCKS: request rejected or failed\0
			break;
		case 92:
			LastError=ERROR_SOCKS4_ERROR2; //SOCKS: request rejected because SOCKS server cannot connect to identd on the client\0
			break;
		case 93:
			LastError=ERROR_SOCKS4_ERROR3; //SOCKS: request rejected because the client program and identd report different user-ids\0
			break;
		default:
			LastError=ERROR_SOCKS4_ERROR4; //SOCKS: General Failure or Failed to connect destination server..\0
			break;
		}
	}

	if(TempData!=NULL) delete [] TempData;
	return false;
}

BOOL __fastcall TPChannel::ConnectSocks5(AnsiString HostName)
{
	PBYTE TempData = new BYTE[3];
	int Len=0;

	if(ProxySettings.Login!="")
	{
		TempData[0]=PTYPE_SOCKS_VER5;
		TempData[1]=1;
		TempData[2]=2;
		/* with authentication */
		/*   -> socks server must respond with :
		*       - socks version (buffer[0]) = 5 => socks5
		*       - socks method  (buffer[1]) = 2 => authentication
		*/

		if(Send(TempData, 3, 5000))
		{
			memset(TempData,0,2);
			if(RecvAny(TempData, 2)!=2)
			{
				if(TempData!=NULL) delete [] TempData;
				return false;
			}
			if(TempData[0]==5&&TempData[1]==2)
			{
				Len = ProxySettings.Login.Length() + ProxySettings.Pass.Length() + 3;
				if(TempData!=NULL) delete [] TempData;
				TempData = new BYTE[Len+1];
				memset(TempData,0,Len+1);
				snprintf (TempData, Len + 1, "\x01%c%s%c%s",
					ProxySettings.Login.Length(), ProxySettings.Login.c_str(),
					ProxySettings.Pass.Length(), ProxySettings.Pass.c_str());

				if(Send(TempData, strlen(TempData), 5000))
				{
					memset(TempData,0,2);
					if(RecvAny(TempData, 2)!=2)
					{
						if(TempData!=NULL) delete [] TempData;
						return false;
					}
					if(TempData[0]==1&&TempData[1]==0)
					{

					}
				}
				else
				{
					if(TempData!=NULL) delete [] TempData;
					return false;
				}
			}
			else
			{
				if(TempData!=NULL) delete [] TempData;
				return false;
			}
		}
		else
		{
			if(TempData!=NULL) delete [] TempData;
			return false;
		}
	}
	else
	{
		TempData[0]=PTYPE_SOCKS_VER5;
		TempData[1]=1;
		TempData[2]=0;
		if(Send(TempData, 3, 5000))
		{
			memset(TempData,0,2);
			RecvAny(TempData, 2);		//Get the reply packet
			if(TempData[0]==5&&TempData[1]==0)
			{

			}
			else
			{
				if(TempData!=NULL) delete [] TempData;
				return false;
			}
		}
		else
		{
			if(TempData!=NULL) delete [] TempData;
			return false;
		}
	}

	// сообщаем имя домена
	Len=HostName.Length()+4+1+2;
    
	if(TempData!=NULL) delete [] TempData;
	TempData = new BYTE[Len];

	TempData[0]=PTYPE_SOCKS_VER5;
	TempData[1]=1;
	TempData[2]=0;
	TempData[3]=3;
	TempData[4]=HostName.Length();
	memcpy(&TempData[5],HostName.c_str(), HostName.Length());
	*(PWORD)(&TempData[HostName.Length()+4+1])=htons(DestPort);
	if(Send(TempData, Len, 5000))
	{
		memset(TempData,0,4);
		if(RecvAny(TempData, 4)==4)		//Get the reply packet
		{
			if (!(TempData[0] == 5 && TempData[1] == 0))
			{
				if(TempData!=NULL) delete [] TempData;
				return false;
			}

			switch(TempData[3]) {
				/* buffer[3] = address type */
				case 1 :
					/* ipv4
					* server socks return server bound address and port
					* address of 4 bytes and port of 2 bytes (= 6 bytes)
					*/
					if (RecvAny(TempData, 6)!=6)
					{
						if(TempData!=NULL) delete [] TempData;
						return false;
					}
					break;
				case 3:
					{
						/* domainname
						* server socks return server bound address and port
						*/
						/* reading address length */
						if (RecvAny(TempData, 1)!=1)
						{
							if(TempData!=NULL) delete [] TempData;
							return false;
						}
						int addr_len = TempData[0];
						if(TempData!=NULL) delete [] TempData;
						TempData=new BYTE[addr_len+2];
						/* reading address + port = addr_len + 2 */
						if (RecvAny(TempData, addr_len+2)!=(addr_len+2))
						{
							if(TempData!=NULL) delete [] TempData;
							return false;
						}
					}
					break;
				case 4 :
					/* ipv6
					* server socks return server bound address and port
					* address of 16 bytes and port of 2 bytes (= 18 bytes)
					*/
					if(TempData!=NULL) delete [] TempData;
					TempData=new BYTE[18];
					if (RecvAny(TempData, 18)!=18)
					{
						if(TempData!=NULL) delete [] TempData;
						return false;
					}
					break;
				default:
					if(TempData!=NULL) delete [] TempData;
					return false;
			}
			if(TempData!=NULL) delete [] TempData;
			return true;
		}
	}


	if(TempData!=NULL) delete [] TempData;
	return false;
}


BOOL __fastcall TPChannel::Connect(AnsiString Address, WORD Port, s_ProxySettings * NewProxySettings)
{
    if(NewProxySettings!=NULL) SetProxy(NewProxySettings);

	DestPort=Port;
	BOOL Result=false;

	AnsiString HostName=GetHostName(Address);

	CurrHostName=HostName;

	Disconnect();

	DestIPAddress=0;
	for(int i=0;i<HostNames->Count;i++)
	{
		if(HostNames->Strings[i]==HostName)
		{
			DestIPAddress=hostIP->Items[i];
			break;
		}
	}

	if(DestIPAddress==0)
	{
		hostent * CurrHost = gethostbyname((char *)HostName.data());
		if(CurrHost!=NULL)
		{
			if(CurrHost->h_addr_list[0]>0)
			{
				//const in_addr* address = (in_addr*)host_info->h_addr_list[0] ;
				//std::cout << " address: " << inet_ntoa( *address ) << '\n' ;
				DestIPAddress=ntohl(((in_addr*)CurrHost->h_addr_list[0])->S_un.S_addr);
				HostNames->Add(HostName);
				hostIP->Add((PVOID)DestIPAddress);
			}
			//delete CurrHost;
		}
	}

	if(DestIPAddress==0) return false;

	DWORD IPAddress=DestIPAddress;
	WORD IPPort=DestPort;
	if(ProxySettings.IP!=0)
	{
		IPAddress=ProxySettings.IP;
		IPPort = ProxySettings.Port;
	}

	if(IPAddress!=0)
	{
		Result=Open(IPAddress,IPPort,IPPROTO_TCP);
	}

	if((ProxySettings.ProxyType==PTYPE_SOCKS_VER4)&&Result)
	{ //
		Result=ConnectSocks4(HostName);
	}
	else if((ProxySettings.ProxyType==PTYPE_SOCKS_VER5)&&Result)
	{
		Result=ConnectSocks5(HostName);
	}

    if(!Result)Disconnect();
	return Result;
}

#endif //__BORLANDC__

void __fastcall TPChannel::Disconnect()
{
	Close();	
}

