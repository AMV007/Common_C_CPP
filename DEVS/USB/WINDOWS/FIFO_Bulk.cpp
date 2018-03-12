#include "stdafx.h"

#include "USB_defs.h"
#include "LUSB.h"
#include "FIFO_Bulk.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//--------------------------------------------------------------------------------------------------
// WriteThread

TWriteFIFOThread::TWriteFIFOThread(HANDLE dHandle, DWORD WriteNumberQueue,
								   DWORD WriteSizeBytesQueue, DWORD WriteFIFO_Size)
{	
	WriteQueueSize=WriteSizeBytesQueue;
	WriteQueueNumber=WriteNumberQueue;

	WriteDataQueue = new BYTE[WriteQueueNumber*WriteQueueSize];

	WriteQueueOverlapped = new OVERLAPPED[WriteQueueNumber];	

	DeviceHandle =dHandle;
	
	
	// FIFO size must be multiply of buffer size
	WriteFIFO_Size = (WriteFIFO_Size/WriteSizeBytesQueue)*WriteSizeBytesQueue;

	// checking, that request size is not larger than FIFO size
	WriteFIFO_Size = max(WriteFIFO_Size,(DWORD)WriteQueueSize); 
	FifoWrite = new c_FIFO<BYTE>(WriteFIFO_Size);
}

TWriteFIFOThread::~TWriteFIFOThread()
{
	Stop();
	if(!WaitFor())
	{
		ForceStop();
	}

	delete FifoWrite;
	FifoWrite=NULL;
	delete WriteDataQueue;	
	WriteDataQueue=NULL;
	delete WriteQueueOverlapped;
	WriteQueueOverlapped=NULL;
}

void TWriteFIFOThread::Execute()
{		
	memset(WriteQueueOverlapped,0,sizeof(OVERLAPPED)*WriteQueueNumber);
	for (int i=0;i<WriteQueueNumber;i++)
	{			
		WriteQueueOverlapped[i].hEvent=CreateEvent(NULL, FALSE, TRUE, NULL);						
	}

	int Index=0;	
	DWORD Size=0;
	while (!Stopped)
	{
		if (WaitForSingleObject(WriteQueueOverlapped[Index].hEvent, 500)==WAIT_OBJECT_0)
		{
			if (!GetOverlappedResult(DeviceHandle,&WriteQueueOverlapped[Index], &Size,FALSE)) Stop();					

			while (!Stopped)
			{
				DWORD SizeWrite = FifoWrite->GetCounter();

				if (SizeWrite>=BULK_ENDPOINT_SIZE)
				{
					SizeWrite=min((DWORD)WriteQueueSize, SizeWrite);
					SizeWrite&=~0x1ff; // должно быть кратно 512, иначе мы ставим не кратно а DSP ставит кратно и все зависает
					FifoWrite->Read(&WriteDataQueue[Index*WriteQueueSize],SizeWrite);		

					if (!WriteFile(DeviceHandle,&WriteDataQueue[Index*WriteQueueSize], SizeWrite,&Size,&WriteQueueOverlapped[Index]))
					{
						if (GetLastError()!=ERROR_IO_PENDING)Stop();
					}

					Index=(++Index)%WriteQueueNumber;
					break;
				}
				else Sleep(5);
			}						
		}
	}	

	CancelIo(DeviceHandle);

	for (int i=0;i<WriteQueueNumber;i++)
	{		
		CloseHandle(WriteQueueOverlapped[i].hEvent);
	}	
}

//--------------------------------------------------------------------------------------------------
// ReadThread

TReadFIFOThread::TReadFIFOThread(HANDLE dHandle, DWORD ReadNumberQueue,
								 DWORD ReadSizeBytesQueue, DWORD ReadFIFO_Size)
{	
	DeviceHandle = dHandle;		
	ReadQueueNumber=ReadNumberQueue;	
	ReadQueueSize=ReadSizeBytesQueue=ReadSizeBytesQueue&~0x1ff; // must be multiply of 512 byte, due to USB restrictions;

	ReadDataQueue = new BYTE[ReadQueueNumber*ReadQueueSize];

	ReadQueueOverlapped = new OVERLAPPED[ReadQueueNumber];

	// FIFO size must be multiply of buffer size
	ReadFIFO_Size = (ReadFIFO_Size/ReadSizeBytesQueue)*ReadSizeBytesQueue;

	// checking, that request size is not larger than FIFO size
	ReadFIFO_Size = max(ReadFIFO_Size,(DWORD)ReadQueueSize);
	FifoRead = new c_FIFO<BYTE>(ReadFIFO_Size);
}

