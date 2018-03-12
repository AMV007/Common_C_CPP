/*
	This is usual list, with pointers
*/
#ifndef _C_LIST_H_
#define _C_LIST_H_

#include <malloc.h>

template <typename NT>
class c_ListNode {
private:
    c_ListNode* pNext;
	c_ListNode* pPrior;
   	template <typename>	friend class c_List;

public:
	NT m_val;
	int m_PtrSize; // we need free resources
	c_ListNode(const NT& val, c_ListNode* Prior) : m_val(val), pNext(NULL), pPrior(Prior),m_PtrSize(0) {}
	~c_ListNode() { if(m_PtrSize)free((PVOID)m_val); delete pNext; }

};

template <typename LT>
class c_List {
private:
	c_ListNode<LT>* __fastcall GetFirst() const { return m_pFirst; }
	c_ListNode<LT>* __fastcall GetLast() const { return m_pLast; }
	c_ListNode<LT>* __fastcall GetNode(unsigned long num);
	c_ListNode<LT>* __fastcall GetNext(const c_ListNode<LT>* pCurrentNode) const;
	c_ListNode<LT>* __fastcall GetPrior(const c_ListNode<LT>* pCurrentNode) const;

	bool __fastcall Delete(c_ListNode<LT>* pNode);
	bool __fastcall Insert(c_ListNode<LT>* pNodeAfter, const LT& val);	
public:
	enum ListMethodResult {
		DeleteSublistSuccess = 40,
		DeleteSublistErrorArgs,
		DeleteSublistErrorMiss
	};
	c_List() : m_pFirst(NULL), m_pLast(NULL), m_count(0) {}
	c_List(const c_List& L);
	~c_List() { delete m_pFirst; }
	const unsigned long Count() const { return m_count; }
	
	c_List& operator =(const c_List& L);
	LT&  __fastcall operator [](const unsigned long num) const;
    //int  __fastcall operator Count();

	LT&  __fastcall Get (unsigned long num);
	bool __fastcall Add(const LT& val);
	bool __fastcall AddPtr(const LT& val, __int32 ValSize);
	bool __fastcall AddStr(const LT& val,int LenOfStr=0);
	bool __fastcall Insert(const unsigned long num, const LT& val);
	bool __fastcall InsertPtr(const unsigned long num, const LT& val, __int32 ValSize);
	bool __fastcall InsertStr(const unsigned long num, const LT& val,int LenOfStr=0);	
	bool __fastcall Exchange(const unsigned long num1,const unsigned long num2); // exchange list members locations
	bool __fastcall Delete(const unsigned long num);	
	unsigned long __fastcall DeleteSublist(const unsigned long beforeFirst, const unsigned long afterLast);
	void __fastcall Clear();	// delete all values in the list
	bool __fastcall SaveToFile(const char * Path, const bool CreateNew = TRUE);
private:
	c_ListNode<LT>* m_pFirst;
	c_ListNode<LT>* m_pLast;
	unsigned long m_count;
};

// методы
template <typename LT>
c_List<LT>::c_List(const c_List& L) : m_pFirst(NULL), m_pLast(NULL), m_count(0) {
	for (const c_ListNode<LT>* pNode = L.m_pFirst; pNode; pNode = pNode->pNext) {
		if (!Append(pNode->m_val)) {
			delete this->m_pFirst;
			this->m_pFirst = NULL;
			this->m_pLast = NULL;
			this->m_count = 0;
			break;
		}
	}
}

template <typename LT>
c_ListNode<LT>* __fastcall c_List<LT>::GetNode(unsigned long num){

    c_ListNode<LT>* pNode=this->m_pFirst;
    if (!pNode) return NULL;

    if(num<=(m_count/2)) // first half
        while (num--) pNode = pNode->pNext;
    else
    { // second half
        pNode = this->m_pLast;
	    if (!pNode) return NULL;

        num = (m_count-num);
        while (num--) pNode = pNode->pPrior;
    }

	return pNode;
}

template <typename LT>
c_ListNode<LT>* __fastcall c_List<LT>::GetNext(const c_ListNode<LT>* pCurrentNode) const {
	if (!pCurrentNode) {
		return NULL;
	}

	return pCurrentNode->pNext;
}

template <typename LT>
c_ListNode<LT>* __fastcall c_List<LT>::GetPrior(const c_ListNode<LT>* pCurrentNode) const {
	if (!pCurrentNode) {
		return NULL;
	}

	return pCurrentNode->pPrior;
}

