/*
 * ftp class
 * modified by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

// ------------------------------- //
// -------- Start of File -------- //
// ------------------------------- //
// ----------------------------------------------------------- // 
// C++ Source Code File Name: gxftp.cpp
// C++ Compiler Used: MSVC, BCC32, GCC, HPUX aCC, SOLARIS CC
// Produced By: DataReel Software Development Team
// File Creation Date: 02/23/2001
// Date Last Modified: 01/01/2009
// Copyright (c) 2001-2009 DataReel Software Development
// ----------------------------------------------------------- // 
// ------------- Program Description and Details ------------- // 
// ----------------------------------------------------------- // 
/*
this library modified dreel
==============================================================
*/
// ----------------------------------------------------------- // 
#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <algorithm>

#include "c_ftp_client.h"
#include "..\\COMMON\\Compatability.h"




c_ftp_client::c_ftp_client(BOOL Useoverlapped)
{    
    useoverlapped=Useoverlapped;
    conBindAccTimeout=3000;
	time_out_ms = 5000; // Default blocking timeout value   	
	reply_buf[0] = 0; 
	command_buf[0] = 0;
    DataPortListening=false;
	data_sock_port = 0;
	passive_mode = 0;

    nonBlocingConn=false;

	FileBytesWritedToServer=FileBytesReadedFromServer=0;

	fileCacheSize=FILE_IO_BLOCK_SIZE_MAX;
	FileIODataCache=NULL;

	data_sock =   NULL;
	if(useoverlapped)   cntrl_sock = new c_sock_over();
	else          	    cntrl_sock = new c_sock_base();

	if(cntrl_sock==NULL) c_ftp_client::SetError(FTP_CH_ERROR_MEM_ALLOC);
}

c_ftp_client::~c_ftp_client()
{
	Close();
    if(data_sock!=NULL)
    {
        delete data_sock;
        data_sock=NULL;
    }
	if(cntrl_sock!=NULL)
    {
        delete cntrl_sock;
        cntrl_sock=NULL;
    }
	if(FileIODataCache!=NULL)
	{
		delete FileIODataCache;
		FileIODataCache=NULL;
	}
}

void __fastcall c_ftp_client::Close()
{
	cntrl_sock->Close();
    
    if(data_sock!=NULL) data_sock->Close();

	DataPortListening=false;
	data_sock_port=0;
};

BOOL __fastcall c_ftp_client::ConnectLoginDir(const char *host, const char * Login, const char * Pass,
    const char * Dir, WORD port)
{
	if(ConnectHost(host,port))
	{
		if(FTPLogin(Login,Pass))
		{
			if(Dir!=NULL&&strlen(Dir)>0)
			{
				if(FTPChDir(Dir))
				{
					return true;
				}
			}
			else return true;
		}
	}
	return false;
}

BOOL __fastcall c_ftp_client::ConnectHost(const char *host, WORD port)
// Function used to connect a FTP client to a server. Returns zero if no
// errors occur.
{
	c_ftp_client::SetError(C_SOCK_NO_ERROR);
    reply_buf[0]=0;

	Close();    

	if(!cntrl_sock->Create(0,port,(char *)host,nonBlocingConn)) return false;

	if(!cntrl_sock->Connect_s())
	{
		c_ftp_client::SetError(FTP_CH_ERROR_CONN);
		return false;
	}

	// Read the server's response
	if(!RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE, "220 "))
	{
		c_ftp_client::SetError(FTP_CH_ERROR_CONN_RESP);
		return false;
	}

	return true;
}

BOOL __fastcall c_ftp_client::SendCommand(const char *command,
										  const char *response, 
										  const char *args)
										  // Function used to send a command to an FTP server and read the server's
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
			c_ftp_client::SetError(FTP_CH_ERROR_MEM_ALLOC);
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
	BOOL res=true;

	// Send command using a blocking write
	if(cntrl_sock->Send((BYTE *)CmdBuf, len)<0)
    {
        res=false;
    }

	// Read the server's response
	if(res&&!RecvResponse(reply_buf, SOCK_REPLY_BUF_SIZE, response))
    {
        res=false;
    }

	// free resources
	if(CmdBuf!=command_buf)
    {
        delete CmdBuf;
    }

	return res;
}


