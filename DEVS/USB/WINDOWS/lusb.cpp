#include "stdafx.h"
#include <stdio.h>
#include <algorithm>
#include "winioctl.h"
#include "..\\..\\..\\COMMON\\Compatability.h"
#include "USB_defs.h"
#include "LUSB.h"

#define DIOC_SEND_COMMAND \
      CTL_CODE(FILE_DEVICE_UNKNOWN, 15, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

TLUSB_BASE::TLUSB_BASE()
{	
	ControlPipeInUse = NULL;
	WriteBulkInUse=NULL;
	ReadBulkInUse=NULL;

	hDevice = INVALID_HANDLE_VALUE;
	LastErrorNumber=LUSB_OK;
	memset(&ControlOverlapped, 0, sizeof(OVERLAPPED));
	memset(&ReadBulkOverlapped, 0, sizeof(OVERLAPPED));
	memset(&WriteBulkOverlapped, 0, sizeof(OVERLAPPED));
	ControlOverlapped.hEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
	ReadBulkOverlapped.hEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
	WriteBulkOverlapped.hEvent=CreateEvent(NULL, FALSE, FALSE, NULL);

	ControlPipeInUse = CreateMutex(NULL,false,NULL);
	WriteBulkInUse= CreateMutex(NULL,false,NULL);
	ReadBulkInUse= CreateMutex(NULL,false,NULL);
}

TLUSB_BASE::~TLUSB_BASE()
{
	CancelIo();

	CloseLDevice();	

	if (ControlOverlapped.hEvent!=NULL)
	{
		::CloseHandle(ControlOverlapped.hEvent);
		ControlOverlapped.hEvent=NULL;
	}

	if (ReadBulkOverlapped.hEvent!=NULL)
	{
		::CloseHandle(ReadBulkOverlapped.hEvent);
		ReadBulkOverlapped.hEvent=NULL;
	}

	if (WriteBulkOverlapped.hEvent!=NULL)
	{
		CloseHandle(WriteBulkOverlapped.hEvent);
		WriteBulkOverlapped.hEvent=NULL;
	}

	if (ControlPipeInUse!=NULL)
	{
		CloseHandle(ControlPipeInUse);
		ControlPipeInUse=NULL;
	}

	if (WriteBulkInUse!=NULL)
	{
		CloseHandle(WriteBulkInUse);
		WriteBulkInUse=NULL;
	}

	if (ReadBulkInUse!=NULL)
	{
		CloseHandle(ReadBulkInUse);
		ReadBulkInUse=NULL;
	}
}

HANDLE __fastcall TLUSB_BASE::GetUSBDeviceHandle()
{
	return hDevice;
}

bool __fastcall TLUSB_BASE::OpenLDevice(BYTE VirtualSlot)
{
	LastErrorNumber=LUSB_OK;

	CloseLDevice();

	if (LastErrorNumber==LUSB_OK)
	{
		CHAR DeviceName[18];
		memset (&DeviceName,0,sizeof(DeviceName));
		// формируем название драйвера USB
#ifdef  __BORLANDC__
//		sprintf(DeviceName, "\\\\.\\LDevUsb%d", VirtualSlot);
		sprintf(DeviceName, "\\\\.\\LDev%d", VirtualSlot);
#else
//      sprintf_s<sizeof(DeviceName)>(DeviceName, "\\\\.\\LDevUsb%d", VirtualSlot);
		sprintf_s<sizeof(DeviceName)>(DeviceName, "\\\\.\\LDev%d", VirtualSlot);
#endif

		OpenLDeviceByName((LPTSTR)DeviceName);        
	}

	//
	return LastErrorNumber!=LUSB_OK ? false : true;
}

bool __fastcall TLUSB_BASE::OpenLDeviceByParam(const CHAR * Name, BYTE VirtualSlot)
{
	CHAR DeviceName[32];	
#ifdef  __BORLANDC__
		sprintf(DeviceName, "\\\\.\\%s%d",Name, VirtualSlot);
#else
		sprintf_s<sizeof(DeviceName)>(DeviceName, "\\\\.\\%s%d",Name, VirtualSlot);
#endif
	return OpenLDeviceByName((LPTSTR)DeviceName);     	
}

bool __fastcall TLUSB_BASE::OpenLDeviceAny(const CHAR *Name)
{
	if(Name==NULL) return false;

	for(BYTE i=0;i<256;i++)
	{		
		if(OpenLDeviceByParam(Name,i)) return true;
	}
	return false;
}

bool __fastcall TLUSB_BASE::OpenLDeviceByName(LPTSTR DeviceName)
{
	LastErrorNumber=LUSB_OK;

	if(DeviceName!=NULL && strlen((CHAR *)DeviceName))
	{
		CloseLDevice();
		// откроем идентификатор для модуля
		hDevice=CreateFile(DeviceName, GENERIC_READ|GENERIC_WRITE, 0, 
			NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if(hDevice<0||hDevice==INVALID_HANDLE_VALUE) 
		{
		    hDevice = INVALID_HANDLE_VALUE;
		    LastErrorNumber=LUSB_ERROR_OPEN_DEVICE;
		}        
		else
		{
			LCardUSBDev=false;
			if(my_wa_strlen(DeviceName)>=4)
			{
				if(strstr((const char *)DeviceName,"LDev")) // current device is LCARD Device
					LCardUSBDev=true;
			}			
		}

	} else LastErrorNumber=LUSB_ERROR_INVALID_PARAMETERS;

	//
	return LastErrorNumber!=LUSB_OK ? false : true;
}

bool __fastcall TLUSB_BASE::CloseLDevice()
{
	LastErrorNumber=LUSB_OK;

	if(hDevice!=INVALID_HANDLE_VALUE) 
	{
		if(!CloseHandle(hDevice)) LastErrorNumber=LUSB_ERROR_CLOSE_DEVICE;
		hDevice=INVALID_HANDLE_VALUE;
	}	

	return LastErrorNumber!=LUSB_OK ? false : true;
}

bool __fastcall TLUSB_BASE::CancelIo()
{
	LastErrorNumber=LUSB_OK;
	if(hDevice!=INVALID_HANDLE_VALUE) 
	{		
		if (!::CancelIo(hDevice)) LastErrorNumber=LUSB_ERROR_CANCEL_IO;		
	}else LastErrorNumber=LUSB_ERROR_DEVICE_CLOSED;
	return LastErrorNumber!=LUSB_OK ? false : true;
}

bool __fastcall TLUSB_BASE::ControlLDevice(DWORD ControlCode, _ControlRequest * CurrentRequest, BYTE DataBuffer[], DWORD DataBufferLength, DWORD Timeout)
{
	if (WaitForSingleObject(ControlPipeInUse, INFINITE)==WAIT_OBJECT_0)
	{
		LastErrorNumber=LUSB_OK;
		DWORD BytesReturned = 0;
		if(LCardUSBDev)
		{
			ControlCode = DIOC_SEND_COMMAND; // всегда
		}
		if(hDevice!=INVALID_HANDLE_VALUE) 
		{
			if (!DeviceIoControl(hDevice, ControlCode, CurrentRequest, sizeof(_ControlRequest),
				DataBuffer, DataBufferLength, &BytesReturned, &ControlOverlapped))
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(ControlOverlapped.hEvent,Timeout)!=WAIT_OBJECT_0)
					{
						::CancelIo(hDevice);
						LastErrorNumber=LUSB_ERROR_TIMEOUT_EXPIRED;
					}                    

					if (!GetOverlappedResult(hDevice, &ControlOverlapped, &BytesReturned, false))
					{
						LastErrorNumber=LUSB_ERROR_ASYNC_ERROR;
					}                                        
				}
				else
				{
					LastErrorNumber=LUSB_ERROR_ASYNC_DATA_TRNSFER;
				}
			}

			if (BytesReturned != DataBufferLength&&LastErrorNumber==LUSB_OK)
			{
				LastErrorNumber=LUSB_ERROR_ASYNC_DATA_TRNSFER_LENGTH;
			}
		}else LastErrorNumber=LUSB_ERROR_DEVICE_CLOSED;

		ReleaseMutex(ControlPipeInUse);
	}else LastErrorNumber = LUSB_ERROR_WAIT_MUTEX;

	return LastErrorNumber!=LUSB_OK ? false : true;
}  

