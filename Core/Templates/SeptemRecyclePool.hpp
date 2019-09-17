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

#include <Core/Containers/SeptemArray.h>

namespace Septem
{
	/*
	*	SharedPtr Recycle Pool
	*	default mod = no thread-safe
	*	please use ESPMode::ThreadSafe for thread-safe
	*/
	template<typename T, ESPMode InMode = ESPMode::Fast>
	//class SEPTEMCORE_API TSharedRecyclePool :£º SEPTEMCORE_API cannot use in template
	class TSharedRecyclePool
	{
	public:
		TSharedRecyclePool(int32 InNum = 1024)
		{
			if (InMode == ESPMode::ThreadSafe)
			{
				poolCriticalSection.Lock();
			}

			ptrPool.Reset(InNum);
			
			for (int32 i = 0; i < InNum; ++i)
			{
				TSharedPtr<T, InMode> ptr(new T());
				ptrPool.Add(ptr);
			}

			if (InMode == ESPMode::ThreadSafe)
			{
				poolCriticalSection.Unlock();
			}
		}

		~TSharedRecyclePool()
		{}

		// Thread safe when InMode = ESPMode::ThreadSafe
		TSharedPtr<T, InMode> Alloc()
		{
			if (InMode == ESPMode::ThreadSafe)
			{
				poolCriticalSection.Lock();
				if (ptrPool.Num() > 0)
				{				
					//TSharedPtr<T, InMode> popPtr(ptrPool.Pop(false));
					TSharedPtr<T, InMode>&& popPtr = ptrPool.Pop(false);
					poolCriticalSection.Unlock();

					return popPtr;
				}
				poolCriticalSection.Unlock();

				return TSharedPtr<T, InMode>(new T());
			}
			return PopOrCreate();
		}

		/* 
		*	Thread safe when InMode = ESPMode::ThreadSafe
		*	ensure the InSharedPtr is valid
		*/
		void Dealloc(const TSharedPtr<T, InMode>& InSharedPtr)
		{
			if (InSharedPtr.IsValid())
			{
				if (InMode == ESPMode::ThreadSafe)
				{
					FScopeLock lockPool(&poolCriticalSection);
					if (ptrPool.GetSlack() > 0)
					{
						ptrPool.Push(InSharedPtr);
					}
				}
				else {
					if (ptrPool.GetSlack() > 0)
					{
						ptrPool.Push(InSharedPtr);
					}
				}
			}
		}

		/*
		*	Thread safe when InMode = ESPMode::ThreadSafe
		*	ensure the InSharedPtr is valid
		*/
		void DeallocForceRecycle(const TSharedPtr<T, InMode>& InSharedPtr)
		{
			if (InSharedPtr.IsValid()) {
				if (InMode == ESPMode::ThreadSafe)
				{
					FScopeLock lockPool(&poolCriticalSection);
					ptrPool.Push(InSharedPtr);
				}
				else {
					ptrPool.Push(InSharedPtr);
				}
			}
		}

		// not thread safe
		int32 Num()
		{
			return ptrPool.Num();
		}

	private:

		// not thread safe
		FORCEINLINE TSharedPtr<T, InMode> PopOrCreate()
		{
			if (ptrPool.Num() > 0)
			{
				return ptrPool.Pop(false);
			}

			return TSharedPtr<T, InMode>(new T());
		}

		// stack of shared ptr object 
		// ensure every sharedptr in pool is valid
		TArray< TSharedPtr<T, InMode> >  ptrPool;
		FCriticalSection poolCriticalSection;
	};
}