TReadFIFOThread::~TReadFIFOThread()
{
	Stop();
	if(!WaitFor())
	{
		ForceStop();
	}

	delete FifoRead;
	delete ReadDataQueue;	
	delete ReadQueueOverlapped;
}

void TReadFIFOThread::Execute()
{
	// ставим все запросы в очередь		
	memset(ReadQueueOverlapped,0,sizeof(OVERLAPPED)*ReadQueueNumber);
	for (int i=0;i<ReadQueueNumber ;i++)
	{			
		ReadQueueOverlapped[i].hEvent=CreateEvent(NULL, FALSE, TRUE , NULL);				
	}

	int Index=0;	
	while (!Stopped)
	{
		if (WaitForSingleObject(ReadQueueOverlapped[Index].hEvent, 500)==WAIT_OBJECT_0)
		{
			DWORD Size=0;
			if (!GetOverlappedResult(DeviceHandle,&ReadQueueOverlapped[Index],&Size,FALSE))Stop();			

			if (Size)
			{
				while (!Stopped)
				{
					if(FifoRead->GetCounterFree()<Size) Sleep(5);					
					else
					{
						FifoRead->Write(&ReadDataQueue[Index*ReadQueueSize],Size);
						break;
					}
				}		
			}				

			if (!ReadFile(DeviceHandle,&ReadDataQueue[Index*ReadQueueSize ],ReadQueueSize,&Size,&ReadQueueOverlapped[Index]))
			{
				if (GetLastError()!=ERROR_IO_PENDING) Stop();						
			}

			Index=(++Index)%ReadQueueNumber;
		}
	}	

	CancelIo(DeviceHandle);			

	for (int i=0;i<ReadQueueNumber ;i++)
	{	
		CloseHandle(ReadQueueOverlapped[i].hEvent);
	}	
}

//---------------------------------------------------------------------------------------
// FIFO_BULK
#define S500_ERROR_BULK_FIFO_EMPTY						-2003
#define S500_ERROR_BULK_FIFO_FULL						-2004

TFIFO_BULK::TFIFO_BULK(HANDLE dHandle ,	DWORD FIFOReadSizeByte,		DWORD FIFOWriteSizeByte, 
										DWORD ReadNumberQueue,		DWORD WriteNumberQueue, 
										DWORD ReadSizeBytesQueue,	DWORD WriteSizeBytesQueue)
{	
	DeviceHandle=dHandle;	

	ReadFIFOThread = new TReadFIFOThread(dHandle,ReadNumberQueue, ReadSizeBytesQueue, FIFOReadSizeByte);
	WriteFIFOThread = new TWriteFIFOThread(dHandle,WriteNumberQueue, WriteSizeBytesQueue, FIFOWriteSizeByte);	
}

TFIFO_BULK::~TFIFO_BULK()
{
	StopFIFO();
	delete ReadFIFOThread;
	delete WriteFIFOThread;	
}

void TFIFO_BULK::StartFIFO()
{
	StopFIFO();

	ReadFIFOThread->FifoRead->Flush();
	WriteFIFOThread->FifoWrite->Flush();

	ReadFIFOThread->Start();
	WriteFIFOThread->Start();
}

void TFIFO_BULK::Flush()
{
	if (!ReadFIFOThread->Stopped)
	{
		ReadFIFOThread->Stop();
		ReadFIFOThread->WaitFor();
		ReadFIFOThread->FifoRead->Flush();
		ReadFIFOThread->Start();
	}
	else
	{
		ReadFIFOThread->FifoRead->Flush();
	}

	if (!WriteFIFOThread->Stopped)
	{
		WriteFIFOThread->Stop();
		WriteFIFOThread->WaitFor();
		WriteFIFOThread->FifoWrite->Flush();
		WriteFIFOThread->Start();
	}
	else
	{
		WriteFIFOThread->FifoWrite->Flush();
	}	
}

