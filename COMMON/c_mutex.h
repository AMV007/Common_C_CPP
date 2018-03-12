/*
 * c_mutex class
 * Developed by Maxim Akristiniy <m_akristiniy@rambler.ru>
 * Maintained by Maxim Akristiniy library
 * See LICENSE file for copyright and license info
*/

#ifndef __H_C_MUTEX__
#define __H_C_MUTEX__

#ifndef _WIN32
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
// library pthread
#endif

// TODO : provide windows support

class c_mutex
{
private:
#ifndef _WIN32
	pthread_mutex_t 
#else
	HANDLE
#endif
	internal_mutex;

public:
	c_mutex()
	{
#ifndef _WIN32
		pthread_mutex_init(&internal_mutex,NULL);
#else
		internal_mutex=CreateMutex (NULL, FALSE, NULL);
#endif
	}

	~c_mutex()
	{
#ifndef _WIN32
		pthread_mutex_destroy(&internal_mutex);
#else
		CloseHandle(internal_mutex);
#endif
	}

#ifndef _WIN32
	pthread_mutex_t * GetPtr()
	{
		return &internal_mutex;
	}
#endif

	bool Trylock()
	{
        int res=0;
#ifndef _WIN32
		res=pthread_mutex_trylock(&internal_mutex);
		if(res==EDEADLK||res==EINVAL) printf("Error TryLock Mutex %d\n", res);
		
#else
        res=WaitForSingleObject(internal_mutex, 0);
        if(res==WAIT_ABANDONED)
        {
            printf("thread got ownership of an abandoned mutex\n");
        }     
#endif
        return (res==0);
	}

	bool Lock()
	{		
        int res = 0;
#ifndef _WIN32		
		res=pthread_mutex_lock(&internal_mutex);
        if(res==EDEADLK||res==EINVAL) printf("Error Lock Mutex %d\n", res);		
#else        
        res=WaitForSingleObject(internal_mutex, INFINITE);
        if(res==WAIT_ABANDONED)
        {
            printf("thread got ownership of an abandoned mutex\n");
        }          
#endif		
        return (res==0);
	}

	bool Unlock()
	{		
#ifndef _WIN32
		int res = false;
		res=pthread_mutex_unlock(&internal_mutex);
		if(res==EDEADLK||res==EINVAL) printf("Error Unlock Mutex %d\n", res);	
		return (res==0);
#else
        return ReleaseMutex(internal_mutex)==TRUE?true:false;
#endif		
	}
};

#endif //__H_C_MUTEX__ 