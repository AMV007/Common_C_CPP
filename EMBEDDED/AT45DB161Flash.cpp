#include "stdafx.h"
#include "AT45DB161Flash.h"


CAT45DB161Flash::CAT45DB161Flash()
{
	
}

CAT45DB161Flash::~CAT45DB161Flash()
{

}


BOOL CAT45DB161Flash::FlashCS(BOOL SetCS)
{
	if(SetCS) PortFlags|=CS_PIN;		
	else PortFlags&=~CS_PIN;	
	
	BOOL res=WriteByteByteBlaster(0,PortFlags);	
	Sleep(10);
	return res;
}



BOOL CAT45DB161Flash::SetFlashToPowerOfTwo(void)
{
	BOOL res=SET_FLASH_CS;
	if(res)
	{
		res=SendFlashCommand(0xa6802a3d);
		CLR_FLASH_CS;
	}
    WaitFlashReady();   
	return res;
}

BOOL CAT45DB161Flash::Disable_Sector_Protection(void)
{
	BOOL res=SET_FLASH_CS;
	if (res)
	{		
		res=SendFlashCommand(0x9a7f2a3d);		
		CLR_FLASH_CS;
	}
    WaitFlashReady();   
	return res;
}

BOOL CAT45DB161Flash::Enable_SectorProtection(void)
{
	BOOL res=SET_FLASH_CS;
	if (res)
	{		
		res=SendFlashCommand(0xa97f2a3d);
		CLR_FLASH_CS;
	}
    WaitFlashReady();   
	return res;	
}


//------------------------------------------------------------
void CAT45DB161Flash::OpenFlash(void)
{    
	CLR_FLASH_CS;	
}

BOOL CAT45DB161Flash::SendFlashCommand(BYTE Command, DWORD Address)
{
	BYTE Cmd[4];
	Cmd[0]=Command;
	Cmd[1]=(BYTE)(Address>>16);
	Cmd[2]=(BYTE)(Address>>8);
	Cmd[3]=(BYTE)(Address);
	return WriteSPIData(Cmd, 4);
}

BOOL CAT45DB161Flash::SendFlashCommand(DWORD Command)
{	
	return WriteSPIData((BYTE *)&Command, sizeof(Command));
}


BOOL CAT45DB161Flash::ReadFlashPageIntoBufer1(DWORD Address)
{
	BOOL res=SET_FLASH_CS;
	if(res)
	{
		res=SendFlashCommand(FLASH_CMD_MAIN_MEM_TO_BUFFER1, Address);		
		CLR_FLASH_CS;			
		if(res)res=WaitFlashReady();					
	}
	return res;
}

BOOL CAT45DB161Flash::WriteFlashBuffer1(DWORD Address, BYTE * Data, DWORD NumData)
{
	BOOL res=SET_FLASH_CS;
	if(res)
	{
		if (res=SendFlashCommand(FLASH_CMD_BUFFER1_WRITE, Address))
		{
			res=WriteSPIData(Data,NumData);			
		}
		CLR_FLASH_CS;
	}
	return res;
}

BOOL CAT45DB161Flash::ProgrammBuffer1_WithErase(DWORD Address)
{
	BOOL res=SET_FLASH_CS;
	if(res)
	{
		res=SendFlashCommand(FLASH_CMD_BUFFER1_PROGRAMM_WITH_ERASE, Address);		
		CLR_FLASH_CS;
	}
	return res;
}



BOOL CAT45DB161Flash::ProgrammFlash(DWORD Address, BYTE * Data, DWORD NumData)
{
	BOOL res=true;
	if (Address&(FLASH_PAGE_MASK))
	{ //адрес не кратен размеру страницы
		DWORD BufferAddress = Address&(FLASH_PAGE_MASK);
		DWORD NumDataToWrite = min(FLASH_PAGE_SIZE-BufferAddress, NumData);
		DWORD PageAddress = Address&(~FLASH_PAGE_MASK);		

		if (res=ReadFlashPageIntoBufer1(PageAddress))
		{
			if (res=WriteFlashBuffer1(BufferAddress,Data,NumDataToWrite))
			{
				if (res=ProgrammBuffer1_WithErase(PageAddress))
				{
					res=WaitFlashReady();

					NumData-=NumDataToWrite;
					Data+=NumDataToWrite;
					Address+=NumDataToWrite;
				}
			}
		}
	}

	while (NumData>=FLASH_PAGE_SIZE&&res)
	{
		DWORD NumDataToWrite = min(FLASH_PAGE_SIZE, NumData);		
		if (res=WriteFlashBuffer1(0,Data,NumDataToWrite))
		{
			if (res=ProgrammBuffer1_WithErase(Address))
			{
				res=WaitFlashReady();

				NumData-=NumDataToWrite;
				Data+=NumDataToWrite;
				Address+=NumDataToWrite;
			}
		}
	}   

	if(NumData&&res)
	{
		if (res=ReadFlashPageIntoBufer1(Address))
		{
			if (res=WriteFlashBuffer1(0,Data,NumData))
			{
				if (res=ProgrammBuffer1_WithErase(Address))
				{
					res=WaitFlashReady();
				}
			}
		}
	}
	return res;
}


