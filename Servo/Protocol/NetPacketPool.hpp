// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include <Core/Public/marco.h>
#include <mutex>
#include <memory>
#include <deque>
#include <queue>

#define MAX_NETPACKET_IN_POOL 1024

/**
 * SPPMode is used select the algorithm of the Servo Packet Pool
 * This is only used by templates at compile time to generate one code path or another.
 */
enum class SPPMode
{
	Stack = 0,
	Queue = 1,
	Heap = 2,
	Fast = Queue
};

namespace Septem {
	/**
	* net packet pool base class
	* for set any pool algorithm
	* not thread safe, need use mutex outside
	*/
	template<typename T>
	class TNetPacketPool
	{
	public:
		TNetPacketPool() = default;
		virtual ~TNetPacketPool()
		{
		}

		/**
		 * Push an item to the pool.
		 * @Thread-safe for pool, but not for sharedptr
		 * @param InSharedPtr The item to add.
		 * @return true if the item was added, false otherwise.
		 * @note To be called only from producer thread(s).
		 * @see Pop
		 */
		virtual bool Push(const std::shared_ptr<T>& InSharedPtr) = 0;

		/**
		 * Removes and returns the item from the tail of the pool.
		 * @Thread-safe for pool, but not for sharedptr
		 * @param OutSharedPtr Will hold the returned value.
		 * @return true if a value was returned, false if the pool was empty.
		 * @note To be called only from consumer thread.
		 * @see Push
		 */
		virtual bool Pop(const std::shared_ptr<T>& OutSharedPtr) = 0;

		// not Thread-safe
		virtual bool IsEmpty() = 0;
	};

	template<typename T>
	class TNetPacketStack
		: public TNetPacketPool<T>
	{
	protected:
		std::deque< std::shared_ptr<T> > Pool;
		std::mutex PoolLock;
	public:
		TNetPacketStack()
		{
			std::lock_guard<std::mutex> scopelock(PoolLock);
			Pool.resize(MAX_NETPACKET_IN_POOL);
		}

		virtual ~TNetPacketStack()
		{
			std::lock_guard<std::mutex> scopelock(PoolLock);
			while (!Pool.empty()) Pool.pop_back();
		}

		// Thread-safe
		virtual bool Push(const std::shared_ptr<T>& InSharedPtr) override
		{
			std::lock_guard<std::mutex> scopelock(PoolLock);
			Pool.emplace_back(InSharedPtr);
			return true;
		}
		
		virtual bool Pop(const std::shared_ptr<T>& OutSharedPtr) override
		{
			std::lock_guard<std::mutex> scopelock(PoolLock);
			if (IsEmpty())
				return false;
			OutSharedPtr = Pool.back();
			Pool.pop_back();
			return true;
		}
		
		virtual bool IsEmpty() override
		{
			return Pool.size() == 0ULL;
		}
	};

	/**
	* net packet pool with Queue strategy
	* Multiple-producers single-consumer (MPSC)  for multi-thread
	 */
	template<typename T>
	class TNetPacketQueue
		: public TNetPacketPool<T>
	{
	protected:
		std::queue< std::shared_ptr<T> > Pool;
		std::mutex PoolLock;
	public:
		TNetPacketQueue()
			: TNetPacketPool()
		{
		}

		virtual ~TNetPacketQueue()
		{
			std::lock_guard<std::mutex> scopelock(PoolLock);
			while (!Pool.empty()) Pool.pop();
		}

		virtual bool Push(const std::shared_ptr<T>& InSharedPtr) override
		{
			std::lock_guard<std::mutex> scopelock(PoolLock);
			Pool.push(InSharedPtr);
			return true;
		}

		virtual bool Pop(const std::shared_ptr<T>& OutSharedPtr) override
		{
			std::lock_guard<std::mutex> scopelock(PoolLock);
			Pool.pop();
			return true;
		}

		virtual bool IsEmpty() override
		{
			return Pool.empty();
		}
	};
#if 0
	/**
	* net packet pool with Heap strategy
	* Attention about maxnum > heap.num
	* add a private lock for Multiple-producers
	*/
	template<typename T>
	class TNetPacketHeap
		: public TNetPacketPool<T>
	{
	protected:
		TArray<TSharedPtr<T, TMode> > heapPool;
		FCriticalSection HeapLock;

	public:
		TNetPacketHeap()
			: TNetPacketPool()
		{
			FScopeLock lockPool(&HeapLock);
			heapPool.Reset(MAX_NETPACKET_IN_POOL);
		}

		virtual ~TNetPacketHeap()
		{
			FScopeLock lockPool(&HeapLock);
			heapPool.Empty(heapPool.Max());
		}

		bool Push(const TSharedPtr<T, TMode>& InSharedPtr) override
		{
			FScopeLock lockPool(&HeapLock);
			return heapPool.HeapPush(InSharedPtr,
				[](const TSharedPtr<T, TMode>& A, const TSharedPtr<T, TMode>& B)
			{
				if (!A.IsValid()) return true;
				if (!B.IsValid()) return false;
				return  *(A.Get()) < *(B.Get());
			}
			) >= 0;
		}

		bool Pop(TSharedPtr<T, TMode>& OutSharedPtr) override
		{
			FScopeLock lockPool(&HeapLock);
			if (IsEmpty())
				return false;
			heapPool.HeapPop(OutSharedPtr,
				[](const TSharedPtr<T, TMode>& A, const TSharedPtr<T, TMode>& B)
			{
				if (!A.IsValid()) return true;
				if (!B.IsValid()) return false;
				return  *(A.Get()) < *(B.Get());
			},
				false
				);
			return true;
		}

		virtual bool IsEmpty() override
		{
			return heapPool.Num() == 0;
		}

	
	};

#endif
}







