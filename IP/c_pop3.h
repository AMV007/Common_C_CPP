// ------------------------------- //
// -------- Start of File -------- //
// ------------------------------- //
// ----------------------------------------------------------- // 
// C++ Header File Name: gxspop3.h 
// C++ Compiler Used: MSVC, BCC32, GCC, HPUX aCC, SOLARIS CC
// Produced By: DataReel Software Development Team
// File Creation Date: 02/23/2001
// Date Last Modified: 01/01/2009
// Copyright (c) 2001-2009 DataReel Software Development
// ----------------------------------------------------------- // 
// ---------- Include File Description and Details  ---------- // 
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
// ----------------------------------------------------------- // 
#ifndef __C_POP3_H__
#define __C_POP3_H__

#include "c_sock_over.h"
#include "..\\CRYPT\\ENCODING\\c_encoding.h" // work with encodings

#define POP3_CH_ERROR_MEM_ALLOC        (POP3_CH_ERROR_BASE -  1)
#define POP3_CH_ERROR_CONN_RESP        (POP3_CH_ERROR_BASE -  2)
#define POP3_CH_ERROR_CONN		       (POP3_CH_ERROR_BASE -  3)
#define POP3_CH_ERROR_REQ_TIMEOUT      (POP3_CH_ERROR_BASE -  4)
#define POP3_CH_ERROR_POP3		       (POP3_CH_ERROR_BASE -  5)
#define POP3_CH_ERROR_POP3_REPLY       (POP3_CH_ERROR_BASE -  6)
#define POP3_CH_ERROR_DISCONNECTED     (POP3_CH_ERROR_BASE -  7)
#define POP3_CH_ERROR_RECEIVE		   (POP3_CH_ERROR_BASE -  8)
#define POP3_CH_ERROR_BUFOVER		   (POP3_CH_ERROR_BASE -  9)


// POP3 client - See RFC 1225, 1460, 1725 for specifications
class c_pop3
{	
public:
	c_pop3(bool UseOverlapped=false) { 
		time_out_ms = 60*1000; // Default blocking timeout value       

		reply_buf[0] = 0; 
		command_buf[0] = 0;

		if(UseOverlapped)   cntrl_sock = new c_sock_over();
		else          	    cntrl_sock = new c_sock_base();
	}
	~c_pop3() {if(cntrl_sock!=NULL)delete cntrl_sock;};

public:
	BOOL __fastcall ConnectEClient(const char *host, const char *username, const char *password,
		int *MessOnServer=NULL, __int32 port = C_SOCK_POP3_PORT);
	BOOL __fastcall ConnectClient(const char *host, 
		int port = C_SOCK_POP3_PORT);
	BOOL __fastcall SendCommand(const char *command, const char *response, 
		const char *args = 0);
	BOOL __fastcall POP3Login(const char *username, const char *password, __int32 *MessOnServer=NULL);
	BOOL __fastcall POP3Logout();
	BOOL __fastcall POP3RSet();
	BOOL __fastcall POP3List(char *buf, __int32 bytes);
	BOOL __fastcall POP3List(int * NumMessages, __int32 * TotMessagesSize=NULL, __int32 * MessagesSizes=NULL, __int32 MaxMessagesCount=0);
	BOOL __fastcall POP3Retr(int message_number, char *buf, __int32 bytes, __int32 * bytesRecieved=NULL);
    __int32  __fastcall POP3GetCodePage(char *buf, __int32 bytes=0);
    e_MIME_Encoding __fastcall POP3GetEncoding(char *buf, __int32 bytes=0);
	BOOL __fastcall POP3Delete(int message_number, char *buf=NULL, __int32 bytes=0);
	BOOL __fastcall POP3Top(int message_number, char *buf, __int32 bytes);
	BOOL __fastcall RecvResponse(char *buf, __int32 bytes, const char *response);	
	void __fastcall SetTimeOut(int mseconds) {
		time_out_ms = mseconds;		
	}
	void __fastcall Close(){cntrl_sock->Close();};

	int  __fastcall GetLocalErrorPOP3(){return LastErrorPOP3;};
	int  __fastcall GetCntrlError(){return cntrl_sock->GetError();};
	int  __fastcall GetCntrlWSAError(){return cntrl_sock->GetWSAError();};
	void __fastcall SetLocalError(int SetError);
	
private:
	Ic_sock * cntrl_sock;
	int LastErrorPOP3;	
	int time_out_ms;  // Number of miliseconds before a blocking timeout	
	char command_buf[SOCK_COMMAND_BUF_SIZE]; // Buffer used to hold the last command
	int MessagesOnServer; // used to determine - alloc memory in List or use internal reply buf
	
public:
	char reply_buf[SOCK_REPLY_BUF_SIZE];   // Buffer used to hold the last reply		
};

#endif // __C_POP3_H__
// ----------------------------------------------------------- //
// ------------------------------- //
// --------- End of File --------- //
// ------------------------------- //
