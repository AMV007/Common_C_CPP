/*
 * c_thread class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

#ifndef __H_C_THREAD__
#define __H_C_THREAD__

#ifdef _WIN32
	// need windows.h
	#include <windows.h>
	#define Sleep_my_thread(milisecond) Sleep(milisecond)
#else
	#include <time.h>
	#include <pthread.h>
	#include <semaphore.h>
#ifndef INVALID_HANDLE_VALUE
	#define INVALID_HANDLE_VALUE ((unsigned int)(-1))
#endif
	// just plug define :
	//#define INFINITE (unsigned int)	(-1)
	#define THREAD_PRIORITY_NORMAL 1
	#define CREATE_SUSPENDED 1

    #include "c_event.h"
#endif



class c_thread
{
private:
#ifdef _WIN32
	HANDLE ThreadHandle;
#else
	pthread_t ThreadHandle;
#endif
	unsigned int ThreadPriority;

    virtual void Execute(void * Context=NULL)=0; // internal use only
    void * ThreadContext; // internal use only

public :
#ifndef _WIN32
	c_event suspend_event;
	c_event work_event;
#endif
	bool Stopped;
	bool Suspended;
    bool Finished;    
	
	c_thread(void * Context=NULL, bool StartOnCreate=false, bool CreateSuspended=false, int StackSize=0);
	virtual ~c_thread(void);

	static void Sleep(int milisecond)
	{
#ifndef _WIN32
		struct timespec RequestedTime;
		RequestedTime.tv_sec=milisecond/1000;
		RequestedTime.tv_nsec=(milisecond-(RequestedTime.tv_sec*1000))*1000000;
		nanosleep(&RequestedTime,NULL);
#else
		::Sleep(milisecond);
#endif
	}	
    
	virtual bool Start(void * Context=NULL, bool CreateSuspended=false,int StackSize=0); // constructor analog, possibility to start - stop thread
	virtual void Resume();	 // if thread was created in suspended state
	
	virtual void Stop(bool WaitForStop=false); // if WaitFor - function will call WaitFor and wait for thread stop
	
	virtual bool WaitFor(unsigned int WaitTimeout_ms=INFINITE);// waiting for thread finish
	virtual bool ForceStop(void); // terminate thread anyway

	virtual bool SetPriority(int Priority);

    friend        
        #ifdef _WIN32
            unsigned int __stdcall
        #else
            void *
        #endif
            StaticThreadRun(void * Args);
};

#endif //__H_C_THREAD__
