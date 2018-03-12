#include "USB_defs.h"

#ifndef __LUSB__
#define __LUSB__


struct ILUSB_BASE
{
    // функции общего назначения
	virtual HANDLE __fastcall GetUSBDeviceHandle(void) = 0;
    virtual bool __fastcall OpenLDevice(BYTE VirtualSlot) = 0;   
	virtual bool __fastcall OpenLDeviceByName(LPTSTR DeviceName)=0;
	virtual bool __fastcall OpenLDeviceByParam(const char *Name, uint8_t VirtualSlot)=0;

	virtual bool __fastcall OpenLDeviceAny(const char *Name="LDev")=0;
	
    virtual bool __fastcall CloseLDevice(void) = 0;	

    virtual bool __fastcall ControlLDevice(uint32_t ControlCode, _ControlRequest * CurrentRequest, uint8_t DataBuffer[], uint32_t DataBufferLength, uint32_t Timeout)=0;
	virtual bool __fastcall CommandControl(_ControlRequest * CurrentRequest)=0;
	virtual bool __fastcall WriteControl(_ControlRequest * CurrentRequest,uint8_t DataBuffer[], uint32_t DataBufferLength, uint32_t Timeout)=0;
	virtual bool __fastcall ReadControl(_ControlRequest * CurrentRequest,uint8_t DataBuffer[], uint32_t DataBufferLength, uint32_t Timeout)=0;

	virtual bool __fastcall WriteBulk(uint8_t DataBuffer[], uint32_t DataBufferLength, uint32_t * BytesWritten, uint32_t Timeout)=0;
	virtual bool __fastcall ReadBulk(uint8_t DataBuffer[], uint32_t DataBufferLength, uint32_t * BytesRead, uint32_t Timeout)=0;

    virtual bool __fastcall CancelIo(void) = 0;
    // основные функции
    virtual bool __fastcall GetModuleName(char mname[], uint32_t Length) = 0;
    // функция выдачи строки с последней ошибкой
	virtual bool __fastcall GetLastErrorString(LPTSTR lpBuffer, uint32_t nSize) = 0;
	virtual __int32 __fastcall  GetLastErrorNum() = 0;
};
//-----------------------------------------------------------------------------
class TLUSB_BASE : public virtual ILUSB_BASE
{
 protected :
	HANDLE ControlPipeInUse;
	HANDLE WriteBulkInUse;
	HANDLE ReadBulkInUse; 

    HINSTANCE hInstance;                  // идентификатор модуля DLL
    HANDLE hDevice;                       // идентификатор устройства    
	OVERLAPPED ControlOverlapped, ReadBulkOverlapped, WriteBulkOverlapped;	

	bool LCardUSBDev;				     //  является ли устройство, устройством фирмя LCARD
	int  LastErrorNumber;                 // номер последней ошибки

    //
 public :

	

    // коструктор/деструктор
    TLUSB_BASE(void);
    virtual ~TLUSB_BASE(void);	

    // функции общего назначения
	virtual HANDLE __fastcall GetUSBDeviceHandle(void);
	virtual bool __fastcall OpenLDevice(BYTE VirtualSlot);
	virtual bool __fastcall OpenLDeviceByName(LPTSTR DeviceName);
	virtual bool __fastcall OpenLDeviceByParam(const char *Name, BYTE VirtualSlot);
	virtual bool __fastcall OpenLDeviceAny(const char *Name="LDev");
    virtual bool __fastcall CloseLDevice(void);
    
	virtual bool __fastcall ControlLDevice(DWORD ControlCode, _ControlRequest * CurrentRequest, BYTE DataBuffer[], DWORD DataBufferLength, DWORD Timeout);
	virtual bool __fastcall CommandControl(_ControlRequest * CurrentRequest);
	virtual bool __fastcall WriteControl(_ControlRequest * CurrentRequest,BYTE DataBuffer[], DWORD DataBufferLength, DWORD Timeout=INFINITE);
	virtual bool __fastcall ReadControl(_ControlRequest * CurrentRequest,BYTE DataBuffer[], DWORD DataBufferLength, DWORD Timeout=INFINITE);

	virtual bool __fastcall WriteBulk(BYTE DataBuffer[], DWORD DataBufferLength, DWORD * BytesWritten, DWORD Timeout=INFINITE);
	virtual bool __fastcall ReadBulk(BYTE DataBuffer[], DWORD DataBufferLength, DWORD * BytesRead, DWORD Timeout=INFINITE);

    virtual bool __fastcall CancelIo(void);
    // основные функции
    virtual bool __fastcall GetModuleName(CHAR mname[], DWORD Length);
    // функция выдачи строки с последней ошибкой
	virtual bool __fastcall GetLastErrorString(LPTSTR lpBuffer, DWORD nSize);
	virtual __int32 __fastcall  GetLastErrorNum();
};

#endif