BOOL __fastcall c_ftp_client::FTPLogin(const char *username,
									   const char *password) 
									   // Function used to logon to an FTP server. Returns zero 
									   // if no errors occur.
{
	if(!SendCommand("USER", "331", username)) return false;
	return SendCommand("PASS", "230 ", password);
}

BOOL __fastcall c_ftp_client::FTPLogout()
// Function used to send the FTP "bye" command. Returns zero if
// no errors occur.
{   
	return SendCommand("QUIT", "221 ");
}

BOOL __fastcall c_ftp_client::FTPImageType(char type)
{
	if(type == 'a' || type == 'A') // ASCII
		return SendCommand("TYPE", "200", "A");
	else // Default to binary
		return SendCommand("TYPE", "200", "I");
}

BOOL __fastcall c_ftp_client::FTPChDir(const char *dname)
{
	return SendCommand("CWD", "250", dname);
}

BOOL __fastcall c_ftp_client::FTPPWD()
{
	return SendCommand("PWD", "257");
}

BOOL __fastcall c_ftp_client::FTPStat()
{
	return SendCommand("STAT", "211 ");
}

BOOL __fastcall c_ftp_client::FTPMkDir(const char *dname)
{
	return SendCommand("MKD", "257", dname);
}

BOOL __fastcall c_ftp_client::FTPRmDir(const char *dname)
{
	return SendCommand("RMD", "250", dname);
}

BOOL __fastcall c_ftp_client::FTPSize(const char *fname)
{
	return SendCommand("SIZE", "213", fname);
}

BOOL __fastcall c_ftp_client::FTPDelete(const char *fname)
{
	return SendCommand("DELE", "250", fname);
}

BOOL __fastcall c_ftp_client::FTPMove(const char *from, const char *to)
{
	if(!SendCommand("RNFR", "350", from)) return false;
	return SendCommand("RNTO", "250", to);
}



BOOL __fastcall c_ftp_client::FTPList(char *sbuf, unsigned bytes, __int32 full,
									  const char *args)
									  // FTP command used to list the contents of the current working
									  // directory. Returns zero if no errors occur.
{	
	if(!OpenDataPort())
	{		
		return false;
	}

	int byte_count = 0;       // Data port byte counter
	int num_read = 0;         // Actual number of bytes read
	int num_req = (int)bytes; // Number of bytes requested 
	char *p = sbuf;           // Pointer to the buffer
	BOOL res;

	if(full) {
		res=SendCommand("LIST", "150",args);		
	}
	else {
		res=SendCommand("NLIST", "150",args);		
	}

	if(!res)
	{
		CloseDataPort();		
		return false;
	}	

	reply_buf[0] = 0; // Reset the reply buffer

	if(passive_mode == 0) { // Active FTP mode
		// Block execution until data is received
		if(!data_sock->Accept_s()) {
			CloseDataPort();
			return false;
		}
	}

	while(byte_count < (int)bytes) { // Loop until the buffer is full
		num_read = ReadDataPort(p, num_req-byte_count);
		if(num_read > 0) {
			byte_count += num_read; // Increment the byte counter
			if(byte_count >= (int)bytes) {
				// The receieve buffer is full - buffer overflow
				if(byte_count >= 0) sbuf[byte_count] = 0;
				CloseDataPort();
				c_ftp_client::SetError(FTP_CH_ERROR_BUFOVER);
				return false;
			} 
			p += num_read; // Move the buffer pointer for the next read
			if(num_read < 0) {
				if(byte_count >= 0) sbuf[byte_count] = 0;
				CloseDataPort();
				c_ftp_client::SetError(FTP_CH_ERROR_RECEIVE);
				return false;
			}
		}
		else {
			break;
		}
	}

	// Null terminate the string buffer
	if(byte_count >= 0) sbuf[byte_count] = 0;
	if(num_read < 0) {
		CloseDataPort();
		c_ftp_client::SetError(FTP_CH_ERROR_RECEIVE);
		return false;
	}

	// Close the data port before the next read
	CloseDataPort();

	// Hanging intermittently when used with WIN32 ftp server's
	// so do not read the server's reply 
	strcpy_loc(reply_buf,SOCK_REPLY_BUF_SIZE, "226 Transfer complete.\r\n");

	return true;
}