template <typename LT>
LT&  __fastcall c_List<LT>::Get(unsigned long num)
{
	c_ListNode<LT>* pNode = GetNode(num);
	return pNode->m_val;
}

template <typename LT>
c_List<LT>& c_List<LT>::operator =(const c_List& L) {
	if (&L != this) {
		Clear();

		for (const c_ListNode<LT>* pNode = L.m_pFirst; pNode; pNode = pNode->pNext) {
			if (!Append(pNode->m_val)) {
				Clear();
				break;
			}
		}
	}

	return *this;
}

template <typename LT>
LT& __fastcall c_List<LT>::operator [](const unsigned long num)const{
	c_ListNode<LT>* pNode = GetNode(num);
	return pNode->m_val;
}



template <typename LT>
bool __fastcall c_List<LT>::Add(const LT& val) {
	c_ListNode<LT>* pNew;
	
	pNew = new c_ListNode<LT>(val, this->m_pLast);
	if(pNew==NULL) return false;	

	if (!this->m_pFirst) {
		this->m_pFirst = pNew;
	}
	if (this->m_pLast) {
		this->m_pLast->pNext = pNew;
	}
	this->m_pLast = pNew;
	++this->m_count;

	return true;
}

template <typename LT>
bool __fastcall c_List<LT>::AddPtr(const LT& val, __int32 ValSize) {
	if(Add((LT)malloc(ValSize)))
	{
		c_ListNode<LT>* LNode=GetLast();
		memcpy(LNode->m_val, val, ValSize);
		LNode->m_PtrSize=ValSize;
		return TRUE;
	}
	return FALSE;	
}

template <typename LT>
bool __fastcall c_List<LT>::AddStr(const LT& val, __int32 LenOfStr) {	
	if(LenOfStr==0)LenOfStr=strlen(val)+1;
	return AddPtr(val,LenOfStr);
}

template <typename LT>
bool __fastcall c_List<LT>::Insert(const unsigned long num, const LT& val) {
	if ((!this->m_pFirst) && (num == 0)) {
		Append(val);
	}
	else {	
		c_ListNode<LT>* pNext = GetNode(num);

		if (!pNext) {
			return false;
		}

		c_ListNode<LT>* pNew;

		try {
			pNew = new c_ListNode<LT>(val, pNext->pPrior);
		}
		catch (bad_alloc&) {
			return false;
		}

		pNew->pNext = pNext;
		if (pNext->pPrior) {
			pNew->pPrior->pNext = pNew;
		}
		else {
			this->m_pFirst = pNew;
		}
		pNext->pPrior = pNew;
		++this->m_count;
	}

	return true;
}

template <typename LT>
bool __fastcall c_List<LT>::InsertPtr(const unsigned long num, const LT& val, __int32 ValSize) {
	if(Insert(num,(LT)malloc(ValSize)))
	{
		c_ListNode<LT>* LNode=GetNode(num);
		memcpy(LNode->m_val, val, ValSize);
		LNode->PtrSize=ValSize;
		return TRUE;
	}
	return FALSE;	
}

template <typename LT>
bool __fastcall c_List<LT>::InsertStr(const unsigned long num, const LT& val,int LenOfStr) {	
	if(LenOfStr==0)LenOfStr=strlen(val)+1;
	return InsertPtr(num, val, LenOfStr);
}

template <typename LT>
bool __fastcall c_List<LT>::Insert(c_ListNode<LT>* pNodeAfter, const LT& val) {
	if (!this->m_pFirst) {
		Append(val);
	}
	else if (!pNodeAfter) {
		return false;
	}
	else {
		c_ListNode<LT>* pNew;

		try {
			pNew = new c_ListNode<LT>(val, pNodeAfter->pPrior);
		}
		catch (bad_alloc&) {
			return false;
		}

		pNew->pNext = pNodeAfter;
		if (pNew->pPrior) {
			pNew->pPrior->pNext = pNew;
		}
		else {
			this->m_pFirst = pNew;
		}
		pNodeAfter->pPrior = pNew;
		++this->m_count;
	}

	return true;
}

