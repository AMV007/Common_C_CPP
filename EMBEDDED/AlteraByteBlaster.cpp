#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#include "AlteraByteBlaster.h"

CAlteraByteBlaster::CAlteraByteBlaster()
{	
	hDevice=INVALID_HANDLE_VALUE;
	ParDev = new CParallel();
}

CAlteraByteBlaster::~CAlteraByteBlaster()
{	
	CloseDevice();
	delete ParDev;
}


bool  CAlteraByteBlaster::OpenDevice(int PortNumber)
{
	ParDev->OpenDevice(PortNumber);

	PortFlags=CLK_PIN|OUT_PIN;

	CloseDevice();

	int res=0;
	TCHAR DeviceName[20];
#if (defined(UNICODE)||defined(_UNICODE))
	#ifdef _MSC_VER
		swprintf_s(DeviceName,(sizeof(DeviceName)/sizeof(TCHAR)),L"\\\\.\\ALTLPT%d",PortNumber);	
	#else
		swprintf(DeviceName,L"\\\\.\\ALTLPT%d",PortNumber);	
	#endif
#else
	#ifdef _MSC_VER
		sprintf_s(DeviceName,(sizeof(DeviceName)/sizeof(TCHAR)),"\\\\.\\ALTLPT%d",PortNumber);	
	#else
		sprintf(DeviceName,"\\\\.\\ALTLPT%d",PortNumber);	
	#endif
#endif
	hDevice = CreateFile(
		DeviceName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if(hDevice!=INVALID_HANDLE_VALUE)
	{
		DWORD Buffer;
		DWORD ReturnLength;
		if (DeviceIoControl(
			hDevice,			/* Handle to device */
			PGDC_IOCTL_GET_DEVICE_INFO_PP,	/* IO Control code */
			(ULONG *)NULL,					/* Buffer to driver. */
			0,								/* Length of buffer in bytes. */
			&Buffer,						/* Buffer from driver. */
			sizeof(Buffer),					/* Length of buffer in bytes. */
			&ReturnLength,				/* Bytes placed in data_buffer. */
			NULL))							/* Wait for operation to complete */
		{
			if (ReturnLength == sizeof(Buffer))
			{
				if (Buffer == PGDC_HDLC_NTDRIVER_VERSION)
				{
					if(!WriteByteByteBlaster(2,2)) res=-1;
				}
				else res=-1;				
			}else res=-1;
		}else res=-1;		
	}else res=-1;

	if(res!=0)
	{
		CloseDevice();
		return false;
	} else return true;
}

void  CAlteraByteBlaster::CloseDevice()
{
	WriteByteByteBlaster(0,CS_PIN);		
	if(hDevice!=INVALID_HANDLE_VALUE)
	{		
		CloseHandle(hDevice);
		hDevice=INVALID_HANDLE_VALUE;		
	}
}

HANDLE  CAlteraByteBlaster::GetDeviceHandle(void)
{
	return hDevice;
}

//--------------------------------------------------------------------------------
// интерфейс byteblaster
//---------------------------------------------------------------------------------

bool  CAlteraByteBlaster::WriteByteByteBlaster(DWORD Pre, DWORD Data)
{
	if(hDevice==INVALID_HANDLE_VALUE) return false;

	BOOL status = FALSE;

	DWORD returned_length = 0;
	DWORD buffer[2];
	buffer[0]=Pre;
	buffer[1]=Data;
	status = DeviceIoControl(
		hDevice,			/* Handle to device */
		PGDC_IOCTL_WRITE_PORT_PP,	/* IO Control code for write */
		buffer,			/* Buffer to driver. */
		sizeof(buffer),			/* Length of buffer in bytes. */
		NULL,				/* Buffer from driver.  Not used. */
		0,							/* Length of buffer in bytes. */
		&returned_length,	/* Bytes returned.  Should be zero. */
		NULL);						/* Wait for operation to complete */

	if ((!status) || (returned_length != 0)) return false;	
	return true;
}

bool  CAlteraByteBlaster::ReadByteByteBlaster(DWORD Pre, DWORD * Data)
{
	if(hDevice==INVALID_HANDLE_VALUE) return false;

	BOOL status = FALSE;

	DWORD returned_length = 0;

	status = DeviceIoControl(
		hDevice,			/* Handle to device */
		PGDC_IOCTL_READ_PORT_PP,	/* IO Control code for Read */
		&Pre,				/* Buffer to driver. */
		sizeof(DWORD),				/* Length of buffer in bytes. */
		Data,				/* Buffer from driver. */
		sizeof(DWORD),				/* Length of buffer in bytes. */
		&returned_length,	/* Bytes placed in data_buffer. */
		NULL);						/* Wait for operation to complete */

	if ((!status) || (returned_length != sizeof(DWORD)))return false;		
	*Data&=0xFF;	
	return true;	
}

bool  CAlteraByteBlaster::WriteByteBlaster(BYTE * Data, DWORD NumData)
{
	if(hDevice==INVALID_HANDLE_VALUE) return false;	

	_PORT_IO_LIST_STRUCT * SendData = new _PORT_IO_LIST_STRUCT[NumData];
	if(SendData==NULL) return false;

	for (DWORD i=0;i<NumData;i++)
	{
		SendData[i].Command=PGDC_WRITE_PORT;
		SendData[i].Data=Data[i];
	}
	
	BOOL status = FALSE;
	DWORD returned_length = 0;
	
	status = DeviceIoControl(
		hDevice,			//Handle to device 
		PGDC_IOCTL_PROCESS_LIST_PP,	// IO Control code for write 
		SendData,			// Buffer to driver. 
		NumData*sizeof(DWORD),			// Length of buffer in bytes. 
		SendData,				// Buffer from driver.  Not used. 
		NumData*sizeof(DWORD),							// Length of buffer in bytes. 
		&returned_length,	// Bytes returned.  Should be zero. 
		NULL);						// Wait for operation to complete 

	delete SendData;
	if ((!status) || (returned_length != NumData*sizeof(DWORD))) return false;	
	return true;	
}

bool  CAlteraByteBlaster::ReadByteBlaster(BYTE * Data, DWORD NumData)
{
	if(hDevice==INVALID_HANDLE_VALUE) return false;

	_PORT_IO_LIST_STRUCT * ReadData = new _PORT_IO_LIST_STRUCT[NumData];
	if(ReadData==NULL) return false;

	for (DWORD i=0;i<NumData;i++)
	{
		ReadData[i].Command=PGDC_READ_PORT;
		ReadData[i].Data=0;
	}
	
	BOOL status = FALSE;
	DWORD returned_length = 0;

	status = DeviceIoControl(
		hDevice,			/* Handle to device */
		PGDC_IOCTL_PROCESS_LIST_PP,	/* IO Control code for write */
		ReadData,			/* Buffer to driver. */
		NumData*sizeof(DWORD),			/* Length of buffer in bytes. */
		ReadData,				/* Buffer from driver.  Not used. */
		NumData*sizeof(DWORD),							/* Length of buffer in bytes. */
		&returned_length,	/* Bytes returned.  Should be zero. */
		NULL);						/* Wait for operation to complete */

	for (DWORD i=0;i<NumData;i++)
	{
		Data[i]=(BYTE)(ReadData[i].Data&0xFF);
	}

	delete ReadData;
	if ((!status) || (returned_length != NumData*sizeof(DWORD))) return false;	
	return true;	
}


//---------------------------------------------------------------------------------
// интерфейс SPI
//---------------------------------------------------------------------------------

bool CAlteraByteBlaster::WriteSPIData(BYTE * Data, DWORD NumData)
{
	if(hDevice==INVALID_HANDLE_VALUE) return false;

	DWORD NumValues = NumData*2*8;
	_PORT_IO_LIST_STRUCT * SendData = new _PORT_IO_LIST_STRUCT[NumValues];
	PortFlags|=(OUT_PIN|CLK_PIN);
	for (DWORD i=0, Counter=0;i<NumData;i++)
	{
		for(int j=7;j>=0;j--,Counter+=2)
		{		
			if(Data[i]&(1<<j)) PortFlags|=OUT_PIN;		
			else			PortFlags&=~OUT_PIN;			
			
			PortFlags&=~CLK_PIN;
			SendData[Counter].Command=PGDC_WRITE_PORT;
			SendData[Counter].Data=PortFlags;

			PortFlags|=CLK_PIN;
			SendData[Counter+1].Command=PGDC_WRITE_PORT;
			SendData[Counter+1].Data=PortFlags;			
		}
	}	
	
	BOOL status = FALSE;
	DWORD returned_length = 0;
	
	status = DeviceIoControl(
		hDevice,			//Handle to device 
		PGDC_IOCTL_PROCESS_LIST_PP,	// IO Control code for write 
		SendData,			// Buffer to driver. 
		NumValues*sizeof(DWORD),			// Length of buffer in bytes. 
		SendData,				// Buffer from driver.  Not used. 
		NumValues*sizeof(DWORD),							// Length of buffer in bytes. 
		&returned_length,	// Bytes returned.  Should be zero. 
		NULL);						// Wait for operation to complete 

	
	if(SendData!=NULL) delete SendData;
	PortFlags|=(OUT_PIN|CLK_PIN);
	if ((!status) || (returned_length != NumValues*sizeof(DWORD))) return false;	
	return true;	
}

bool CAlteraByteBlaster::ReadSPIData(BYTE * ReadData, DWORD NumData)
{	
	if(hDevice==INVALID_HANDLE_VALUE) return false;
	
	DWORD NumValues = NumData*3*8;
	_PORT_IO_LIST_STRUCT *Data = new _PORT_IO_LIST_STRUCT[NumValues];	

	PortFlags|=(OUT_PIN|CLK_PIN);
	for (DWORD i=0;i<NumValues;i+=3)
	{
		PortFlags&=~CLK_PIN;
		Data[i].Command=PGDC_WRITE_PORT;
		Data[i].Data=PortFlags;

		PortFlags|=CLK_PIN;
		Data[i+1].Command=PGDC_WRITE_PORT;
		Data[i+1].Data=PortFlags;
		
		Data[i+2].Command=PGDC_READ_PORT;
		Data[i+2].Data=0;
	}
	
	BOOL status = FALSE;
	DWORD returned_length = 0;
	status = DeviceIoControl(
		GetDeviceHandle(),			
		PGDC_IOCTL_PROCESS_LIST_PP,	
		Data,			
		NumValues*sizeof(DWORD),	
		Data,			
		NumValues*sizeof(DWORD),	
		&returned_length,
		NULL);			
	
	for (DWORD Counter=0, i=0;i<NumData;i++)
	{		
		ReadData[i]=0;
		for (int j=7;j>=0;j--,Counter+=3)
		{
			if(!(Data[Counter+2].Data&IN_PIN)) ReadData[i]|=(1<<j);				
		}
	}
	
	if(Data!=NULL) delete Data;
	if ((!status) || (returned_length != NumValues*sizeof(DWORD))) return false;	
	return true;		
}

bool CAlteraByteBlaster::WriteSPIByte(BYTE Data)
{	
	return WriteSPIData(&Data,1);
}

bool CAlteraByteBlaster::ReadSPIByte(BYTE & ReadData)
{	
	return ReadSPIData(&ReadData,1);
}