bool __fastcall TLUSB_BASE::CommandControl(_ControlRequest *CurrentRequest)
{
	DWORD ControlCode = DIOC_SEND_COMMAND; // всегда
	BYTE DataBuffer[1];
	return ControlLDevice(ControlCode,CurrentRequest,DataBuffer,0,1000);
}

bool __fastcall TLUSB_BASE::WriteBulk(BYTE DataBuffer[], DWORD DataBufferLength, DWORD * BytesWritten, DWORD Timeout)
{
	if(BytesWritten==NULL) return false;

	if (WaitForSingleObject(WriteBulkInUse,INFINITE)==WAIT_OBJECT_0)
	{

		LastErrorNumber=LUSB_OK;
		*BytesWritten = 0;
		if(hDevice!=INVALID_HANDLE_VALUE) 
		{

			if (!WriteFile(hDevice, DataBuffer, DataBufferLength, BytesWritten, &WriteBulkOverlapped))
			{
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(WriteBulkOverlapped.hEvent,Timeout)!=WAIT_OBJECT_0)
					{
						::CancelIo(hDevice);
						LastErrorNumber=LUSB_ERROR_TIMEOUT_EXPIRED;
					}                    

					if (!GetOverlappedResult(hDevice, &WriteBulkOverlapped, BytesWritten, false))
					{
						LastErrorNumber=LUSB_ERROR_ASYNC_ERROR;
					}
				}else LastErrorNumber=LUSB_ERROR_ASYNC_DATA_TRNSFER;                          
			}            

			if (*BytesWritten != DataBufferLength) LastErrorNumber=LUSB_ERROR_ASYNC_DATA_TRNSFER_LENGTH;
		}else LastErrorNumber=LUSB_ERROR_DEVICE_CLOSED;

		ReleaseMutex(WriteBulkInUse);
	} else LastErrorNumber = LUSB_ERROR_WAIT_MUTEX;

	return LastErrorNumber!=LUSB_OK ? false : true;
}

