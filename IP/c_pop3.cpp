#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include "c_pop3.h"
#include "..\\COMMON\\Compatability.h" // compatabile funtions for Microsoft- Borland compilers
#include "..\\CRYPT\\ENCODING\\c_codepage.h" // work with codepage



// ------------------------------- //
// -------- Start of File -------- //
// ------------------------------- //
// ----------------------------------------------------------- // 
// C++ Source Code File Name: gxspop3.cpp
// C++ Compiler Used: MSVC, BCC32, GCC, HPUX aCC, SOLARIS CC
// Produced By: DataReel Software Development Team
// File Creation Date: 02/23/2001
// Date Last Modified: 01/01/2009
// Copyright (c) 2001-2009 DataReel Software Development
// ----------------------------------------------------------- // 
// ------------- Program Description and Details ------------- // 
// ----------------------------------------------------------- // 
/*
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  
USA

Post Office Protocol 3 (POP3) classes used with applications
that require use of embedded POP3 client/server functions.

modified version dreel library
*/

int GetStrPos(const char * buf, const char * Example,int BufLen=0)
{
    if(buf==NULL||Example==NULL||BufLen<0) return -1;
    if (!BufLen) BufLen=strlen(buf);

    const char * TempExample=Example;
    __int32 res=-1;
    __int32 tempres=-1;

    for(int i=0;i<BufLen;i++)
    {
        if(buf[i]!=*TempExample)
        {
            TempExample=Example;
            tempres=-1;
        }
        if(buf[i]==*TempExample)
        {
            if(tempres==-1)tempres=i;
            TempExample++;
            if(*TempExample==0)
            {   // match founded
                res=tempres;
                break;
            }
        }
    }
    return res;
}
// ----------------------------------------------------------- // 
BOOL __fastcall c_pop3::ConnectEClient(const char *host, const char *username, const char *password,
		int *MessOnServer, __int32 port)
{
	if(ConnectClient(host,port))
	{
		if(POP3Login(username,password,MessOnServer))
		{
			return true;
		}
	}
	return false;
}
BOOL __fastcall c_pop3::ConnectClient(const char *host, __int32 port)
// Function used to connect a POP3 client to a server. Returns zero if no
// errors occur.
{
	MessagesOnServer=-1;
	SetLocalError(C_SOCK_NO_ERROR);
	reply_buf[0]=0;	

	if(!cntrl_sock->Create(0,(WORD)port,(char *)host)) return false;

	if(!cntrl_sock->Connect_s())
	{
		SetLocalError(POP3_CH_ERROR_CONN);
		return false;
	}

	// Read the server's response
	if(!RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE, "+OK"))
	{
		SetLocalError(POP3_CH_ERROR_CONN_RESP);
		return false;
	}

	return true;
}

BOOL __fastcall c_pop3::SendCommand(const char *command, 
									const char *response, 
									const char *args)
									// Function used to send a command to a POP3 server and read the server's
									// response. Returns zero if no errors occur.
{
	char * CmdBuf=command_buf;
	if(command==NULL||response==NULL) return false;	

	int totlen=5;// \r\n +' '+ \x0 + rezerv
	totlen+=strlen(command);
	if(args)totlen+=strlen(args);

	if(totlen>SOCK_COMMAND_BUF_SIZE)
	{
		CmdBuf = new char[totlen];
		if(CmdBuf==NULL)
		{
			SetLocalError(POP3_CH_ERROR_MEM_ALLOC);
			return false;
		}
	}

	// Clear the command and response buffers	
	CmdBuf[0]=0;
	reply_buf[0]=0;

	if(args) {		
		sprintf_loc(CmdBuf,totlen, "%s %s\r\n", command, args);
	}
	else {		
		sprintf_loc(CmdBuf,totlen,"%s\r\n", command);
	}

	int len = strlen(CmdBuf);
	bool res=true;

	// Send POP3 command using a blocking write
	if(cntrl_sock->Send((BYTE *)CmdBuf, len)<0)res=false;

	// Read the server's response
	if(res&&!RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE-1, response)) res=false;	

	// free resources
	if(CmdBuf!=command_buf) delete CmdBuf;

	return res;
}

