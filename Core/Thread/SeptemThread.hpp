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
#include <Core/Templates/SeptemRecyclePool.hpp>

#ifdef LINUX
#include <pthread.h>
#endif // LINUX

#include <queue>
#include <functional>/// delegate

namespace Septem
{
	/*
	* Template Task Thread
	* Thread safe code style
	* Program Guide
	```
	class XThread : public TTaskThread<TaskType>
	{
	//...
	};
	```
	*/
	template<typename TaskType >
	class TTaskThread : public TSharedRecyclePool<TaskType>
	{
	public:
		TTaskThread()
			:TSharedRecyclePool<TaskType>()
		{
			m_Thread = 0;
			bRunning = false;
			i_ThreadState = 0;
			m_QueueLocker = PTHREAD_MUTEX_INITIALIZER;
		}

		virtual ~TTaskThread()
		{}

		//=============== thread begin	=================
	public:
		void CreateThread();
		void StopThread();
		void JoinThread();
		/// block call & delay stop thread
		void DelayStopThread();
		///pthread_create(&m_Thread, NULL, ThreadRun, (void*)this)
		static void* ThreadRun(void* arg);
	protected:
		pthread_t m_Thread;
		virtual void Init();
		virtual void Run();
		virtual void Destory();
		// if you want to push task before init(), you need set bRunning = true first.
		bool bRunning;
		/*
		*	0: nothing
		*	1: init
		*	2: running
		*	3: stoping
		*	4: destorying
		*/
		int32 i_ThreadState;
		//===============  thread end		=================


		//============ task  queue begin =================
	public:
		// thread safe
		void PushTask(std::shared_ptr<TaskType>& InTask);
		// thread safe
		void PushTask(std::shared_ptr<TaskType>&& InTask);
		// thread safe
		bool PopTask(std::shared_ptr<TaskType>& OutTask);
		// thread safe
		void ClearTaskQueue();
	protected:
		// task queue
		std::queue< std::shared_ptr<TaskType> > taskQueue;
		// queue locker
		LOCKTYPE m_QueueLocker;
		//============ task  queue end	 =================

		//============ task delegate begin =================
	public:
		// delegate example: bool DelegateFunc(std::shared_ptr<TaskType>& InTaskPtr);
		std::function< void(std::shared_ptr<TaskType>&) > TaskDelegate;
		virtual void OnDoTask(std::shared_ptr<TaskType>& InTaskPtr);
		//============ task delegate end	   =================
	};

	template<typename TaskType>
	inline void TTaskThread<TaskType>::CreateThread()
	{
		if (pthread_create(&m_Thread, NULL, ThreadRun, (void*)this))
		{
			printf("Create thread failed!\n");
			return;
		}
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::StopThread()
	{
		bRunning = false;
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::JoinThread()
	{
		if (m_Thread > 0)
		{
			pthread_join(m_Thread, NULL);
		}
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::DelayStopThread()
	{
		bRunning = false;
		JoinThread();
	}

	template<typename TaskType>
	inline void * TTaskThread<TaskType>::ThreadRun(void * arg)
	{
		TTaskThread<TaskType>* thread = (TTaskThread<TaskType>*) arg; //dynamic_cast<TTaskThread<TaskType>*>(arg);
		check(thread);
		thread->i_ThreadState = 1;
		thread->Init();
		thread->bRunning = true;
		thread->i_ThreadState = 2;
		thread->Run();
		thread->i_ThreadState = 3;
		thread->Destory();
		return nullptr;
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::Init()
	{
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::Run()
	{
		std::shared_ptr < TaskType > pCacheTask;
		while (bRunning)
		{
			// poptask is thread safe here!
			if (PopTask(pCacheTask) && pCacheTask)
			{
				OnDoTask(pCacheTask);
			}
		}
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::Destory()
	{
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::PushTask(std::shared_ptr<TaskType>& InTask)
	{
		if (bRunning)
		{
			ScopeLock _scopelock(&m_QueueLocker);
			taskQueue.push(InTask);
		}
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::PushTask(std::shared_ptr<TaskType>&& InTask)
	{
		if (bRunning)
		{
			ScopeLock _scopelock(&m_QueueLocker);
			taskQueue.push(InTask);
		}
	}

	template<typename TaskType>
	inline bool TTaskThread<TaskType>::PopTask(std::shared_ptr<TaskType>& OutTask)
	{
		ScopeLock _scopelock(&m_QueueLocker);
		if (taskQueue.empty())
		{
			return false;
		}

		OutTask = taskQueue.front();
		taskQueue.pop();
		return true;
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::ClearTaskQueue()
	{
		ScopeLock _scopelock(&m_QueueLocker);
		while (taskQueue.size() > 0) taskQueue.pop();
	}

	template<typename TaskType>
	inline void TTaskThread<TaskType>::OnDoTask(std::shared_ptr<TaskType>& InTaskPtr)
	{
		TaskDelegate(InTaskPtr);
	}

}