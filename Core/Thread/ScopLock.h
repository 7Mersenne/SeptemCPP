/*
	Copyright (c) 2013-2019 7Mersenne All Rights Reserved.
	
	LICENSE:	GNU General Public License V3.0

	As a special exception,  you may use this file  as part of a free software library without
	restriction.  Specifically,  if other files instantiate templates  or use macros or inline
	functions from this file, or you compile this file and link it with other files to produce
	an executable,  this file does not by itself cause the resulting executable to be covered
	by the GNU General Public License. This exception does not however invalidate any other
	reasons why the executable file might be covered by the GNU General Public License.

	Support Email:	guij@sari.ac.cn
*/

#include <Core/Public/marco.h>

#ifdef LINUX
#include <pthread.h>
#define LOCKTYPE pthread_mutex_t
#else
#define LOCKTYPE uint64
#endif // LINUX


namespace Septem
{
	/*
	*	User Guide
	Sample Code
	```
	LOCKTYPE locker;
	locker = PTHREAD_MUTEX_INITIALIZER;
	{
		ScopeLock _scopelock(&locker);
	}
	```

	*/
	class ScopeLock
	{
	public:
		/*
		*	1. Constructor & wrap the pointer of lock
		*	2. Lock the lockObject
		*	@param InLockObject
		*/
		ScopeLock(LOCKTYPE* InLockObject)
			:lockObject(InLockObject)
		{
			check(lockObject);

#ifdef LINUX
			pthread_mutex_lock(lockObject);
#endif // LINUX

		}

		/** Destructor & unlock the lockObject*/
		~ScopeLock()
		{
			check(lockObject);

#ifdef LINUX
			pthread_mutex_unlock(lockObject);
#endif // LINUX
		}

	private:
		/** Default constructor (hidden on purpose). */
		ScopeLock();

		/** Copy constructor( hidden on purpose). */
		ScopeLock(const ScopeLock& InScopeLock);

		/** Assignment operator (hidden on purpose). */
		ScopeLock& operator=(ScopeLock& InScopeLock)
		{
			return *this;
		}

	private:
		LOCKTYPE* lockObject;
	};
}