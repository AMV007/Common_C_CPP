// ------------------------------- //
// -------- Start of File -------- //
// ------------------------------- //
// ----------------------------------------------------------- // 
// C++ Header File Name: gxsmtp.h
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

Simple Mail Transfer Protocol (SMTP) classes used with applications
that require use of embedded SMTP client/server functions.

this library - modified dreel
==============================================================
*/
// ----------------------------------------------------------- // 
#ifndef __C_SMTP_H__
#define __C_SMTP_H__

#include "c_sock_over.h"

#define SMTP_TIME_BUF_SIZE		(1024*1)

//-ERRORS----------------------------------------------------------
#define SMTP_CH_ERROR_MEM_ALLOC						(SMTP_CH_ERROR_BASE -  1)
#define SMTP_CH_ERROR_CONN_RESP						(SMTP_CH_ERROR_BASE -  2)
#define SMTP_CH_ERROR_CONN							(SMTP_CH_ERROR_BASE -  3)
#define SMTP_CH_ERROR_REQ_TIMEOUT					(SMTP_CH_ERROR_BASE -  4)
#define SMTP_CH_ERROR_SMTP							(SMTP_CH_ERROR_BASE -  5)
#define SMTP_CH_ERROR_SMTP_REPLY					(SMTP_CH_ERROR_BASE -  6)
#define SMTP_CH_ERROR_DISCONNECTED					(SMTP_CH_ERROR_BASE -  7)
#define SMTP_CH_ERROR_RECEIVE						(SMTP_CH_ERROR_BASE -  8)
#define SMTP_CH_ERROR_BUFOVER						(SMTP_CH_ERROR_BASE -  9)
#define SMTP_CH_ERROR_ELOGIN						(SMTP_CH_ERROR_BASE - 10)
#define SMTP_CH_ERROR_ELOGIN_NOT_SUPPORT			(SMTP_CH_ERROR_BASE - 11)


//-SMTP_CLASS------------------------------------------------------

// SMTP client - See RFC 821 for specifications
// http://www.sendmail.org
class c_smtp
{	
public:
	

	c_smtp(bool UseOverlapped=false) { 
		time_out_ms = 60*1000; // Default blocking timeout value   		
		reply_buf[0] = 0; 
		command_buf[0] = 0;

		if(UseOverlapped)   cntrl_sock = new c_sock_over();
		else          	    cntrl_sock = new c_sock_base();
	}
	~c_smtp() { if(cntrl_sock!=NULL)delete cntrl_sock; }

public:
	// usually used
	BOOL __fastcall ConnectEClient(const char *host, 
		const char *username, const char *password, 
		int port = C_SOCK_SMTP_PORT);
	BOOL __fastcall ConnectClient(const char *host, 
		int port = C_SOCK_SMTP_PORT);
	BOOL __fastcall SendCommand(const char *command, const char *response, 
		const char *args = NULL);
	BOOL __fastcall SMTPLogin(const char *domain_name=NULL);
	BOOL __fastcall ESMTPLogin(const char *client_name=NULL);
	BOOL __fastcall ESMTPAuthLogin(const char *username, 
		const char *password);
	BOOL __fastcall SMTPLogout();
	BOOL __fastcall SMTPRSet();
	BOOL __fastcall SMTPMailFrom(const char *from_email_address);
	BOOL __fastcall SMTPRcptTo(const char *to_email_address);
	BOOL __fastcall SMTPData(const char *body_text, __int32 body_len);
	BOOL __fastcall RecvResponse(char *buf, __int32 bytes, const char *response);	
	void __fastcall SetTimeOut(int mseconds) {
		time_out_ms=mseconds;
	}
	BOOL __fastcall SMTPSendMessage(const char *to,
                                    const char *from,
		                            const char *subject,
                                    const char *body,
                                    const char * codepage=NULL, // just name, no codepage coding in code
                                    const char * encoding=NULL); // just name, no codepage coding in code
	void __fastcall Close(){cntrl_sock->Close();};
	char * __fastcall GetSMTPTimeStamp();

	bool __fastcall CheckError(int close_on_err);  
	void __fastcall SetLocalError(int SetError);
	int  __fastcall GetLocalErrorSMTP(){return LastErrorSMTP;};
	int  __fastcall GetCntrlError(){return cntrl_sock->GetError();};
	int  __fastcall GetCntrlWSAError(){return cntrl_sock->GetWSAError();};

private:
	Ic_sock * cntrl_sock; // control data socket
	int LastErrorSMTP;	
	int time_out_ms;  // Number of mseconds before a blocking timeout	
	char command_buf[SOCK_COMMAND_BUF_SIZE]; // Buffer used to hold the last command  
	char time_buf[SMTP_TIME_BUF_SIZE];	
public:	
	char reply_buf[SOCK_REPLY_BUF_SIZE];   // Buffer used to hold the last reply		
};

#endif // __C_SMTP_H__
// ----------------------------------------------------------- //
// ------------------------------- //
// --------- End of File --------- //
// ------------------------------- //

