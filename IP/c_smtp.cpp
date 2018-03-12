#include "stdafx.h"

#include <string.h>
#include <stdio.h>
#include <time.h>


#include "c_smtp.h"
#include "..\\COMMON\\Compatability.h" // compatabile funtions for Microsoft- Borland compilers
#include "..\\CRYPT\\base64.h" // for authentification

// ------------------------------- //
// -------- Start of File -------- //
// ------------------------------- //
// ----------------------------------------------------------- // 
// C++ Source Code File Name: gxsmtp.cpp
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

Simple Mail Transfer Protocol (SMTP) classes used with applications
that require use of embedded SMTP client/server functions.

modified dreel library
*/

// ----------------------------------------------------------- // 
BOOL __fastcall c_smtp::ConnectEClient(const char *host, 
		const char *username, const char *password, __int32 port)
{
	if(ConnectClient(host,port))
	{
		if(SMTPLogin())
		{
			if(ESMTPAuthLogin(username, password))
			{				
				return true;
			}
		}
	}
	return false;
}
// ----------------------------------------------------------- // 
BOOL __fastcall c_smtp::ConnectClient(const char *host, __int32 port)
// Function used to connect a SMTP client to a server. Returns zero if no
// errors occur.
{
	SetLocalError(C_SOCK_NO_ERROR);
	reply_buf[0]=0;	

	if(!cntrl_sock->Create(0,(WORD)port,(char *)host)) return false;

	if(!cntrl_sock->Connect_s())
	{
		SetLocalError(SMTP_CH_ERROR_CONN);
		return false;
	}

	// Read the server's response
	if(!RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE, "220"))
	{
		SetLocalError(SMTP_CH_ERROR_CONN_RESP);
		return false;
	}

	return true;
}

BOOL __fastcall c_smtp::SendCommand(const char *command, 
									const char *response, 
									const char *args)
									// Function used to send a command to an SMTP server and read the server's
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
			SetLocalError(SMTP_CH_ERROR_MEM_ALLOC);
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

	// Send SMTP command using a blocking write
	if(cntrl_sock->Send((BYTE *)CmdBuf, len)<0)res=false;

	// Read the server's response
	// PC-lint 04/26/2005: Possible access of out-of-bounds pointer
	if(res&&!RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE-1, response)) res=false;	

	// free resources
	if(CmdBuf!=command_buf) delete CmdBuf;

	return res;
}

BOOL __fastcall c_smtp::SMTPLogin(const char *domain_name) 
// Function used to send the SMTP "Hello" command. Returns zero if 
// no errors occur.
{
	if(domain_name==NULL) domain_name=cntrl_sock->GetDomainName();
	return SendCommand("HELO", "250", domain_name);
}

BOOL __fastcall c_smtp::ESMTPLogin(const char *client_name) 
// Login for ESMTP mail servers if roaming. Returns zero if 
// no errors occur.
{
	if(client_name==NULL) client_name=cntrl_sock->GetDomainName();
	return SendCommand("EHLO", "250", client_name);
}

BOOL __fastcall c_smtp::ESMTPAuthLogin(const char *username, 
									   const char *password)
{		
	if(!SendCommand("AUTH LOGIN", "334"))
	{
		SetLocalError(SMTP_CH_ERROR_ELOGIN_NOT_SUPPORT);
		return false;
	}

	if(!SendCommand(base64_encode((PBYTE)username).c_str(), "334")
	   ||!SendCommand(base64_encode((PBYTE)password).c_str(), "235"))
	{
		SetLocalError(SMTP_CH_ERROR_ELOGIN);
		return false;
	}
	return true;
}

BOOL __fastcall c_smtp::SMTPLogout()
// Function used to send the SMTP "QUIT" command. Returns zero if
// no errors occur.
{
	BOOL res=SendCommand("QUIT", "221");
	cntrl_sock->Close();
	return res;
}

BOOL __fastcall c_smtp::SMTPRSet()
// Function used to send the SMTP "Reset" command. Returns zero if
// no errors occur.
{
	return SendCommand("RSET", "250");
}

BOOL __fastcall c_smtp::SMTPMailFrom(const char *from_email_address)
// Function used to send the SMTP "MAIL FROM" command. Returns zero if 
// no errors occur.
{
	return SendCommand("MAIL FROM:", "250", from_email_address);
}

