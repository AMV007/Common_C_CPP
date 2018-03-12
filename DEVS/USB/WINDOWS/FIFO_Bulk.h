#ifndef __C_FIFO_BULK_H__
#define __C_FIFO_BULK_H__

#include "..\\..\\..\\Common_C_CPP.h"
#include "..\\..\\..\\COMMON\\c_FIFO.h"
#include "..\\..\\..\\COMMON\\c_thread.h"

//-----------------------------------------------
class TReadFIFOThread:public c_thread
{
protected:
	HANDLE DeviceHandle;
	int ReadQueueSize;
	int ReadQueueNumber;

	BYTE * ReadDataQueue;				// data for read USB request
	OVERLAPPED * ReadQueueOverlapped;	// overlapped array queue

	void Execute(void);

public :	
	TReadFIFOThread(HANDLE dHandle, DWORD ReadNumberQueue,
						DWORD ReadSizeBytesQueue, DWORD ReadFIFO_Size);
	~TReadFIFOThread(void);		

	c_FIFO<BYTE> * FifoRead;	
};
//-----------------------------------------------------
class TWriteFIFOThread:public c_thread
{
protected:
	HANDLE DeviceHandle;
	int WriteQueueSize;
	int WriteQueueNumber;

	BYTE * WriteDataQueue;				// data for write USB request
	OVERLAPPED * WriteQueueOverlapped;	// overlapped array queue

	void Execute(void);

public :	
	TWriteFIFOThread(HANDLE dHandle, DWORD WriteNumberQueue,
						DWORD WriteSizeBytesQueue, DWORD WriteFIFO_Size);
	~TWriteFIFOThread(void);	

	c_FIFO<BYTE> * FifoWrite;
};
//-----------------------------------------------
class COMMON_C_CPP_API TFIFO_BULK
{
 protected :
	TReadFIFOThread * ReadFIFOThread;
	TWriteFIFOThread * WriteFIFOThread;			
	
	HANDLE DeviceHandle;
		
 public :
	
    // коструктор/деструктор
    TFIFO_BULK(HANDLE dHandle ,	DWORD FIFOReadSizeByte, DWORD FifoWriteSizeByte, 
								DWORD ReadNumberQueue,	DWORD WriteNumberQueue, 
								DWORD ReadSizeBytesQueue, DWORD WriteSizeBytesQueue);
    ~TFIFO_BULK(void);	
    
	// common functions
	virtual void StartFIFO(void);
	virtual void Flush(void);
	virtual void StopFIFO(void);
	virtual DWORD __fastcall GetReadFIFOCounter(void);	
	virtual DWORD __fastcall GetReadFIFOSize(void);		
	virtual DWORD __fastcall GetWriteFIFOCounterFree(void);	
	virtual DWORD __fastcall GetWriteFIFOSize(void);		
	virtual void __fastcall GetFIFORunningState(DWORD * ReadFIFORunning, DWORD * WriteFIFORunning);

	virtual bool __fastcall ReadBulkFIFO(BYTE * Data, DWORD DataLength, DWORD * BytesReaded, DWORD Timeout);	
	virtual bool __fastcall WriteBulkFIFO(BYTE * Data, DWORD DataLength, DWORD * BytesWritten, DWORD Timeout);	
	
	int LastErrorNumber;		
};



#endif //__C_FIFO_BULK_H__