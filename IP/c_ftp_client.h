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
// C++ Header File Name: gxsftp.h
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

File Transfer Protocol (FTP) classes used with applications
that require use of embedded FTP client/server functions.

Changes:
==============================================================
03/10/2002: The c_ftp_client class no longer uses the C++ fstream 
class as the underlying file system. The gxDatabase file pointer 
routines are now to used define the underlying file system used by 
the c_ftp_client class. This change was made to support large files 
and NTFS file system enhancements. To enable large file support 
users must define the __64_BIT_DATABASE_ENGINE__ preprocessor 
directive.

06/10/2002: Added gxsFTP_BUF_SIZE constant used to replace
gxsBUF_SIZE, which was previously used to set the fixed FTP 
replay buffer sizes. This change was made to accommodate larger 
fixed FTP reply buffers without changing the gxsFTP_BUF_SIZE 
constant.

06/10/2002: Added the c_ftp_client::passive_mode variable 
used internally to toggle passive FTP mode on and off.

06/10/2002: Added the c_ftp_client::FTPPassive() and 
c_ftp_client::FTPActive() functions used to toggle passive FTP
mode on and off.
==============================================================

*/
// ----------------------------------------------------------- // 
#ifndef __H_C_FTP_CLIENT__
#define __H_C_FTP_CLIENT__

#ifdef __BORLANDC__
	// borland only functions
    #include <classes.hpp>
#endif

#include "c_sock_compl.h"

// Set the reply buffer size
#define MAX_FTP_DIR_LIST_SIZE   (1024*64)

#define FTP_CH_ERROR_BUFOVER        (FTP_CH_ERROR_BASE -  1)
#define FTP_CH_ERROR_RECEIVE        (FTP_CH_ERROR_BASE -  2)
#define FTP_CH_ERROR_FTP            (FTP_CH_ERROR_BASE -  3)
#define FTP_CH_ERROR_DISCONNECT     (FTP_CH_ERROR_BASE -  4)
#define FTP_CH_ERROR_REQ_TIMEOUT    (FTP_CH_ERROR_BASE -  5)
#define FTP_CH_ERROR_MEM_ALLOC	    (FTP_CH_ERROR_BASE -  6)

#define FTP_CH_ERROR_DP_SEND		(FTP_CH_ERROR_BASE -  7)
#define FTP_CH_ERROR_DP_VALID		(FTP_CH_ERROR_BASE -  8)
#define FTP_CH_ERROR_DP_REPLY		(FTP_CH_ERROR_BASE -  9)
#define FTP_CH_ERROR_DP_CONN		(FTP_CH_ERROR_BASE - 10)
#define FTP_CH_ERROR_DP_BIND		(FTP_CH_ERROR_BASE - 11)
#define FTP_CH_ERROR_DP_HOST_INFO	(FTP_CH_ERROR_BASE - 12)
#define FTP_CH_ERROR_DP_SEND_F		(FTP_CH_ERROR_BASE - 13)

#define FTP_CH_ERROR_UF_SEND_STOR	(FTP_CH_ERROR_BASE - 14)
#define FTP_CH_ERROR_UF_ACCEPT		(FTP_CH_ERROR_BASE - 15)
#define FTP_CH_ERROR_UF_READ_FILE	(FTP_CH_ERROR_BASE - 16)
#define FTP_CH_ERROR_UF_SEND_DATA	(FTP_CH_ERROR_BASE - 17)

#define FTP_CH_ERROR_CONN			(FTP_CH_ERROR_BASE - 18)
#define FTP_CH_ERROR_CONN_RESP		(FTP_CH_ERROR_BASE - 19)

#define FTP_CH_ERROR_OPEN_FILE		(FTP_CH_ERROR_BASE - 20)

#define FTP_CH_ERROR_PARAM  		(FTP_CH_ERROR_BASE - 21)
#define FTP_CH_ERROR_OPEN_DATAPORT	(FTP_CH_ERROR_BASE - 22)

#define FTP_CH_ERROR_FTP_REPLY      (FTP_CH_ERROR_BASE - 23)


#define FTP_IO_BLOCK_SIZE       (8*1024)
#define FILE_IO_BLOCK_SIZE_MAX  (FTP_IO_BLOCK_SIZE*64)


// FTP client - See RFC 959 for specifications
class c_ftp_client
{
private:	

	// размер кешa для чтения файла перед его записью. в FTP
	int fileCacheSize;
	PBYTE FileIODataCache;
	int LastErrorFTP;	

	char port_buf[MAX_HOST_NAME_LEN];
	BOOL DataPortListening;
	BOOL passive_mode;  // True if this client is in passive FTP mode
    BOOL useoverlapped;

	int time_out_ms;  // Number of mseconds before a blocking timeout	

	Ic_sock * cntrl_sock;
	c_sock_compl * data_sock; // Socket used to transfer data to this client

    BOOL nonBlocingConn;
    __int32 conBindAccTimeout;