BOOL __fastcall c_smtp::SMTPRcptTo(const char *to_email_address)
// Function used to send the SMTP "RCPT TO" command. Returns zero if
// no errors occur.
{
	return SendCommand("RCPT TO:", "250", to_email_address);
}

BOOL __fastcall c_smtp::SMTPData(const char *body_text, __int32 body_len)
// Function used to send a text body. Returns zero if no errors occur.
{
	if(!SendCommand("DATA", "354")) return false;

	// Send the message body
	if(cntrl_sock->Send((PBYTE)body_text, body_len) < 0) return false;

	// Send the end of message sequence
	strcpy_loc(command_buf,SOCK_COMMAND_BUF_SIZE, "\r\n.\r\n");
	int len = strlen(command_buf);
	if(cntrl_sock->Send((PBYTE)command_buf, len) < 0) return false;

	// Read the server's response	
	return RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE-1, "250");
}

BOOL __fastcall c_smtp::RecvResponse(char *buf, __int32 bytes, const char *response)
// Blocking receive function used to read a reply from an SMTP server
// following a command. If the specified response is not received within
// the timeout period this function will return false to indicate an error.
// Returns true if successful.
{

	bytes-=1;// PC-lint 04/26/2005: Possible access of out-of-bounds pointer

	int bytes_read = 0;           // Reset the byte counter
	int num_read = 0;         // Actual number of bytes read
	int num_req = (int)bytes; // Number of bytes requested 
	char *p = buf;            // Pointer to the buffer
	int len = 3;

	while(bytes_read < bytes) { // Loop until the buffer is full
		if(!cntrl_sock->WaitForRecieveData(time_out_ms)) { 
			SetLocalError(SMTP_CH_ERROR_REQ_TIMEOUT);
			if(bytes_read >= 0) buf[bytes_read] = 0;
			return false;
		}
		if((num_read = cntrl_sock->RecvAny((PBYTE)p, num_req-bytes_read)) > 0) {
			bytes_read += num_read;   // Increment the byte counter
			p += num_read;            // Move the buffer pointer for the next read

			if(buf[bytes_read-1] == '\n') { // We received the entire reply line
				buf[bytes_read] = 0; // NULL terminate the reply
				if(bytes_read > len) { // Look for numeric response code
					if(strncmp(response, buf, len) == 0) {
						return true; // Found matching numeric reply code
					}
					else {
						// Server replied with an error
						SetLocalError(SMTP_CH_ERROR_SMTP);
						return false;
					}
				}
				else { // Invalid reply code
					SetLocalError(SMTP_CH_ERROR_SMTP_REPLY);
					return false;
				}
			}
		}
		if(num_read == 0) {
			if(bytes_read >= 0) buf[bytes_read] = 0;
			SetLocalError(SMTP_CH_ERROR_DISCONNECTED);
			return false; // An error occurred during the read
		}
		if(num_read < 0) {
			if(bytes_read >= 0) buf[bytes_read] = 0;
			SetLocalError(SMTP_CH_ERROR_RECEIVE);			
			return false; // An error occurred during the read
		}
	}

	// The receieve buffer is full - buffer overflow
	SetLocalError(SMTP_CH_ERROR_BUFOVER);	
	if(bytes_read >= 0) buf[bytes_read] = 0;
	return false;
}


