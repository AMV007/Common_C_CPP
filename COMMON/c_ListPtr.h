/*
	This called list, but realy this is array of pointers,
	dynamically wided.
*/
#ifndef _C_LIST_PTR_H_
#define _C_LIST_PTR_H_

#ifdef WIN32
#include "c_file.h"
#else
error !!! only for windows now ??
#endif //#ifdef WIN32
#include <malloc.h>

class c_ListPtr {
private:
	int m_Count;
	int m_Capacity;
	PVOID *Data;
public:
	c_ListPtr(int Capacity=0): Data(NULL),m_Count(0),m_Capacity(0){if(Capacity)SetCapacity(Capacity);};
	~c_ListPtr(){Clear();};

    int __fastcall Count()const{return m_Count;};

	BOOL __fastcall SetCapacity(const int Capacity=0);
	BOOL __fastcall Add(const PVOID NewData);
	BOOL __fastcall AddPtr(const PVOID PtrData, int PtrDataLen);
	BOOL __fastcall AddStr(const char* StrData, int StrDataLen=0);
	BOOL __fastcall Insert(const int Pos, const PVOID NewData);
	BOOL __fastcall InsertPtr(const int Pos,const PVOID PtrData, int PtrDataLen);
	BOOL __fastcall InsertStr(const int Pos,const char* StrData, int StrDataLen=0);
	BOOL __fastcall Exchange(const int Pos1, const int Pos2);
	// just only remove, not delete data
	BOOL __fastcall Remove(int Pos);
	// remove and delete data pointer
	BOOL __fastcall Delete(int Pos);
	void __fastcall Clear();
	BOOL __fastcall SaveToFile(const TCHAR * Path, const BOOL CreateNew,const int BufferSize=(1024*1024*10));
};

#endif //_C_LIST_PTR_H_