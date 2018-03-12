#ifndef ATFLASH_H
#define ATFLASH_H


#include "AlteraByteBlaster.h"


// FLASH Read Command
#define FLASH_CMD_MAIN_MEMORY_PAGE_READ         0xD2
#define FLASH_CMD_CONTINIUOS_ARRAY_READ_LEGACY  0xE8
#define FLASH_CMD_CONTINIUOS_ARRAY_READ_LF      0x03
#define FLASH_CMD_CONTINIUOS_ARRAY_READ_HF      0x0B
#define FLASH_CMD_BUFFER1_READ_LF               0xD1
#define FLASH_CMD_BUFFER2_READ_LF               0xD3
#define FLASH_CMD_BUFFER1_READ_LEGACY           0xD4
#define FLASH_CMD_BUFFER2_READ_LEGACY           0xD6

// FLASH Write Command
#define FLASH_CMD_BUFFER1_WRITE                 0x84
#define FLASH_CMD_BUFFER2_WRITE                 0x87

#define FLASH_CMD_BUFFER1_PROGRAMM_WITH_ERASE   0x83
#define FLASH_CMD_BUFFER2_PROGRAMM_WITH_ERASE   0x86
#define FLASH_CMD_BUFFER1_PROGRAMM_NO_ERASE     0x88
#define FLASH_CMD_BUFFER2_PROGRAMM_NO_ERASE     0x89

#define FLASH_CMD_PAGE_ERASE                    0x81
#define FLASH_CMD_BLOCK_ERASE                   0x50
#define FLASH_CMD_SECTOR_ERASE                  0x7C

#define FLASH_CMD_MAIN_MEM_PAGE_PROGR_THRU_BUF1 0x82
#define FLASH_CMD_MAIN_MEM_PAGE_PROGR_THRU_BUF2 0x85

// FLASH_ADDITIONAL_COMMAND
#define FLASH_CMD_MAIN_MEM_TO_BUFFER1           0x53
#define FLASH_CMD_MAIN_MEM_TO_BUFFER2           0x55
#define FLASH_CMD_MAIN_MEM_TO_BUFFER1_COMP      0x60
#define FLASH_CMD_MAIN_MEM_TO_BUFFER2_COMP      0x61
#define FLASH_CMD_AUTO_PAGE_REWR_THRU_BUFFER1   0x58
#define FLASH_CMD_AUTO_PAGE_REWR_THRU_BUFFER2   0x59
#define FLASH_CMD_DEEP_POWER_DOWN               0xB9
#define FLASH_CMD_RESUME_FROM_DEEP_POWER_DOWN   0xAB
#define FLASH_CMD_STATUS_REGISTER_READ          0xD7
#define FLASH_CMD_GET_CHIP_ID                   0x9F
#define FLASH_CMD_SECTOR_PROTECTION_READ        0x32
#define FLASH_CMD_SECTOR_LOCKDOWN_READ          0x35
#define FLASH_CMD_SECURITY_READ                 0x77

#define FLASH_PAGE_SIZE	0x200
#define FLASH_PAGE_MASK	0x1ff

#define SET_FLASH_CS FlashCS(false)
#define CLR_FLASH_CS FlashCS(true)

typedef enum
{
	AT_Buffer1,
	AT_Buffer2
}_AT_FlashBuffers;



class CAT45DB161Flash: public CAlteraByteBlaster
{
private:
	
public:
	CAT45DB161Flash();
	~CAT45DB161Flash();	

	BOOL FlashCS(BOOL SetCS);	

	BOOL SetFlashToPowerOfTwo();	

	void OpenFlash(void);
	void CloseFlash(void);
	BOOL FlashPageCommand(BYTE Cmd, WORD PageAddress, WORD ByteAddress);
	BOOL FlashAddressCommand(BYTE Cmd, DWORD Address);
	BOOL ReadFlashPageIntoBufer1(DWORD Adress);
	BOOL WriteFlashBuffer1(DWORD Address, BYTE * Data, DWORD NumData);
	BOOL ProgrammBuffer1_WithErase(DWORD Address);
	BOOL ProgrammFlash(DWORD Address, BYTE * Data, DWORD NumData);
	BOOL ContiniousReadFlash(DWORD Address, BYTE * Data, DWORD NumData);
	BOOL SendFlashCommand(BYTE Command, DWORD Address);
	BOOL SendFlashCommand(DWORD Command);
	BOOL GetFlashChipId(BYTE * pData);
	BOOL Enable_SectorProtection(void);
	BOOL Disable_Sector_Protection(void);
	BOOL GetFlashStatus(BYTE & Data);
	BOOL WaitFlashReady();
	BYTE GetFlashSectorProtectionRegister();
	BYTE GetFlashSectorLockDownRegister();
	BYTE GetFlashSecurityRegister();
	BOOL EraseFlashSector(BYTE Sector);
};



#endif


