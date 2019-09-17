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

#pragma once

#include <Core/Public/marco.h>

#ifdef LINUX
#include <pthread.h>
#endif // LINUX

#include <Core/Thread/ScopLock.h>

namespace Septem
{

	/**
	 * Singleton Template
	 * first init when user first call T::Get()
	 * The private pointer cannot be deleted in main() of program
	 * ::Get() is Thread-Safe
	 * ::Singleton() is !Not-Thread-Safe!
	 * Inherit sample:
	 * class TClassname : public TSingleton<TClassname> {xxx};
	 * get instance : TClassname::Get();
	 */
	template<typename T>
	class TSingleton
	{
	public:
		// thread safe; singleton will init when first call get()
		static T* Get();
		// thread safe; singleton will init when first call getRef()
		static T& GetRef();

		// danger call, but fast
		static T* Singleton();
		// danger call, but fast
		static T& SingletonRef();

	protected:
		static T* pSingleton;
#ifdef LINUX
		static pthread_mutex_t m_SingletonLock;
#endif // LINUX

	private:
		TSingleton()
		{
			// && "Singleton can't create 2 object!"
			check(nullptr == pSingleton);
			pSingleton = static_cast<T*>(this);
		}
	public:
		virtual ~TSingleton()
		{
			pSingleton = nullptr;
		}
	};

	template<typename T>
	inline T * TSingleton<T>::Get()
	{
		if (nullptr == pSingleton) {
			ScopeLock lockSingleton(&m_SingletonLock);
			pSingleton = new T();
		}

		return pSingleton;
	}

	template<typename T>
	inline T & TSingleton<T>::GetRef()
	{
		if (nullptr == pSingleton) {
			ScopeLock lockSingleton(&m_SingletonLock);
			pSingleton = new T();
		}

		return *pSingleton;
	}

	template<typename T>
	inline T * TSingleton<T>::Singleton()
	{
		return pSingleton;
	}

	template<typename T>
	inline T & TSingleton<T>::SingletonRef()
	{
		check(pSingleton);
		return *pSingleton;
	}

	//-----------------------------------------------------------------------------
	// static values
	//-----------------------------------------------------------------------------

	template<typename T>
	T* TSingleton<T>::pSingleton = nullptr;

#ifdef LINUX
	template<typename T>
	pthread_mutex_t TSingleton<T>::m_SingletonLock = PTHREAD_MUTEX_INITIALIZER;
#endif

}