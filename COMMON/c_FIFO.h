//-----------------------------------------------------------------------------
#ifndef __C_FIFO_BASE_H__
#define __C_FIFO_BASE_H__

#ifdef _WIN32
	#include <windows.h>
	#define Sleep_my_FIFO(milisecond) Sleep(milisecond)
#else
	#include <time.h>
	#include <semaphore.h>

	inline void Sleep_my_FIFO(int milisecond)
	{
		struct timespec RequestedTime;
		RequestedTime.tv_sec=milisecond/1000;
		RequestedTime.tv_nsec=(milisecond-(RequestedTime.tv_sec*1000))*1000000;
		nanosleep(&RequestedTime,NULL);
	}
#endif

// taking in mind, that int operations must be atomic !!

template <typename NT>
class c_FIFO
{
protected :
	NT *	m_Array;

	unsigned int m_wait_time_for_new_data;

	unsigned int		m_Size;
	unsigned int		WritePointer;
	unsigned int		ReadPointer;

	// for some thread safety
	bool 	PossibleUseCounters;
	bool	    PossibleRead;
	bool    	PossibleWrite;
	bool 	ThisTerminating;

	// programm must not change these fields, if them where changed,
	// must use functions GetSizeLength and GetSizeBytes instead
	unsigned int SizeBytes;		// size in bytes of FIFO

	//sem_t	ReadAccess;
	//sem_t	WriteAccess;

public:
	c_FIFO(unsigned int Length, unsigned int wait_time_for_new_data=1):
		PossibleUseCounters(true),PossibleRead(true),PossibleWrite(true),ThisTerminating(false)
	{
		m_wait_time_for_new_data=wait_time_for_new_data;
		m_Size=Length+1; // because therre will be stuck if WritePointer=ReadPointer
		SizeBytes = m_Size*sizeof(NT);

		m_Array = new NT[m_Size];

		Flush();
	}

	~c_FIFO(void)
	{
		TerminateWait();
		if (m_Array!=NULL) delete [] m_Array;
		m_Array=NULL;
	}

	void TerminateWait()
	{
		ThisTerminating=true;
	}

	void EnableWaitBack()
	{
		ThisTerminating=false;
	}

	// common purpoise functions
	virtual bool Write(NT Data[], unsigned int DataLength)
	{
		while(GetCounterFree()<DataLength)
		{ // blocking input
			if(ThisTerminating) return false;
			Sleep_my_FIFO(1);
		}

		while(!PossibleWrite)
		{
			if(ThisTerminating) return false;
			Sleep_my_FIFO(m_wait_time_for_new_data);
		}

		PossibleWrite = false;

		if ((WritePointer+DataLength)<m_Size)
		{
			memcpy(&m_Array[WritePointer],Data,DataLength);
			WritePointer+=DataLength;
		}
		else
		{
			unsigned Size1=m_Size-WritePointer;
			unsigned Size2=DataLength-Size1;

			memcpy(&m_Array[WritePointer],&Data[0],Size1);
			memcpy(&m_Array[0],&Data[Size1],Size2);
			WritePointer=Size2;
		}

		PossibleWrite = true;
		return true;
	}

	virtual bool Read(NT Data[], unsigned int DataLength)
	{
		while (GetCounter()<DataLength)
		{ // blocking output, may be later will be choise to non block
			if(ThisTerminating) return false;
			Sleep_my_FIFO(1);
		}

		while(!PossibleRead)
		{
			if(ThisTerminating) return false;
			Sleep_my_FIFO(m_wait_time_for_new_data);
		}

		PossibleRead = false;

		if ((ReadPointer+DataLength)<m_Size)
		{
			memcpy(&Data[0], &m_Array[ReadPointer],DataLength);
			ReadPointer+=DataLength;
		}
		else
		{
			unsigned int Size1=m_Size-ReadPointer;
			unsigned int Size2=DataLength-Size1;

			memcpy(&Data[0], &m_Array[ReadPointer],Size1);
			memcpy(&Data[Size1], &m_Array[0],Size2);
			ReadPointer=Size2;
		}

		PossibleRead = true;
		return true;
	}

	virtual unsigned int GetCounter(void)		// number of data elements in FIFO
	{
		unsigned int res=0;
		while(!PossibleUseCounters)
		{
			if(ThisTerminating) return 0;
			Sleep_my_FIFO(m_wait_time_for_new_data);
		}
		PossibleUseCounters=false;

		if(ReadPointer==WritePointer) res=0;
		else if(ReadPointer<WritePointer)
		{
			res= (WritePointer-ReadPointer);
		}
		else
		{ //ReadPointer>WritePointer
			res= ((m_Size-ReadPointer)+WritePointer);
		}
		PossibleUseCounters=true;
		return res;
	}

	virtual unsigned int GetCounterFree(void)   // number of free data space in FIFO
	{
		while(!PossibleUseCounters)
		{
			if(ThisTerminating) return 0;
			Sleep_my_FIFO(m_wait_time_for_new_data);
		}
		return m_Size-1-GetCounter();
	}

	// this funstions returning correct FIFO size, even if SizeBytes or where changed in programm
	virtual unsigned int GetSizeLength(void)	// FIFO length in elements
	{
		return m_Size-1;
	}

	virtual unsigned int GetSizeBytes(void)		// FIFO length in bytes
	{
		return (m_Size-1)*sizeof(NT);
	}

	virtual unsigned int GetCapacityBytes(void)		// FIFO length in bytes
	{
		return SizeBytes;
	}

	virtual void Flush()
	{
		while(!PossibleUseCounters)
		{
			if(ThisTerminating) return;
			Sleep_my_FIFO(m_wait_time_for_new_data);
		}

		PossibleUseCounters = false;

		while(!PossibleRead)
		{
			if(ThisTerminating) return;
			Sleep_my_FIFO(m_wait_time_for_new_data);
		}
		if(ThisTerminating) return;

		while(!PossibleWrite)
		{
			if(ThisTerminating) return;
			Sleep_my_FIFO(m_wait_time_for_new_data);
		}
		if(ThisTerminating) return;
		PossibleWrite = false;

		WritePointer=ReadPointer=0;

		PossibleRead=PossibleWrite=PossibleUseCounters = true;
	}
};

#endif //__C_FIFO_BASE_H__