BOOL __fastcall c_ftp_client::OpenDataPort()
{
	int reuse_opt = 1; // Socket reuse option

    CloseDataPort();
    if(data_sock==NULL)
    {
        data_sock = new c_sock_compl(useoverlapped);
        if(data_sock!=NULL)
        {
            data_sock->conn_bind_sock->SetSockTimeout(conBindAccTimeout,conBindAccTimeout,conBindAccTimeout);
            data_sock->accept_sock->SetSockTimeout(conBindAccTimeout,conBindAccTimeout,conBindAccTimeout);
        }
        else
        {
            c_ftp_client::SetError(FTP_CH_ERROR_MEM_ALLOC);
            return false;
        }
    }

	if(passive_mode == 1) 
    {
		// Send the passive FTP mode command instead of the port command
		if(!SendCommand("PASV", "227"))
		{
			c_ftp_client::SetError(FTP_CH_ERROR_DP_SEND);
			return false;
		}

		// NOTE: The server may send back a 226 reply code so the 
		// client must check the reply code be parsing the 277 reply.
		int reply_code;
		sscanf(reply_buf, "%d ", &reply_code); 
		if(reply_code == 226) { // Remove the first line of text.
			int len = strlen(reply_buf);
			int offset = 0;
			for(int i = 0; i < len; i++) {
				offset++;
				// The server's reply should always be terminated with a
				// \r\n at the end of each reply.
				if(reply_buf[i] == '\n') break;
			}
			// Remove the 226 reply line
			memmove(reply_buf, (reply_buf+offset), (len-offset));

			// Null terminate the string
			reply_buf[(len-offset)] = 0; 
		}

		// Validate the server's reply account for the \r\n terminator
		if(reply_buf[strlen(reply_buf)-3] != ')') {
			if(reply_buf[strlen(reply_buf)-3] != '.') {
				c_ftp_client::SetError(FTP_CH_ERROR_DP_VALID);
				return false;
			}
		}

		char s1[25]; char s2[25]; char s3[25]; char s4[25];
		char ip_port[255];
		// Parse the server's reply and check for errors
		sscanf(reply_buf, "%d %s %s %s %s%[^\r\n]", 
			&reply_code, s1, s2, s3, ip_port, s4); 

		if(reply_code != 227) {
			c_ftp_client::SetError(FTP_CH_ERROR_DP_REPLY);
			return false;
		}

		// Parse the IP and port information from the ip_port string.
		// The server will issue a PORT command formatted as a series of six 
		// numbers separated by commas. The first four octets is the IP 
		// address of the server and last two octets comprise the port that 
		// will be used for the data connection. To find the port multiply 
		// the fifth octet by 256 and then add the sixth octet to the total.
		int ip1, ip2, ip3, ip4, p1, p2;
		sscanf(ip_port, "(%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &p1, &p2);

		// Construct the server's host IP and port number
		sprintf_loc(ip_port,255, "%d.%d.%d.%d", ip1, ip2, ip3, ip4); 
		int port = (p1*256) + p2;

		if(	  !data_sock->Create(0,(WORD)port,ip_port)
			||!data_sock->SetSockOpt(SOL_SOCKET, SO_REUSEADDR,
			&reuse_opt, (int)sizeof(reuse_opt))
			||!data_sock->Connect_s()
			)
		{
			c_ftp_client::SetError(FTP_CH_ERROR_DP_CONN);
			return false;
		}		
	}
	else 
    {		
		if(!DataPortListening) //не все серваки поддерживают переподключение на уже использованный IP, не знаю почему
		{
			if(	  !data_sock->Create(0,0)
				||!data_sock->SetSockOpt(SOL_SOCKET, SO_REUSEADDR,
				&reuse_opt, sizeof(reuse_opt))
				||!data_sock->Bind_s()	// Bind the socket
				||!data_sock->GetSockName()
				||!data_sock->Listen_s()	// Listen for connection
				)
			{
				c_ftp_client::SetError(FTP_CH_ERROR_DP_BIND);
				return false;  
			}          


			// Get this machine's IP address and send it to the FTP server
			hostent * hostinfo = cntrl_sock->GetHostInformation();
			if(!hostinfo) {
				c_ftp_client::SetError(FTP_CH_ERROR_DP_HOST_INFO);
				return false;
			}  
            
            cntrl_sock->GetSockName();
            cntrl_sock->sock_addr.sin_addr;
            char *ControlAddr = (char *) &cntrl_sock->sock_addr.sin_addr;
            
			SOCKADDR_IN * Data_Conn_AddrIn=&data_sock->conn_bind_sock->sock_addr;
			for(int i = 0; ; i++) 
            {
				IN_ADDR *ialist = (IN_ADDR *)hostinfo->h_addr_list[i];
				if(ialist == NULL) break;
				memcpy(&Data_Conn_AddrIn->sin_addr.s_addr, ialist, sizeof(IN_ADDR));                 
                if(memcmp(ControlAddr, (char *) &Data_Conn_AddrIn->sin_addr,4)==0) break;
			}

			char *a = (char *) &Data_Conn_AddrIn->sin_addr;
			char *b = (char *) &Data_Conn_AddrIn->sin_port;

            port_buf[0]=0;
			sprintf_loc(port_buf,MAX_HOST_NAME_LEN, "PORT %d,%d,%d,%d,%d,%d", (a[0]&0xff), (a[1]&0xff),
				(a[2]&0xff), (a[3]&0xff), (b[0]&0xff), (b[1]&0xff));

			// Port the client is listening on
			data_sock_port = (int)ntohs(Data_Conn_AddrIn->sin_port);

			DataPortListening=true;
		}
        
		// Send the port command
		if(!SendCommand(port_buf, "200")) 
        {
			c_ftp_client::SetError(FTP_CH_ERROR_DP_SEND_F);
			return false;
		}
	}

	return true;
}

void __fastcall c_ftp_client::CloseDataPort()
{
    data_sock_port = 0;

    if(data_sock==NULL) return;

    data_sock->Close(); //не все серваки поддерживают переподключение на уже использованный IP, не знаю почему

    //не все серваки поддерживают переподключение на уже использованный IP, не знаю почему
    //if(passive_mode == 1) data_sock->Close();
    //else data_sock->accept_sock->Close();
}

int __fastcall c_ftp_client::ReadDataPort(void *buf, unsigned bytes)
{
    if(data_sock==NULL) return 0;
	if(passive_mode == 1) {
		return data_sock->conn_bind_sock->RecvAny((BYTE *)buf, bytes);
	}
	else { // Active ftp mode
		return data_sock->accept_sock->RecvAny((BYTE *)buf, bytes);
	}
}

int __fastcall c_ftp_client::WriteDataPort(void *buf, unsigned bytes)
{
    if(data_sock==NULL) return 0;
	if(passive_mode == 1) 
    {
		return data_sock->conn_bind_sock->Send((BYTE *)buf, bytes); // Blocking client write
	}
	else 
    { // Active ftp mode
		return data_sock->accept_sock->Send((BYTE *)buf, bytes); // Blocking server write
	}
}


BOOL __fastcall c_ftp_client::RecvResponse(char *buf, __int32 bytes,  const char *response)
// Blocking receive function used to read a reply from an FTP server
// following a command. If the specified response is not received within
// the timeout period this function will return false to indicate an error.
// Returns true if successful.
{
	bytes=bytes-1; // for possible buffer overran, not occure

	int byte_count = 0;       // Byte counter
	int num_read;         // Actual number of bytes read
	int num_req = (int)bytes; // Number of bytes requested 
	char *p = buf;            // Pointer to the buffer
	int reply_code = 0;       // Reply code 

	while(byte_count < bytes) { // Loop until the buffer is full
		if(!cntrl_sock->WaitForRecieveData(time_out_ms)) {
			c_ftp_client::SetError(FTP_CH_ERROR_REQ_TIMEOUT);
			if(byte_count >= 0) buf[byte_count] = 0;
			return false;
		}

		if((num_read = cntrl_sock->RecvAny((BYTE *)p, num_req-byte_count)) > 0) {
			byte_count += num_read; // Increment the byte counter
			p += num_read;          // Move the buffer pointer for the next read

			if(byte_count >= 4) 
            {
				sscanf(buf, "%d", &reply_code);
				// Check for FTP error codes 400 and higher
				if(reply_code >= 400) 
                {
					buf[byte_count] = 0; // Null terminate the reply buffer
					c_ftp_client::SetError(FTP_CH_ERROR_FTP);
					return false;
				}
				if((reply_code == 110) || (reply_code == 120) ||
					(reply_code == 202) || (reply_code == 332)) 
                {
						buf[byte_count] = 0; // Null terminate the reply buffer
						c_ftp_client::SetError(FTP_CH_ERROR_FTP_REPLY);
						return false;
				}
				if((reply_code == 125) || (reply_code == 225)) {
					buf[byte_count] = 0; // Null terminate the reply buffer
					return true; // Data connection already open
				}
			}
			
			// Search for a matching string
			char *pattern = (char *)response;			
			for(int i=0;i<byte_count;i++)
			{
				if(buf[i]!=*pattern)pattern = (char *)response;
				if(buf[i]==*pattern)
				{
					pattern++;
					if(*pattern == 0) {
						if(byte_count >= 0) buf[byte_count] = 0;
						return true; // Found matching string
					}	
				}				
			}						
		}
		if(num_read == 0) {
			//if(bytes_read >= 0) buf[bytes_read] = 0;
			c_ftp_client::SetError(FTP_CH_ERROR_DISCONNECT);
			return false; // An error occurred during the read
		}
		if(num_read < 0) {
			if(byte_count >= 0) buf[byte_count] = 0;
			c_ftp_client::SetError(FTP_CH_ERROR_RECEIVE);
			return false; // An error occurred during the read
		}
	}

	// The receieve buffer is full - buffer overflow
	c_ftp_client::SetError(FTP_CH_ERROR_BUFOVER);
	if(byte_count >= 0) buf[byte_count] = 0;
	return false;
}

BOOL __fastcall c_ftp_client::SetMaxFileCacheSize(int FileCacheSize)
{
	if(FileIODataCache==NULL)
	{		
		fileCacheSize=MIN(int,FileCacheSize,FILE_IO_BLOCK_SIZE_MAX);

		// размер также должен быть кратен FTP_IO_BLOCK_SIZE
		fileCacheSize=((int)(fileCacheSize/FTP_IO_BLOCK_SIZE))*FTP_IO_BLOCK_SIZE;

		// если получили нулевой, то проверяем
		fileCacheSize=MIN(int,fileCacheSize,FTP_IO_BLOCK_SIZE);
		return true;
	}
	return false;
}

const char * GetFileNamePos(const char * Path)
{
	int PathLen = strlen(Path);
	for(int i=(PathLen-1);i>0;i--)
	{
		if(Path[i]=='\\')
		{
			if(i==(PathLen-1)) return NULL;
			return &Path[i+1];
		}
	}
	return NULL;
}

BOOL __fastcall c_ftp_client::DownloadFile(const TCHAR * FileName, const char * ServerFileName)
{
	const char * FileNameOnServer=ServerFileName;
	if(FileNameOnServer==NULL)
	{
#ifdef _UNICODE
#else
		FileNameOnServer=(char *)GetFileNamePos(FileName);
#endif
		if(FileNameOnServer==NULL) return false;
	}	

	HANDLE hFile=CreateFile(FileName, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, 0, NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		BOOL res=DownloadFile(hFile,FileNameOnServer);
		CloseHandle(hFile);
		return res;
	}
	else
	{
		c_ftp_client::SetError(FTP_CH_ERROR_OPEN_FILE);
		return false;
	}
}

BOOL __fastcall c_ftp_client::DownloadFile(HANDLE hFile, const char * ServerFileName)
{	
	if(	hFile==NULL||hFile==INVALID_HANDLE_VALUE) return false;	

	if(FileIODataCache==NULL)
	{
		FileIODataCache = new BYTE[fileCacheSize];
		if(FileIODataCache==NULL)
		{
			c_ftp_client::SetError(FTP_CH_ERROR_MEM_ALLOC);
			return false;
		}
	}       

	if(!OpenDataPort()) return false;

	if(!SendCommand("RETR", "150",ServerFileName)) {
		CloseDataPort();
		return false;
	}

	reply_buf[0] = 0; // Reset the reply buffer

	if(FTPActiveGet()) { // Active FTP mode
		// Block execution until data is received
		if(!data_sock->Accept_s()) {
			CloseDataPort();
			return false;
		}
	}


	BOOL res=true;

	DWORD NumWriteBytes=0;
	int num_read=1; // Actual number of bytes read
	while(num_read>0)
	{
		num_read = ReadDataPort(FileIODataCache, FTP_IO_BLOCK_SIZE);
		if(num_read > 0) 
		{
			FileBytesReadedFromServer+=num_read;
			if(!WriteFile(hFile, FileIODataCache, num_read,&NumWriteBytes,NULL)) 
			{
				res=false;
				break;
			}			
		}
		else if(num_read<0){
			res=false;
			break;
		}
	}

	// Close the data port before the next read
	CloseDataPort();   //    accept_sock->Close();  

	// Hanging intermittently when used with WIN32 ftp server's
	// so do not read the server's reply 
	strcpy(reply_buf, "226 Transfer complete.\r\n");

	return res;
}

BOOL __fastcall c_ftp_client::UploadFile(const TCHAR * FileName, const char * ServerFileName)
{
	const char * FileNameOnServer=ServerFileName;
	if(FileNameOnServer==NULL)
	{
#ifdef _UNICODE
#else
		FileNameOnServer=(char *)GetFileNamePos(FileName);
#endif
		if(FileNameOnServer==NULL) return false;
	}	

	HANDLE hFile=CreateFile(FileName, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, 0, NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		BOOL res=UploadFile(hFile,FileNameOnServer);
		CloseHandle(hFile);
		return res;
	}
	else
	{
		c_ftp_client::SetError(FTP_CH_ERROR_OPEN_FILE);
		return false;
	}
}

BOOL __fastcall c_ftp_client::UploadFile(HANDLE hFile, const char * ServerFileName)
{	
	LARGE_INTEGER FileSize;
	if(	hFile==NULL||hFile==INVALID_HANDLE_VALUE
		||!GetFileSizeEx(hFile,&FileSize))
    {
        c_ftp_client::SetError(FTP_CH_ERROR_PARAM);
        return false;
    }

	if(FileIODataCache==NULL)
	{
		FileIODataCache = new BYTE[fileCacheSize];
		if(FileIODataCache==NULL)
		{
			c_ftp_client::SetError(FTP_CH_ERROR_MEM_ALLOC);
			return false;
		}
	}	

	if(!OpenDataPort())
    {
        c_ftp_client::SetError(FTP_CH_ERROR_OPEN_DATAPORT);
        return false;
    }

	if(!SendCommand("STOR", "150",ServerFileName))
	{
		CloseDataPort();
		c_ftp_client::SetError(FTP_CH_ERROR_UF_SEND_STOR);
		return false;
	}

	reply_buf[0] = 0; // Reset the reply buffer

	// Block execution until data is received

	if(FTPActiveGet() == 1)
	{ // Active FTP mode
		if(!data_sock->Accept_s())
		{
			CloseDataPort();
			c_ftp_client::SetError(FTP_CH_ERROR_UF_ACCEPT);
			return false;
		}
	}

	BOOL res=true;

	DWORD NumReadBytes=0;
	while(FileSize.QuadPart)
	{
		if(!ReadFile(hFile,FileIODataCache,fileCacheSize,&NumReadBytes,NULL))
		{
			c_ftp_client::SetError(FTP_CH_ERROR_UF_READ_FILE);
			res=false;
			break;
		}

		if(NumReadBytes==0) break;
		FileSize.QuadPart-=NumReadBytes;

		PBYTE DataPtr = FileIODataCache;
		while(NumReadBytes)
		{
			int TXSize = MIN(int,FTP_IO_BLOCK_SIZE, NumReadBytes);
			if(WriteDataPort(DataPtr, TXSize)<0)
			{
				/*
				AnsiString report=" LastError ="+(AnsiString)LastError
				+" NumReadBytes "+(AnsiString)NumReadBytes
				+" FileBytesWrited "+(AnsiString)FileBytesWrited
				+" TXSize "+(AnsiString)TXSize
				;
				MessageBox(NULL,report.c_str(),"",0);
				*/

				c_ftp_client::SetError(FTP_CH_ERROR_UF_SEND_DATA);
				res=false;
				break;
			}
			DataPtr += TXSize;
			NumReadBytes-=TXSize;
			FileBytesWritedToServer+=TXSize;

		}
	}


	CloseDataPort();  // close accept

	return res;
}

BOOL __fastcall c_ftp_client::UploadFile(const char * Data, size_t DataLen, const char * ServerFileName)
{	
	size_t FileSize = DataLen;
	if(	Data==NULL || ServerFileName==NULL || FileSize==0)
    {
        c_ftp_client::SetError(FTP_CH_ERROR_PARAM);
        return false;
    }

	if(FileIODataCache==NULL)
	{
		FileIODataCache = new BYTE[fileCacheSize];
		if(FileIODataCache==NULL)
		{
			c_ftp_client::SetError(FTP_CH_ERROR_MEM_ALLOC);
			return false;
		}
	}	

	if(!OpenDataPort())
    {
        c_ftp_client::SetError(FTP_CH_ERROR_OPEN_DATAPORT);
        return false;
    }

	if(!SendCommand("STOR", "150",ServerFileName))
	{
		CloseDataPort();
		c_ftp_client::SetError(FTP_CH_ERROR_UF_SEND_STOR);
		return false;
	}

	reply_buf[0] = 0; // Reset the reply buffer

	// Block execution until data is received

	if(FTPActiveGet() == 1)
	{ // Active FTP mode
		if(!data_sock->Accept_s())
		{
			CloseDataPort();
			c_ftp_client::SetError(FTP_CH_ERROR_UF_ACCEPT);
			return false;
		}
	}

	BOOL res=true;

	DWORD NumReadBytes=0;
    size_t Offset=0;
	while(FileSize)
	{
        NumReadBytes=min(fileCacheSize, (int)FileSize);
        memcpy(FileIODataCache, Data+Offset, NumReadBytes);
        Offset+=NumReadBytes;		

		if(NumReadBytes==0) break;
		FileSize-=NumReadBytes;

		PBYTE DataPtr = FileIODataCache;
		while(NumReadBytes)
		{
			int TXSize = MIN(int,FTP_IO_BLOCK_SIZE, NumReadBytes);
			if(WriteDataPort(DataPtr, TXSize)<0)
			{
				/*
				AnsiString report=" LastError ="+(AnsiString)LastError
				+" NumReadBytes "+(AnsiString)NumReadBytes
				+" FileBytesWrited "+(AnsiString)FileBytesWrited
				+" TXSize "+(AnsiString)TXSize
				;
				MessageBox(NULL,report.c_str(),"",0);
				*/

				c_ftp_client::SetError(FTP_CH_ERROR_UF_SEND_DATA);
				res=false;
				break;
			}
			DataPtr += TXSize;
			NumReadBytes-=TXSize;
			FileBytesWritedToServer+=TXSize;

		}
	}


	CloseDataPort();  // close accept

	return res;
}

BOOL __fastcall c_ftp_client::CheckError(BOOL close_on_err)
{
	if(cntrl_sock->GetError() != C_SOCK_NO_ERROR) {
		if(reply_buf[0] != 0) { // Check the reply buffer
		}
		if(close_on_err) {
			Close();
		}
		return false;
	}
	return true; // No errors reported
}

void __fastcall c_ftp_client::SetError(int SetError)
{
	//c_sock_compl::c_ftp_client::SetError(SetError);
	// чтобы первая ошибка не затиралась
	if(LastErrorFTP!=C_SOCK_NO_ERROR&&SetError!=C_SOCK_NO_ERROR) return;

    if(LastErrorFTP==C_SOCK_NO_ERROR)
    {
	    LastErrorFTP=SetError;
    }
	if(SetError==C_SOCK_NO_ERROR)
	{
        if(data_sock!=NULL) data_sock->ClearError();
		cntrl_sock->ClearError();        
	}	
}

#ifdef __BORLANDC__
// Borland C++ only functions
BOOL __fastcall c_ftp_client::FTPList(TStringList * Data)
{
	if(Data==NULL) return false;
	Data->Clear();

    BOOL res=false;

	char * Tempdata = new char [MAX_FTP_DIR_LIST_SIZE];
    if(Tempdata==NULL)  data_sock->c_ftp_client::SetError(FTP_CH_ERROR_MEM_ALLOC);

    Tempdata[0]=0;
	if(FTPList(Tempdata,MAX_FTP_DIR_LIST_SIZE,1))
	{
		Data->Text=Tempdata; // автоматом разбивается с delimiter="\r\n"
		res=true;
	}

    delete Tempdata;
	return res;
}

BOOL IsLinuxDir(AnsiString TmpStr)
{
    if(TmpStr.Length()<10) return false;
    if(TmpStr[1]!='d') return false;

    for(int i=1;i<=10;i++)
    {
        char CurrChar =TmpStr[i];
        switch(i)
        {
        case 0:
            if(CurrChar!='-'&&CurrChar!='d') return false;
            break;
        case 2:
        case 5:
        case 8:
            if(CurrChar!='-'&&CurrChar!='r') return false;
            break;
        case 3:
        case 6:
        case 9:
            if(CurrChar!='-'&&CurrChar!='w') return false;
            break;
        case 4:
        case 7:
        case 10:
            if(CurrChar!='-'&&CurrChar!='x') return false;
            break;
        default:
            break;
        }
    }
    return true;
}

BOOL __fastcall c_ftp_client::FTPDirList(TStringList * Data)
{
	if(FTPList(Data))
    {
        // remove entries without Dir attribute
        for(int i=0;i<Data->Count;i++)
        {
            AnsiString CurrStr=Data->Strings[i];
            if(CurrStr.Pos("<DIR>")!=0||
                IsLinuxDir(CurrStr))
            {
                // последнее слово - это название каталога
                __int32 LastSpacePos=CurrStr.LastDelimiter(' ');
                Data->Strings[i] =CurrStr.SubString(LastSpacePos+1,Data->Strings[i].Length()-LastSpacePos);

                if(Data->Strings[i]!="."
                   &&Data->Strings[i]!="..")
                {
                    continue;
                }
            }

            Data->Delete(i);
            i--;
        }
        return true;
    }

    return false;
}
#endif


// ----------------------------------------------------------- //
// ------------------------------- //
// --------- End of File --------- //
// ------------------------------- //
