/*
 * c_thread class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

#include "stdafx.h"
#include "c_thread.h"
#include <time.h>
#include <string.h>

//------------------------------------------------------------------------------------------
// c_thread
c_thread::c_thread(void * Context, bool StartOnCreate, bool CreateSuspended, int StackSize)
:ThreadHandle(INVALID_HANDLE_VALUE),
 ThreadPriority(THREAD_PRIORITY_NORMAL),
 Stopped(true),
 Suspended(false),
 Finished(false),
 ThreadContext(Context)
{
    if(StartOnCreate)
	{
		Start(Context,CreateSuspended,StackSize);
	}
}

c_thread::~c_thread()
{
	ForceStop();
}

static
#ifdef _WIN32
    unsigned int __stdcall
#else
    void *
#endif
StaticThreadRun(void * Args)
{	
	c_thread * ThisThread = (c_thread *) Args;
#ifndef _WIN32
	if(ThisThread->Suspended)
	    ThisThread->suspend_event.WaitFor();    
#endif
    if(!ThisThread->Stopped)ThisThread->Execute(ThisThread->ThreadContext);
	ThisThread->Finished=true;
#ifndef _WIN32
	ThisThread->work_event.Set();
#endif
	return 0;
}



bool c_thread::Start(void * Context,bool CreateSuspended,int StackSize)
{
	ForceStop();

	Stopped=false;
    ThreadContext = Context;

    if(Context!=NULL) ThreadContext = Context;

#ifdef _WIN32
	int StartFlags=0;
#endif
	if(CreateSuspended)
	{
		Suspended=true;
#ifdef _WIN32
		StartFlags=CREATE_SUSPENDED;
#endif
	}else Finished=false;

#ifdef _WIN32
	if(StackSize) StartFlags|=STACK_SIZE_PARAM_IS_A_RESERVATION;
	ThreadHandle=CreateThread(NULL,StackSize,(LPTHREAD_START_ROUTINE)StaticThreadRun,this,StartFlags,NULL);
	if(ThreadHandle==NULL) return false;
#else

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if(StackSize) pthread_attr_setstacksize(&attr, StackSize);

	if (pthread_create(&ThreadHandle, &attr, StaticThreadRun,this) != 0)
	{
		ThreadHandle = INVALID_HANDLE_VALUE;
		return false;
	}
#endif
	if(ThreadPriority != THREAD_PRIORITY_NORMAL)
	{
		SetPriority(ThreadPriority);
	}
	return true;
}

void  c_thread::Resume(void)
{
	if(Suspended)

		Finished=false;
		Suspended=false;{
#ifdef _WIN32
		ResumeThread(ThreadHandle);
#else
		suspend_event.Set();
#endif
	}
}

void  c_thread::Stop(bool WaitForStop)
{
	Stopped=true;
	if(Suspended) Resume();
	if(WaitForStop) WaitFor();
}

bool  c_thread::SetPriority(int Priority)
{	
	ThreadPriority = Priority;
	if(ThreadHandle!=INVALID_HANDLE_VALUE) 
	{
#ifdef _WIN32
		return SetThreadPriority(ThreadHandle, Priority)==TRUE?true:false;
#else
		struct sched_param param;
		param.__sched_priority = Priority;
		pthread_setschedparam(ThreadHandle, SCHED_RR,&param);
#endif
	}

	return true;
}

bool c_thread::ForceStop()
{
	bool res=true;
	Stop();
	if(!WaitFor(0))
	{ // thread not already stopped
#ifdef _WIN32
		res=TerminateThread(ThreadHandle,0)==TRUE?true:false;
#else
		res=!pthread_cancel(ThreadHandle);
		pthread_join(ThreadHandle, NULL);
#endif
		ThreadHandle=INVALID_HANDLE_VALUE;
	}
	return res;
}

bool c_thread::WaitFor(unsigned int WaitTimeout_ms)
{
	if (ThreadHandle==INVALID_HANDLE_VALUE
#ifdef _WIN32
			||ThreadHandle==NULL
#endif
	)return true;

	if(Suspended)
	{
		Resume();
	}

#ifdef _WIN32
	if (WaitForSingleObject(ThreadHandle, WaitTimeout_ms)!=WAIT_OBJECT_0)
	{
		return false;
	}
	CloseHandle(ThreadHandle);
	ThreadHandle=INVALID_HANDLE_VALUE;
#else
	work_event.WaitFor(WaitTimeout_ms);
	int res=pthread_join(ThreadHandle, NULL);
	if (res!= 0)
	{
		return false;
	}
	ThreadHandle=INVALID_HANDLE_VALUE;

#endif
	return true;
}