BOOL __fastcall c_smtp::SMTPSendMessage(const char *to, 
										const char *from, 
										const char *subject, 
										const char *body,
                                        const char * codepage,
                                        const char * encoding)
										// Function used to send a formatted message. Returns zero if no errors
										// occur.
{
	if(!SMTPRSet()) return false;
	if(!SMTPMailFrom(from)) return false;		
	if(!SMTPRcptTo(to)) return false;

	if(!SendCommand("DATA", "354")) return false;

	// Construct a message header	
	char * TimeStamp=GetSMTPTimeStamp();
	unsigned header_len = strlen(from) + strlen(to) + strlen(subject) + \
		strlen(TimeStamp);

	header_len += 50; // Allocate space for additional formatting
    if(codepage!=NULL) header_len +=strlen(codepage)+35;
    if(encoding!=NULL)  header_len +=strlen(encoding)+28;
	char *message_header = new char[header_len+1];
	if(!message_header)
	{
		SetLocalError(SMTP_CH_ERROR_MEM_ALLOC);
		return false;
	}

	// Format the message header
	// 11/17/2007: Fix for LF problem
	// SMTP protocol reported an error condition
	// 451 See http://pobox.com/~djb/docs/smtplf.html
	// http://cr.yp.to/docs/smtplf.html
	sprintf_loc(message_header,header_len+1, "Date: %s\r\nFrom: %s\r\nTo: %s\r\nSubject: %s\r\n",
		TimeStamp, from, to, subject);

    if(codepage!=NULL)
    {
        sprintf_loc(message_header+strlen(message_header),header_len+1-strlen(message_header), "Content-Type: text/plain; charset=%s\r\n",
		    codepage);
    }

    if(encoding!=NULL)
    {
        sprintf_loc(message_header+strlen(message_header),header_len+1-strlen(message_header), "Content-Transfer-Encoding: %s\r\n",
		    encoding);
    }

	// Send the message header
	if(cntrl_sock->Send((PBYTE)message_header, (int)strlen(message_header)) < 0) {
		// PC-lint 04/26/2005: Avoid inappropriate de-allocation error,
		// but message_header is not a user defined type so there is no destructor
		// to call for the array members.
		delete[] message_header;
		return false;
	}

	// Send the body of the message
	if(cntrl_sock->Send((PBYTE)body, (int)strlen(body)) < 0) {
		// PC-lint 04/26/2005: Avoid inappropriate de-allocation error,
		// but message_header is not a user defined type so there is no destructor
		// to call for the array members.
		delete[] message_header;
		return false;
	}

	// Send the end of message sequence
	// PC-lint 04/26/2005: Avoid inappropriate de-allocation error,
	// but message_header is not a user defined type so there is no destructor
	// to call for the array members.
	delete[] message_header;

	strcpy_loc(command_buf,SOCK_COMMAND_BUF_SIZE ,"\r\n.\r\n");
	int len = strlen(command_buf);
	if(cntrl_sock->Send((PBYTE)command_buf, len) < 0) return false;

	// Read the server's response
	if(!RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE, "250")) return false;
	return true;
}
char * __fastcall c_smtp::GetSMTPTimeStamp()
// Function used to make a SMTP time string. Returns a
// null terminated time string in the following format: 
// Weekday, Day Month Year HH:MM:SS - Timezone
{		
	time_t STime;
	tm *TimeBuffer;	
	time(&STime);


#if defined (__WIN32__)
	TimeBuffer = localtime(&STime);
#elif defined (__UNIX__) && defined (__REENTRANT__)
	// Using the UNIX localtime_r() function instead of localtime()
	localtime_r(&STime, TimeBuffer);
#else // Use localtime() by default
	TimeBuffer = localtime(&STime);
#endif	
	
	// Format the SMTP date string
	strftime(time_buf,SMTP_TIME_BUF_SIZE,"%a, %d %b %Y %H:%M:%S", TimeBuffer);	

	// timezone
	TIME_ZONE_INFORMATION TmpTimeZone;
	GetTimeZoneInformation(&TmpTimeZone);
    __int32 TimeBufLen=strlen(time_buf);
    __int32 TimeZoneOffset=-(TmpTimeZone.Bias/60);
	sprintf_loc(time_buf+TimeBufLen,SMTP_TIME_BUF_SIZE-TimeBufLen," %+.2d00",TimeZoneOffset);

	return time_buf;
}

void __fastcall c_smtp::SetLocalError(int SetError)
{
	//c_sock_compl::SetLocalError(SetError);
	// чтобы первая ошибка не затиралась
	if(LastErrorSMTP!=C_SOCK_NO_ERROR&&SetError!=C_SOCK_NO_ERROR) return;

	LastErrorSMTP=SetError;
	if(SetError==C_SOCK_NO_ERROR)
	{        
		cntrl_sock->ClearError();		
	}	
}
// ----------------------------------------------------------- //
// ------------------------------- //
// --------- End of File --------- //
// ------------------------------- //
