
#include "stdafx.h"
#include "Parallel.h"
#include "..\\..\\WINDOWS\\DLPORTIO.h"

CParallel::CParallel()
{
	PortAddress=0x378;
}

CParallel::~CParallel()
{
	
}

bool CParallel::OpenDevice(int PortNumber)
{
	switch(PortNumber)
	{
	case 1:PortAddress=0x378; break;
	case 2:PortAddress=0x3BC; break;
	case 3:PortAddress=0x278; break;
	default: return false;
	}
	return true;
}

void CParallel::WriteByte(BYTE Data)
{	
	DlPortWritePortUchar(PortAddress, Data);	
}

BYTE CParallel::ReadByte()
{
	return DlPortReadPortUchar(PortAddress);
}

void CParallel::WriteDWORD(DWORD Data)
{	
	DlPortWritePortUlong(PortAddress, Data);	
}

DWORD CParallel::ReadDWORD()
{
	return DlPortReadPortUlong(PortAddress);
}

void CParallel::WriteData(BYTE *Data, DWORD NumData)
{
	DlPortWritePortBufferUchar(PortAddress, Data, NumData);
}

void CParallel::ReadData(BYTE *Data, DWORD NumData)
{
	DlPortReadPortBufferUchar(PortAddress, Data, NumData);
}
//------------------------------------------------------------------------------