BOOL __fastcall c_pop3::POP3Login(const char *username, 
								  const char *password, __int32 *MessOnServer) 
								  // Function used to logon to a POP3 server. Returns zero 
								  // no errors occur.
{
	MessagesOnServer=-1;
	if(!SendCommand("USER", "+OK", username)) return false;		
	if(!SendCommand("PASS", "+OK", password)) return false;
	
	int RespSize = strlen(reply_buf);	
	for(int i=0;i<RespSize;i++)
	{		
		if(isdigit(reply_buf[i]))
		{
			MessagesOnServer=atoi(&reply_buf[i]);			
			break;
		}		
	}
	if(MessOnServer!=NULL)*MessOnServer=MessagesOnServer;
	return true;
}

BOOL __fastcall c_pop3::POP3Logout()
// Function used to send the POP3 "QUIT" command. Returns zero 
// if no errors occur.
{
	BOOL res=SendCommand("QUIT", "+OK");
	cntrl_sock->Close();
	return res;
}

BOOL __fastcall c_pop3::POP3RSet()
// Function used to send the POP3 "Reset" command. Returns zero 
// if no errors occur.
{
	return SendCommand("RSET", "+OK");
}

BOOL __fastcall c_pop3::POP3List(char *buf, __int32 bytes)
// Function used to list all the message numbers on the POP3 server.
// Returns zero if no errors occur.
{
	strcpy(command_buf, "LIST\r\n");
	int len = strlen(command_buf);

	// Send POP3 command using a blocking write
	if(cntrl_sock->Send((PBYTE)command_buf, len) < 0) return false;

	// Read the server's response
	return RecvResponse(buf, bytes, "\r\n.\r\n");
}

#define LIST_BUF_SIZE	(1024*1)
BOOL __fastcall c_pop3::POP3List(int * NumMessages, __int32 * TotMessagesSize, __int32 * MessagesSize, __int32 MaxMessagesCount)
// Function used to list all the message numbers on the POP3 server.
// Returns zero if no errors occur.
{	
	bool res=false;
	char * ListBuf = reply_buf;
	if(MessagesOnServer>50) ListBuf=new char [LIST_BUF_SIZE];
	if(ListBuf == NULL)
	{
		SetLocalError(POP3_CH_ERROR_MEM_ALLOC);
		return false;
	}
	ListBuf[0]=0;	

	if(TotMessagesSize!=NULL)*TotMessagesSize=0;

	char MsgTempBuf[32];
	int MsgTempBufPointer=0;	

	int CurrMsgSizeNumber=0; // current index of message size

	int EventProgress=0;
	if(POP3List(ListBuf,LIST_BUF_SIZE))
	{
		int RespSize = strlen(ListBuf);		
		for(int i=0;i<RespSize;i++)
		{
			char CurrChar = ListBuf[i];
			switch(EventProgress)
			{
			case 0:
				if(i<(RespSize-2)&&!memcmp(&ListBuf[i],"OK",2))
				{
					MsgTempBufPointer=0;					

					EventProgress++;
					i+=3;

					res=true;
				}
				break;
			case 1:
				if(isdigit(CurrChar)&&MsgTempBufPointer<(int)sizeof(MsgTempBuf))
				{
					MsgTempBuf[MsgTempBufPointer++]=CurrChar;
				}
				else if(CurrChar!=' ') // if number of messages are space trailing
				{
					MsgTempBuf[MsgTempBufPointer]=0; // end of string
					if(NumMessages!=NULL)*NumMessages=atoi(MsgTempBuf);
					EventProgress++;
				}
				break;
			case 2:
				if(i<(RespSize-2)&&!memcmp(&ListBuf[i],"\r\n",2))
				{ // begin new MessageIndex
					MsgTempBufPointer=0;					

					EventProgress++;
					i+=1;
				}
				break;
			case 3:
				if(isdigit(CurrChar))
				{
					MsgTempBuf[MsgTempBufPointer++]=CurrChar;
				}				
				else if(CurrChar==' ')
				{ // begin message size
					MsgTempBuf[MsgTempBufPointer]=0; // end of string
					CurrMsgSizeNumber=atoi(MsgTempBuf)-1; // current index of message size

					MsgTempBufPointer=0;					

					EventProgress++;			
				}
				else
				{ // no more messages in a row
					EventProgress=100;
				}
				break;
			case 4:
				if(isdigit(CurrChar))
				{
					MsgTempBuf[MsgTempBufPointer++]=CurrChar;
				}
				else if(i<(RespSize-2)&&!memcmp(&ListBuf[i],"\r\n",2))
				{
					MsgTempBuf[MsgTempBufPointer]=0; // end of string
					int CurrMesSize = atoi(MsgTempBuf);
					if(TotMessagesSize!=NULL) *TotMessagesSize+=CurrMesSize;

					if(CurrMsgSizeNumber>=0&&CurrMsgSizeNumber<MaxMessagesCount
						&&MessagesSize!=NULL)
					{
						MessagesSize[CurrMsgSizeNumber]=CurrMesSize;
					}

					// begin new MessageIndex
					MsgTempBufPointer=0;					

					EventProgress=3;
					i+=1;
				}
				break;
			default:
				break;
			}		
		}		
	}

	if(ListBuf!=reply_buf)delete ListBuf;

	return res;
}

