#include "stdafx.h"
#include "c_ListPtr.h"

BOOL __fastcall c_ListPtr::SetCapacity(const __int32 Capacity)
{
	int Delta=Capacity-m_Capacity;		
	if(Capacity==0)
	{ // grow
		if(m_Capacity>64)Delta=m_Capacity/4;
		else if(m_Capacity>8)Delta=16;
		else Delta=4;
	}

	if(Delta<0) return false;

	m_Capacity=m_Capacity+Delta;
	Data=(PVOID *)realloc(Data,m_Capacity*sizeof(PVOID));
	if(Data!=NULL) return true;
	return false;
}

BOOL __fastcall c_ListPtr::Add(const PVOID NewData)
{
	if(m_Count==m_Capacity)
		if(!SetCapacity()) return false;
	Data[m_Count++]=NewData;
	return true;
}

BOOL __fastcall c_ListPtr::AddPtr(const PVOID PtrData, __int32 PtrDataLen)
{		
	BYTE * NewPtrData = new BYTE[PtrDataLen];
	memcpy(NewPtrData,PtrData,PtrDataLen);
	BOOL res= Add(NewPtrData);		
	if(!res)delete NewPtrData;
	return res;
}

BOOL __fastcall c_ListPtr::AddStr(const char* StrData, __int32 StrDataLen)
{
	if(StrDataLen==0)StrDataLen=strlen(StrData)+1;
	return AddPtr((PVOID)StrData,StrDataLen);
}

BOOL __fastcall c_ListPtr::Insert(const __int32 Pos, const PVOID NewData)
{
	if(Pos>m_Count) return false;
	if(m_Count==m_Capacity)
		if(!SetCapacity()) return false;		

	memmove(Data[Pos+1],Data[Pos],(m_Count-Pos)*sizeof(PVOID));
	Data[Pos]=NewData;
	m_Count++;
	return true;
}

BOOL __fastcall c_ListPtr::InsertPtr(const __int32 Pos,const PVOID PtrData, __int32 PtrDataLen)
{		
	BYTE * NewPtrData = new BYTE[PtrDataLen];
	memcpy(NewPtrData,PtrData,PtrDataLen);
	BOOL res=Insert(Pos,NewPtrData);		
	if(!res) delete NewPtrData;
	return res;
}

BOOL __fastcall c_ListPtr::InsertStr(const __int32 Pos,const char* StrData, __int32 StrDataLen)
{
	if(StrDataLen==0)StrDataLen=strlen(StrData)+1;
	return InsertPtr(Pos,(PVOID)StrData,StrDataLen);
}

BOOL __fastcall c_ListPtr::Exchange(const __int32 Pos1, const __int32 Pos2)
{
	if(Pos1<0||Pos2<0||Pos1>=m_Count||Pos2>=m_Count) return FALSE;
	PVOID TempData = Data[Pos1];
	Data[Pos1]=Data[Pos2];
	Data[Pos2]=TempData;
	return true;
}

// just only remove, not delete data
BOOL __fastcall c_ListPtr::Remove(int Pos)
{
	if(Pos>m_Count) return false;
	memmove(Data[Pos],Data[Pos+1], (m_Count-Pos-1)*sizeof(PVOID));
	m_Count--;
	return true;
}

// remove and delete data pointer
BOOL __fastcall c_ListPtr::Delete(int Pos)
{
	if(Pos>m_Count) return false;
	delete Data[Pos];
	return Remove(Pos);		
}

void __fastcall c_ListPtr::Clear()
{
	for(int i=0;i<m_Count;i++)
	{
		delete Data[i];
	}
	free(Data);
    Data=NULL;
	m_Count=0;
	m_Capacity=0;
}

BOOL __fastcall c_ListPtr::SaveToFile(const TCHAR * Path, const BOOL CreateNew, const __int32 BufferSize) 
{
	BOOL res=true;
	c_file * SaveFile = new c_file(Path,true,true,CreateNew);
	if(!SaveFile->Opened()) 
	{
		res=false;
	}
	if(res) 
	{
		if(!CreateNew) 
		{
			SaveFile->SetPos(0, C_FILE_FILE_END);
		}

		PBYTE Buffer = new BYTE[BufferSize];
		int BytesInBuffer=0;
		if(Buffer==NULL) res=false;
		else
		{

			for(int i=0;res&&i<this->m_Count;i++) 
			{	
				int CurrStrLen=strlen((char *)Data[i])*sizeof(char);
				if((BufferSize-BytesInBuffer)>(CurrStrLen+3))
				{
					memcpy(&Buffer[BytesInBuffer],Data[i],CurrStrLen);
					BytesInBuffer+=CurrStrLen;

					// end of line symbol
					memcpy(&Buffer[BytesInBuffer],"\r\n",2);
					BytesInBuffer+=2;
				}
				else
				{
					res=SaveFile->Write(Buffer, BytesInBuffer);
                    BytesInBuffer=0;
				}
			}

			if(BytesInBuffer&&res)
			{ // finally data
				res=SaveFile->Write(Buffer, BytesInBuffer);		
			}

			delete Buffer;
		}
	}

	delete SaveFile;
	return res;
}