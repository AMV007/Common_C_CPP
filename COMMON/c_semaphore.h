
#pragma once
#ifndef __H_C_SEMAPHORE__
#define __H_C_SEMAPHORE__

#include <stdint.h>

#ifndef _WIN32
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
// library rt
#else
#include <windows.h>
#endif

// TODO : provide windows support

class c_semaphore
{
private:
	uint32_t MaxSemCount;
#ifndef _WIN32
	sem_t		
#else
	HANDLE
#endif
		internal_semaphore;
public:
	c_semaphore(uint32_t MaxSemCount=INT32_MAX)
	{

		this->MaxSemCount = MaxSemCount;
#ifndef _WIN32
		sem_init(&internal_semaphore,0,1);
#else
		internal_semaphore = CreateSemaphore( 
			NULL,           // default security attributes
			this->MaxSemCount,  // initial count
			this->MaxSemCount,  // maximum count
			NULL);          // unnamed semaphore
#endif
	}

	~c_semaphore()
	{
#ifndef _WIN32
		sem_destroy(&internal_semaphore);
#else
		CloseHandle(internal_semaphore);
#endif
	}

#ifndef _WIN32
	sem_t * GetPtr()
	{
		return &internal_semaphore;
	}
#else
	HANDLE GetPtr()
	{
		return internal_semaphore;
	}
#endif

	bool Trylock()
	{
#ifndef _WIN32
		int res=sem_trywait(&internal_semaphore);
		if(res==EINTR||res==EINVAL) printf("Error TryLock semaphore %d", res);
		return (res==0);
#endif
	}

	bool Lock(int WaitTimeout_ms)
	{		
#ifndef _WIN32
		int res=0;
		struct timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_nsec+=(WaitTimeout_ms%1000)*1000;
		ts.tv_sec+=WaitTimeout_ms/1000;

		while ((res=sem_timedwait(&internal_semaphore, &ts)) == -1 && errno == EINTR)
			continue;       /* Restart if interrupted by handler */
		return (res==0);
#else
		uint32_t dwWaitResult = WaitForSingleObject( 
            internal_semaphore,   // handle to semaphore
            WaitTimeout_ms);           // zero-second time-out interval
		switch (dwWaitResult) 
        { 
            // The semaphore object was signaled.
            case WAIT_OBJECT_0: 
				return true;
            // The semaphore was nonsignaled, so a time-out occurred.
            case WAIT_TIMEOUT: 
			default:
                return false;
        }
#endif		
	}

	bool Lock()
	{
#ifndef _WIN32
		int res=sem_wait(&internal_semaphore);
		if(res==EINTR||res==EINVAL) printf("Error Lock semaphore %d", res);
		return (res==0);
#else
		return Lock(0);
#endif
	}

	bool Unlock()
	{
#ifndef _WIN32
		int res=sem_post(&internal_semaphore);
		if(res==EINTR||res==EINVAL) printf("Error UnLock semaphore %d", res);
		return (res==0);
#else
		if (!ReleaseSemaphore( 
			internal_semaphore,  // handle to semaphore
			1,            // increase count by one
			NULL) )       // not interested in previous count
		{
			return false;
		}
		return true;
#endif
	}
};

#endif //__H_C_SEMAPHORE__