BOOL __fastcall c_pop3::POP3Retr(int message_number,
								 char *buf, __int32 bytes, __int32 * bytesRecieved)
// Function used to retrieve a message from the POP3 server.
// Returns zero if no errors occur.
{
	sprintf(command_buf, "RETR %d\r\n", message_number);
	int len = strlen(command_buf);

	// Send POP3 command using a blocking write
	if(cntrl_sock->Send((PBYTE)command_buf, len) < 0) return false;

    // Read the server's response
    BOOL res=RecvResponse(buf, bytes, "\r\n.\r\n");
    if(bytesRecieved!=NULL)
    {
        __int32 OKPos=GetStrPos(buf,"+OK",bytes);
        if(OKPos!=-1)
        {
            *bytesRecieved=GetStrPos(buf+OKPos,"\r\n.\r\n",bytes);
            if(*bytesRecieved<0)*bytesRecieved=0;
        }
        else *bytesRecieved=0;
    }

	return res;
}

// get charset
int __fastcall c_pop3::POP3GetCodePage(char *buf, __int32 bytes)
{
    if(bytes==0) bytes=strlen(buf);
    __int32 charsetpos=GetStrPos(buf,"charset=",bytes);
    if(charsetpos==-1) return 0;
    charsetpos+=8; //"charset="

    char CodePageName[32];
    __int32 CodePageNamePointer=0;
    memset(CodePageName,0,sizeof(CodePageName));
    for(int i=charsetpos;i<bytes;i++)
    {
        char CurrChar=buf[i];
        if(!isalnum(CurrChar)&&CurrChar!='-')
        { // end of charset name
            break;
        }

        if(CodePageNamePointer<(int)sizeof(CodePageName))
        {
            CodePageName[CodePageNamePointer++]=(char)tolower(CurrChar);
        }
    }
	return c_codepage::GetCodePage(CodePageName);
}

// get encoding position
e_MIME_Encoding __fastcall c_pop3::POP3GetEncoding(char *buf, __int32 bytes)
{
    if(bytes==0) bytes=strlen(buf);
    __int32 EncodingPos=GetStrPos(buf,"Content-Transfer-Encoding:",bytes);
    if(EncodingPos==-1) return EMIME_Unknown;

    EncodingPos+=26;
    char EncodingName[128];
    __int32 EncodingPointer=0;
    memset(EncodingName,0,sizeof(EncodingName));
    for(int i=EncodingPos;i<bytes;i++)
    {
        char CurrChar=buf[i];
        if(EncodingPointer==0&&CurrChar==' ') continue;

        if(CurrChar=='\r'||CurrChar=='\n'||CurrChar==' ')
        { // end of charset name
            break;
        }

        if(EncodingPointer<(int)sizeof(EncodingName))
        {
            EncodingName[EncodingPointer++]=(char)tolower(CurrChar);
        }
    }

    return c_encoding::GetEncoding(EncodingName);
}

