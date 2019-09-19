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
#include <Core/Thread/ScopLock.h>
//shared_ptr
#include <memory>
//container
#include <stack>

namespace Septem
{
	/*
	* SharedPtr Recycle Pool
	* Thread safe
	*/
	template<typename T>
	class TSharedRecyclePool
	{
	public:
		TSharedRecyclePool(int32 InNum = DEFAULT_RECYCLE_POOL_SIZE)
		{
			// init locker
			m_pool_locker = PTHREAD_MUTEX_INITIALIZER;

			Reset(InNum);
		}

		virtual ~TSharedRecyclePool()
		{}

		/*
		* Reset the pool to PoolCount elements
		*if PoolCount < m_pool.size(), do nothing
		*/
		void Reset(int32 PoolCount = DEFAULT_RECYCLE_POOL_SIZE)
		{
			ScopeLock _scopelock(&m_pool_locker);
			int32 imax = PoolCount - (int32)m_pool.size();
			if (imax > 0)
			{
				// push new object into pool
				for (int32 i = 0; i < imax; ++i)
				{
					m_pool.push(std::make_shared<T>());
				}
			}
		}

		/*
		* Reset the pool to PoolCount elements
		*if PoolCount < m_pool.size(), pop noneed elements
		*/
		void Resize(int32 PoolCount = DEFAULT_RECYCLE_POOL_SIZE)
		{
			ScopeLock _scopelock(&m_pool_locker);
			int32 imax = PoolCount - m_pool.size();
			if (imax > 0)
			{
				// push new object into pool
				for (int32 i = 0; i < imax; ++i)
				{
					m_pool.push(std::make_shared<T>());
				}
			}
			else {
				///make imax > 0
				imax = -imax;
				// pop new object from pool
				for (int32 i = 0; i < imax; ++i)
				{
					m_pool.pop();
				}
			}
		}

		int32 Num()
		{
			return (int32)m_pool.size();
		}

		SIZE_T Size()
		{
			return m_pool.size();
		}

		std::shared_ptr<T> Alloc(bool bNew = true)
		{
			std::shared_ptr<T> ret;

			{
				ScopeLock _scopelock(&m_pool_locker);
				if (!m_pool.empty())
				{
					ret = m_pool.top();
					m_pool.pop();
					return ret;
				}
			}

			if (bNew)
				return std::make_shared<T>();
		}

		/*
		* User Guide
		* pool.Dealloc(std::move(InSharedPtr));
		*/
		void Dealloc(std::shared_ptr<T>&& InSharedPtr)
		{
			/// check ptr is valid
			if (InSharedPtr)
			{
				ScopeLock _scopelock(&m_pool_locker);
				m_pool.push(InSharedPtr);
			}
		}

		void Dealloc(std::shared_ptr<T>& InSharedPtr)
		{
			/// check ptr is valid
			if (InSharedPtr)
			{
				ScopeLock _scopelock(&m_pool_locker);
				m_pool.push(InSharedPtr);
			}
		}

	protected:
		std::stack< std::shared_ptr<T> > m_pool;

		//pool locker
		LOCKTYPE m_pool_locker;
	};
}