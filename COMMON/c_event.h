/*
 * c_event class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

#ifndef __H_C_EVENT__
#define __H_C_EVENT__

#ifndef _WIN32
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
// library rt
#include "c_mutex.h"
#else
#include <windows.h>
#endif


// TODO : provide windows support
#ifndef INFINITE
#define INFINITE (unsigned int)	(-1)
#endif

class c_event
{
private:
#ifndef _WIN32
	bool condition;
	c_mutex internal_mutex;
	pthread_cond_t  internal_cond;
#else
    HANDLE hEvent;
#endif
public:
	c_event(bool InitialState=false, bool Manual=false)
#ifndef _WIN32
	:condition(false)
#endif
	{
#ifndef _WIN32
		pthread_cond_init(&internal_cond,NULL);
        if(InitialState) Set();
#else
        hEvent = CreateEvent( 
            NULL,               // default security attributes
            Manual,             // manual-reset event
            InitialState,       // initial state is nonsignaled
            TEXT("WriteEvent")  // object name
        ); 
#endif
	}

	~c_event()
	{
		Set();
#ifndef _WIN32
		pthread_cond_destroy(&internal_cond);
#else 
        if(hEvent!=NULL)
        {
            CloseHandle(hEvent);
            hEvent=NULL;
        }
#endif
	}

	bool Set()
	{
		bool res=false;
#ifndef _WIN32
		if(internal_mutex.Lock())
		{
			condition = true;
			pthread_cond_signal(&internal_cond);
			internal_mutex.Unlock();
			res=true;
		}
        return res;
#else
        return SetEvent(hEvent)==TRUE?true:false;
#endif
		
	}

	bool Reset()
	{
#ifndef _WIN32
		if(internal_mutex.Lock())
		{
			condition = false;
			internal_mutex.Unlock();
		}
        return true;
#else
        return ResetEvent(hEvent)==TRUE?true:false;
#endif
	}

	bool WaitFor(unsigned int WaitTimeout_ms=INFINITE)
	{
		bool res=false;
#ifndef _WIN32
		if(WaitTimeout_ms==INFINITE)
		{
			if(internal_mutex.Lock())
			{
				while(!condition)
					pthread_cond_wait(&internal_cond, internal_mutex.GetPtr());
				condition = false;
				internal_mutex.Unlock();
				res=true;
			}
		}
		else
		{
			if(internal_mutex.Lock())
			{
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				ts.tv_nsec+=(WaitTimeout_ms%1000)*1000;
				ts.tv_sec+=WaitTimeout_ms/1000;

				while(!condition)
				{
					if(pthread_cond_timedwait(&internal_cond, internal_mutex.GetPtr(),&ts)==0) continue;
					else break;
				}
				if(condition)res=true;
				internal_mutex.Unlock();
			}
		}
        return res;
#else
        return WaitForSingleObject(hEvent, WaitTimeout_ms) == WAIT_OBJECT_0;
#endif		
	}
};

#endif //__H_C_EVENT__ 