template <typename LT>
bool __fastcall c_List<LT>::Exchange(const unsigned long num1, const unsigned long num2) {

	c_ListNode<LT>* pNode1 = GetNode(num1);
	c_ListNode<LT>* pNode2 = GetNode(num2);

	if (!pNode1||!pNode2) {
		return false;
	}

	if(pNode1==m_pFirst)m_pFirst=pNode2;
    else if(pNode2==m_pFirst)m_pFirst=pNode1;

	if(pNode1==m_pLast)m_pLast=pNode2;
    else if(pNode2==m_pLast)m_pLast=pNode1;

	// change in neighbor members
	if(pNode1->pPrior)pNode1->pPrior->pNext=pNode2;
	if(pNode1->pNext)pNode1->pNext->pPrior=pNode2;

	if(pNode2->pPrior)pNode2->pPrior->pNext=pNode1;
	if(pNode2->pNext)pNode2->pNext->pPrior=pNode1;

	// change internal members
	c_ListNode<LT>* Next1=pNode1->pNext;
	c_ListNode<LT>* Prior1=pNode1->pPrior;
	
	pNode1->pNext=pNode2->pNext;
	pNode1->pPrior=pNode2->pPrior;

	pNode2->pNext=Next1;
	pNode2->pPrior=Prior1;

	return true;
}

template <typename LT>
bool __fastcall c_List<LT>::Delete(const unsigned long num) {
	c_ListNode<LT>* pNode = GetNode(num);

	if (!pNode) {
		return false;
	}

	if (this->m_pFirst == pNode) {
		this->m_pFirst = pNode->pNext;
		if (!this->m_pFirst) {
			this->m_pLast = NULL;
		}
		else {
			pNode->pNext->pPrior = NULL;
			pNode->pNext = NULL;
		}
		delete pNode;
		--this->m_count;

		return true;
	}

	pNode->pPrior->pNext = pNode->pNext;
	if (pNode->pNext) {
		pNode->pNext->pPrior = pNode->pPrior;
		pNode->pNext = NULL;
	}
	if (this->m_pLast == pNode) {
		this->m_pLast = pNode->pPrior;
	}
	delete pNode;
	--this->m_count;

	return true;
}

template <typename LT>
bool __fastcall c_List<LT>::Delete(c_ListNode<LT>* pNode) {
	if (!pNode) {
		return false;
	}

	if (this->m_pFirst == pNode) {
		this->m_pFirst = pNode->pNext;
		if (!this->m_pFirst) {
			this->m_pLast = NULL;
		}
		else {
			pNode->pNext->pPrior = NULL;
			pNode->pNext = NULL;
		}
		delete pNode;
		--this->m_count;

		return true;
	}

	pNode->pPrior->pNext = pNode->pNext;
	if (pNode->pNext) {
		pNode->pNext->pPrior = pNode->pPrior;
		pNode->pNext = NULL;
	}
	if (this->m_pLast == pNode) {
		this->m_pLast = pNode->pPrior;
	}
	delete pNode;
	--this->m_count;

	return true;
}

template <typename LT>
unsigned long __fastcall c_List<LT>::DeleteSublist(const unsigned long beforeFirst, const unsigned long afterLast) {
	if ((afterLast < beforeFirst) || ((afterLast - beforeFirst) < 2)) {
		return DeleteSublistErrorArgs;
	}	

	c_ListNode<LT>* bF = GetNode(beforeFirst);
	c_ListNode<LT>* aL = GetNode(afterLast);

	if ((!bF) || (!aL)) {
		return DeleteSublistErrorMiss;
	}

	c_ListNode<LT>* DeleteFirst = bF->pNext;

	bF->pNext = aL;
	aL->pPrior->pNext = NULL;
	aL->pPrior = bF;
	delete DeleteFirst;
	this->m_count -= (afterLast - beforeFirst - 1);

	return DeleteSublistSuccess;
}

template <typename LT>
void __fastcall c_List<LT>::Clear() {
	if(this->m_pFirst)
	{
		delete this->m_pFirst;
		this->m_pFirst=NULL;
		this->m_pLast=NULL;
		this->m_count=0;
	}
}

template <typename LT>
bool __fastcall c_List<LT>::SaveToFile(const char * Path, const bool CreateNew) {

    bool res=TRUE;
	c_file * SaveFile = new c_file(Path,true,true,CreateNew);
	if(!SaveFile->Opened()) {
		res=FALSE;
	}
    if(res) {
        if(!CreateNew) {
            SaveFile->SetPos(0, C_FILE_FILE_END);
        }

        for(unsigned long i=0;res&&i<this->m_count;i++) {
            c_ListNode<LT>* TempNode = GetNode(i);
            if(TempNode->m_PtrSize)
                res=SaveFile->WriteLine(TempNode->m_val);
            else
                res=SaveFile->WriteLine((char*)&TempNode->m_val, sizeof(LT));
        }
    }

    delete SaveFile;
	return res;
}

#endif //_C_LIST_H_