BOOL CAT45DB161Flash::ContiniousReadFlash(DWORD Address, BYTE * Data, DWORD NumData)
{
	BOOL res=false;

	if(res=SET_FLASH_CS)
	{
		if (res=SendFlashCommand(FLASH_CMD_CONTINIUOS_ARRAY_READ_LEGACY, Address))
		{
			if (res=SendFlashCommand(FLASH_CMD_CONTINIUOS_ARRAY_READ_LEGACY, Address))
			{
				// Don't Care Byte
				if (res=ReadSPIData(Data, NumData))
				{
					
				}
			}
		}
		CLR_FLASH_CS;
	}
	return res;
}

BOOL CAT45DB161Flash::GetFlashChipId(BYTE * pData)
{
	BOOL res=false;
	if (res=SET_FLASH_CS)
	{
		if (res=WriteSPIByte(FLASH_CMD_GET_CHIP_ID))
		{
			res=ReadSPIData(pData, 4);			
		}
		CLR_FLASH_CS;
	}
	return res;
}

BOOL CAT45DB161Flash::EraseFlashSector(BYTE Sector)
{
	BOOL res=false;
	if(res=SET_FLASH_CS)
	{
		DWORD Address=(DWORD)Sector<<12;
		res=SendFlashCommand(FLASH_CMD_SECTOR_ERASE, Address);		
		CLR_FLASH_CS;			
		if (res) res=WaitFlashReady();					
	}
	return res;
}

BOOL CAT45DB161Flash::GetFlashStatus(BYTE & Data)
{
	BOOL res=false;
	if(res=SET_FLASH_CS)
	{
		if (res=WriteSPIByte(FLASH_CMD_STATUS_REGISTER_READ))
		{
			if(res=ReadSPIByte(Data))
			{
				
			}
		}
		CLR_FLASH_CS;
	}
	return true;
}

BOOL CAT45DB161Flash::WaitFlashReady()
{
	BYTE Data=0;
	DWORD tend=GetTickCount()+5000;
	while (!(Data&(1<<7)))
	{
		if (!GetFlashStatus(Data)) return false;
		if(GetTickCount()>tend) return false;
		Sleep(0);		
	}
	return true;
}

BYTE CAT45DB161Flash::GetFlashSectorProtectionRegister()
{	
	BYTE Data=0;
	BOOL res=SET_FLASH_CS;
	if(res)
	{
		if(res=WriteSPIByte(FLASH_CMD_SECTOR_PROTECTION_READ))
		{
			if (res=WriteSPIByte(0))
			{			
				res=ReadSPIByte(Data);			
			}
		}
		CLR_FLASH_CS;
	}
	return Data;
}

BYTE CAT45DB161Flash::GetFlashSectorLockDownRegister()
{	
	BYTE Data=0;
	BOOL res=SET_FLASH_CS;
	if (res)
	{
		if(res=WriteSPIByte(FLASH_CMD_SECTOR_LOCKDOWN_READ))
		{
			if(res=WriteSPIByte(0))
			{
				res=ReadSPIByte(Data);
			}
		}
		CLR_FLASH_CS;;
	}
	return Data;
}

BYTE CAT45DB161Flash::GetFlashSecurityRegister()
{	
	BYTE Data=0;
	BOOL res=SET_FLASH_CS;
	if(res)
	{
		if(res=WriteSPIByte(FLASH_CMD_SECURITY_READ))
		{
			if(res=WriteSPIByte(0))
			{	
				res=ReadSPIByte(Data);
			}
		}
		CLR_FLASH_CS;
	}
	return Data;
}

