
#ifndef __CParallel_H__
#define __CParallel_H__
class CParallel
{
private:	
	DWORD PortAddress;
public:
	CParallel();
	~CParallel();

	bool OpenDevice(int PortNumber);

	void WriteDWORD(DWORD Data);
	DWORD ReadDWORD();

	void WriteByte(BYTE Data);
	BYTE ReadByte();
	void WriteData(BYTE * Data, DWORD NumData);
	void ReadData(BYTE * Data, DWORD NumData);

};

#endif