BOOL __fastcall c_pop3::POP3Delete(int message_number,
								   char *buf, __int32 bytes)
// Function used to delete a message from the POP3 server.
// Returns zero if no errors occur.
// but messageas are deletet only, if quit command issued !!!
{
	sprintf(command_buf, "DELE %d\r\n", message_number);
	int len = strlen(command_buf);

	// Send POP3 command using a blocking write
	if(cntrl_sock->Send((PBYTE)command_buf, len) < 0) return false;

	// Read the server's response	
	if(buf==NULL)
	{
		buf=reply_buf;
		bytes=SOCK_REPLY_BUF_SIZE;
	}
	return RecvResponse(buf, bytes, "+OK");	
}

BOOL __fastcall c_pop3::POP3Top(int message_number, char *buf, __int32 bytes)
// Function used to retrieve a message header from the POP3 server.
// Returns zero if no errors occur.
{
	sprintf_loc(command_buf,SOCK_COMMAND_BUF_SIZE, "TOP %d 0\r\n", message_number);
	int len = strlen(command_buf);

	// Send POP3 command using a blocking write
	if(cntrl_sock->Send((PBYTE)command_buf, len) < 0) return false;

	// Read the server's response
	return RecvResponse(buf, bytes, "\r\n.\r\n");	
}

BOOL __fastcall c_pop3::RecvResponse(char *buf, __int32 bytes, const char *response)
// Blocking receive function used to read a reply from an POP3 server
// following a command. If the specified response is not received within
// the timeout period this function will return false to indicate an error.
// Returns true if successful.
{
	bytes-=1; // for array overloading

	int bytes_read = 0;           // Reset the byte counter
	int num_read = 0;         // Actual number of bytes read
	int num_req = (int)bytes; // Number of bytes requested 
	char *p = buf;            // Pointer to the buffer
    __int32 OKPos=-1;
	while(bytes_read < bytes) { // Loop until the buffer is full
		if(!cntrl_sock->WaitForRecieveData(time_out_ms)) {
			SetLocalError(POP3_CH_ERROR_REQ_TIMEOUT);
			if(bytes_read >= 0) buf[bytes_read] = 0;
			return false;
		}
		if((num_read = cntrl_sock->RecvAny((PBYTE)p, num_req-bytes_read)) > 0) {
			bytes_read += num_read;   // Increment the byte counter
			p += num_read;            // Move the buffer pointer for the next read

            if(OKPos==-1)
            {
                OKPos=GetStrPos(buf,"+OK",bytes_read);
            }
            if(OKPos!=-1)
            {
                __int32 RespPos=GetStrPos(buf+OKPos,response,bytes_read-OKPos);
                if(RespPos!=-1)
                {
                    if(bytes_read >= 0) buf[bytes_read] = 0;
                    return true; // Found matching string
                }
            }
		}
		if(num_read == 0) {
			if(bytes_read >= 0) buf[bytes_read] = 0;
			SetLocalError(POP3_CH_ERROR_DISCONNECTED);
			return false; // An error occurred during the read
		}
		if(num_read < 0) {
			if(bytes_read >= 0) buf[bytes_read] = 0;
			SetLocalError(POP3_CH_ERROR_RECEIVE);	
			return false; // An error occurred during the read
		}
	}

	// The receieve buffer is full - buffer overflow
	SetLocalError(POP3_CH_ERROR_BUFOVER);	
	if(bytes_read >= 0) buf[bytes_read] = 0;
	return false;
}

void __fastcall c_pop3::SetLocalError(int SetError)
{
	//c_sock_compl::SetLocalError(SetError);
	// чтобы первая ошибка не затиралась
	if(LastErrorPOP3!=C_SOCK_NO_ERROR&&SetError!=C_SOCK_NO_ERROR) return;

	LastErrorPOP3=SetError;
	if(SetError==C_SOCK_NO_ERROR)
	{        
		cntrl_sock->ClearError();	
	}
}
// ----------------------------------------------------------- //
// ------------------------------- //
// --------- End of File --------- //
// ------------------------------- //