	void __fastcall SetError(int SetError);

public:
	c_ftp_client(BOOL Useoverlapped=FALSE);
	~c_ftp_client();

public:
	void __fastcall Close();
	// usually using
	BOOL __fastcall ConnectLoginDir(const char *host, const char * Login="anonymous", 
        const char * Pass="max@mail.ru", const char * Dir=NULL, WORD port = C_SOCK_FTP_PORT);
	BOOL __fastcall ConnectHost(const char *host, WORD port = C_SOCK_FTP_PORT);
	BOOL __fastcall SendCommand(const char *command, const char *response,
		const char *args = 0);
	BOOL __fastcall FTPLogin(const char *username, const char *password);
	BOOL __fastcall FTPLogout();
	BOOL __fastcall FTPImageType(char type = 'I');
	BOOL __fastcall FTPList(char *sbuf, unsigned bytes, __int32 full = 1,
		const char *args = 0);
	BOOL __fastcall FTPChDir(const char *dname);
    BOOL __fastcall FTPDirUp(){return FTPChDir("..");};
	BOOL __fastcall FTPMkDir(const char *dname);
	BOOL __fastcall FTPRmDir(const char *dname);
	BOOL __fastcall FTPPWD();
	BOOL __fastcall FTPStat();
	BOOL __fastcall FTPSize(const char *fname);
	BOOL __fastcall FTPDelete(const char *fname);
	BOOL __fastcall FTPMove(const char *from, const char *to);  

	BOOL __fastcall UploadFile(HANDLE hFile, const char * ServerFileName);
	BOOL __fastcall UploadFile(const TCHAR * FileName, const char * ServerFileName=NULL);

	BOOL __fastcall DownloadFile(HANDLE hFile, const char * ServerFileName);
	BOOL __fastcall DownloadFile(const TCHAR * FileName,const char * ServerFileName=NULL);
    BOOL __fastcall UploadFile(const char * Data, size_t DataLen, const char * ServerFileName);

	BOOL __fastcall SetMaxFileCacheSize(int CacheSizeMax); // maximum cache size for file reading

	BOOL __fastcall OpenDataPort();

	void __fastcall FTPPassiveSet() {FTPModeSet(TRUE);};
	void __fastcall FTPActiveSet() {FTPModeSet(FALSE);};

    void __fastcall FTPModeSet(int passive)
    // FTP mode 1= passive 0 - active
    {
            passive_mode = passive;
            DataPortListening=false;

            if(data_sock==NULL) return;
            data_sock->Close();
        };
    __int32  __fastcall FTPModeGet(){return passive_mode;}; // 0 - active 1 - passive
	BOOL  __fastcall FTPPassiveGet(){return passive_mode;}; // 0 - active 1 - passive
    BOOL  __fastcall FTPActiveGet(){return !passive_mode;}; // 0 - active 1 - passive
	int  __fastcall ReadDataPort(void *buf, unsigned bytes);
	int  __fastcall WriteDataPort(void *buf, unsigned bytes);
	void __fastcall CloseDataPort();
	BOOL __fastcall RecvResponse(char *buf, __int32 bytes, const char *response);
	BOOL __fastcall WaitForReply(SOCKET s);  

	void __fastcall SetExtParam(int WaitRecieve_ms,
                                __int32 ConBindAccTimeout=3000, BOOL NonBlockingConnection=false) {
		time_out_ms = WaitRecieve_ms;		
        conBindAccTimeout=ConBindAccTimeout;

        nonBlocingConn=NonBlockingConnection;

        cntrl_sock->SetSockTimeout(ConBindAccTimeout,ConBindAccTimeout,ConBindAccTimeout);

        if(data_sock==NULL) return;  
        data_sock->conn_bind_sock->SetSockTimeout(ConBindAccTimeout,ConBindAccTimeout,ConBindAccTimeout);
        data_sock->accept_sock->SetSockTimeout(ConBindAccTimeout,ConBindAccTimeout,ConBindAccTimeout);
	}

	BOOL __fastcall CheckError(int close_on_err);  	
	int  __fastcall GetErrorFTP(){return LastErrorFTP;};
	int  __fastcall GetCntrlError(){return cntrl_sock->GetError();};
	int  __fastcall GetCntrlWSAError(){return cntrl_sock->GetWSAError();};
	int  __fastcall GetDataError(){
            if(data_sock==NULL) return 0;
            return data_sock->GetError();
        };
	int  __fastcall GetDataWSAError(){
            if(data_sock==NULL) return 0;
            return data_sock->GetWSAError();
        };

#ifdef __BORLANDC__
	// borland only functions
    //#include <classes.hpp>
    BOOL __fastcall FTPList(TStringList * Data);
	BOOL __fastcall FTPDirList(TStringList * Data);
#endif

private:
	char command_buf[SOCK_COMMAND_BUF_SIZE];   // Buffer used to hold the last command  

public:

	int data_sock_port; // FTP data port client is listening on

	char reply_buf[SOCK_REPLY_BUF_SIZE]; // Buffer used to hold the last reply    
	// statistic
	volatile __int64 FileBytesWritedToServer;
	volatile __int64 FileBytesReadedFromServer;
};

#endif // __H_C_FTP_CLIENT__
// ----------------------------------------------------------- //
// ------------------------------- //
// --------- End of File --------- //
// ------------------------------- //