void TFIFO_BULK::StopFIFO()
{
	ReadFIFOThread->Stop();
	WriteFIFOThread->Stop();			
		
	ReadFIFOThread->WaitFor();
	WriteFIFOThread->WaitFor();
}

bool TFIFO_BULK::ReadBulkFIFO(BYTE *Data, DWORD DataLength, DWORD * BytesReaded ,DWORD Timeout)
{
	LastErrorNumber=LUSB_OK;
	*BytesReaded=0;
	if(ReadFIFOThread->FifoRead==NULL) return false;

	DWORD tend=GetTickCount()+Timeout;
	while (DataLength&&(GetTickCount()<tend))
	{
		DWORD FIFOCounter=ReadFIFOThread->FifoRead->GetCounter();
		if (FIFOCounter)
		{
			DWORD SizeRecieved = min(FIFOCounter, DataLength);
			if (!ReadFIFOThread->FifoRead->Read(Data, SizeRecieved))
			{
				if (ReadFIFOThread->Stopped)LastErrorNumber=LUSB_ERROR_READ_FIFO_THREAD_DEAD;
				else LastErrorNumber = S500_ERROR_BULK_FIFO_EMPTY;
				return false;
			}

			Data+=SizeRecieved;
			DataLength-=SizeRecieved;
			*BytesReaded+=SizeRecieved;
		}
		else
		{
			if(ReadFIFOThread->Stopped)
			{
				LastErrorNumber=LUSB_ERROR_READ_FIFO_THREAD_DEAD;
				return false;
			}
			Sleep(5);
		}
	}

	return true;
}

bool TFIFO_BULK::WriteBulkFIFO(BYTE *Data, DWORD DataLength, DWORD * BytesWritten, DWORD Timeout)
{
	LastErrorNumber=LUSB_OK;
	*BytesWritten=0;	

	DWORD tend = GetTickCount()+Timeout;

	while (DataLength&&(GetTickCount()<tend))
	{
		DWORD FIFOFreeCounter=WriteFIFOThread->FifoWrite->GetCounterFree();
		if (FIFOFreeCounter)
		{
			DWORD SizeWritted = min(FIFOFreeCounter, DataLength);
			if (!WriteFIFOThread->FifoWrite->Write(Data, SizeWritted))
			{
				if (WriteFIFOThread->Stopped)LastErrorNumber=LUSB_ERROR_WRITE_FIFO_THREAD_DEAD;	
				else LastErrorNumber = S500_ERROR_BULK_FIFO_FULL;
				return false;
			}
			Data+=SizeWritted;
			DataLength-=SizeWritted;
			*BytesWritten+=SizeWritted;
		}
		else
		{
			if(WriteFIFOThread->Stopped)
			{
				LastErrorNumber=LUSB_ERROR_WRITE_FIFO_THREAD_DEAD;
				return false;
			}
			Sleep(5);
		}
	}

	return true;
}

DWORD TFIFO_BULK::GetReadFIFOCounter()
{
	return ReadFIFOThread->FifoRead->GetCounter();
}

DWORD TFIFO_BULK::GetReadFIFOSize()
{
	return ReadFIFOThread->FifoRead->GetCapacityBytes();
}

DWORD TFIFO_BULK::GetWriteFIFOSize()
{
	return WriteFIFOThread->FifoWrite->GetCapacityBytes();
}

DWORD TFIFO_BULK::GetWriteFIFOCounterFree()
{
	return WriteFIFOThread->FifoWrite->GetCounterFree();
}

void TFIFO_BULK::GetFIFORunningState(DWORD  * ReadFIFORunning, DWORD * WriteFIFORunning)
{
	*ReadFIFORunning=!ReadFIFOThread->Stopped;
	*WriteFIFORunning=!WriteFIFOThread->Stopped;
}


#ifdef _MANAGED
#pragma managed(pop)
#endif