bool __fastcall TLUSB_BASE::ReadBulk(BYTE DataBuffer[], DWORD DataBufferLength, DWORD * BytesRead, DWORD Timeout)
{
	if(BytesRead==NULL) return false;

	if(WaitForSingleObject(ReadBulkInUse,INFINITE)==WAIT_OBJECT_0)
	{

		LastErrorNumber=LUSB_OK;
		*BytesRead=0;		

		if(hDevice!=INVALID_HANDLE_VALUE) 
		{
			if (!ReadFile(hDevice, DataBuffer, DataBufferLength, BytesRead, &ReadBulkOverlapped))
			{			
				if (GetLastError() == ERROR_IO_PENDING)
				{
					if (WaitForSingleObject(ReadBulkOverlapped.hEvent,Timeout)!=WAIT_OBJECT_0)
					{
						::CancelIo(hDevice);
						LastErrorNumber=LUSB_ERROR_TIMEOUT_EXPIRED;
					}                    

					if (!GetOverlappedResult(hDevice, &ReadBulkOverlapped, BytesRead, false))
					{
						LastErrorNumber=LUSB_ERROR_ASYNC_ERROR;
					}
				} else LastErrorNumber=LUSB_ERROR_ASYNC_DATA_TRNSFER;                            
			}            

			if (*BytesRead != DataBufferLength) LastErrorNumber=LUSB_WARNING_ASYNC_DATA_TRNSFER_LENGTH;

		}else LastErrorNumber=LUSB_ERROR_DEVICE_CLOSED;

		ReleaseMutex(ReadBulkInUse);
	}else LastErrorNumber = LUSB_ERROR_WAIT_MUTEX;
	
	return (LastErrorNumber==LUSB_OK||LastErrorNumber==LUSB_WARNING_ASYNC_DATA_TRNSFER_LENGTH) ? true : false;
}


bool __fastcall TLUSB_BASE::GetLastErrorString(LPTSTR lpBuffer, DWORD nSize)
{
	// not realized
	return LastErrorNumber!=LUSB_OK ? false : true;
}

int __fastcall TLUSB_BASE::GetLastErrorNum()
{
	return LastErrorNumber;
}

bool __fastcall TLUSB_BASE::GetModuleName(CHAR mname[], DWORD Length)
{
	if(mname!=NULL) 
	{
		if(hDevice!=INVALID_HANDLE_VALUE) 
		{

			_ControlRequest Request;
			Request.RequestDirectionIN=true;
			Request.bRequest=11;
			Request.wValue=0;
			Request.wIndex=0;

			memset(mname,0,Length);			
			ControlLDevice(0,&Request,(BYTE *) mname, Length, 200);		
		}else LastErrorNumber=LUSB_ERROR_DEVICE_CLOSED;
	} else LastErrorNumber=LUSB_ERROR_INVALID_PARAMETERS;
	return LastErrorNumber!=LUSB_OK ? false : true;
}

bool __fastcall TLUSB_BASE::ReadControl(_ControlRequest *CurrentRequest, BYTE DataBuffer[], DWORD DataBufferLength, DWORD Timeout)
{
	LastErrorNumber=LUSB_OK;

	if (DataBuffer==NULL||DataBufferLength==NULL) LastErrorNumber=LUSB_ERROR_INVALID_PARAMETERS;

	CurrentRequest->RequestDirectionIN=true;

	DWORD SizeSended=0;

	while (LastErrorNumber==LUSB_OK&&SizeSended<DataBufferLength)
	{
		DWORD Size=std::min<DWORD>(DataBufferLength,MAX_CONTROL_TRANSFER_SIZE);

		if (ControlLDevice(0,CurrentRequest,DataBuffer+SizeSended, Size, Timeout)) SizeSended+=Size;		
	}

	return LastErrorNumber!=LUSB_OK ? false : true;
}

bool __fastcall TLUSB_BASE::WriteControl(_ControlRequest *CurrentRequest, BYTE DataBuffer[], DWORD DataBufferLength, DWORD Timeout)
{
	LastErrorNumber=LUSB_OK;

	if (DataBuffer==NULL||DataBufferLength==NULL) LastErrorNumber=LUSB_ERROR_INVALID_PARAMETERS;

	CurrentRequest->RequestDirectionIN=false;

	DWORD SizeSended=0;

	while (LastErrorNumber==LUSB_OK&&SizeSended<DataBufferLength)
	{
		DWORD Size=std::min<DWORD>(DataBufferLength,MAX_CONTROL_TRANSFER_SIZE);

		if (ControlLDevice(0,CurrentRequest,DataBuffer+SizeSended, Size, Timeout)) SizeSended+=Size;		
	}

	return LastErrorNumber!=LUSB_OK ? false : true;
}



