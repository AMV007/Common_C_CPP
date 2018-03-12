
#ifndef __ALTERA_BYTEBLASTER__H
#define __ALTERA_BYTEBLASTER__H

#include "..\\DEVS\\PORT\\PARALLEL\\WINDOWS\\Parallel.h"
// комманды драйвера
#define PGDC_IOCTL_GET_DEVICE_INFO_PP 0x00166A00L
#define PGDC_IOCTL_READ_PORT_PP       0x00166A04L
#define PGDC_IOCTL_WRITE_PORT_PP      0x0016AA08L
#define PGDC_IOCTL_PROCESS_LIST_PP    0x0016AA1CL
#define PGDC_READ_INFO                0x0a80
#define PGDC_READ_PORT                0x0a81
#define PGDC_WRITE_PORT               0x0a82
#define PGDC_PROCESS_LIST             0x0a87
#define PGDC_HDLC_NTDRIVER_VERSION    2

#pragma pack(1)
typedef struct 
{
	WORD Command;
	WORD Data;
} _PORT_IO_LIST_STRUCT;
#pragma pack()

// пины байт бластера
/*
0-clk
1 - out
2 - in
*/
#define CLK_PIN	(0x1)
#define CS_PIN	(0x2)
#define OUT_PIN	(0x40)
#define IN_PIN	(0x80)

class CAlteraByteBlaster
{
private:
	HANDLE hDevice;	
	CParallel * ParDev;
public:
	BYTE PortFlags;
	CAlteraByteBlaster();
	~CAlteraByteBlaster();

	HANDLE GetDeviceHandle(void);

	virtual void CloseDevice();
	virtual bool OpenDevice(int PortNumber);

	bool WriteByteByteBlaster(DWORD Pre, DWORD Data);
	bool ReadByteByteBlaster(DWORD Pre, DWORD * Data);
	bool WriteByteBlaster(BYTE * Data, DWORD NumData);
	bool ReadByteBlaster(BYTE * Data, DWORD NumData);

	bool WriteSPIByte(BYTE Data);
	bool ReadSPIByte(BYTE & Data);

	bool WriteSPIData(BYTE * Data, DWORD NumData);
	bool ReadSPIData(BYTE * Data, DWORD NumData);
};


#endif