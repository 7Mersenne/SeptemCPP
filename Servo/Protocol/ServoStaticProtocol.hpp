// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include <Core/Public/marco.h>
#include "ServoProtocol.h"
#include "NetBufferWrapper.hpp"


namespace Septem
{
	/**
*	Template Servo Net Packet
*	Head + TSNetBufferWrapper<T> + Foot
*/
	template<typename T>
	struct TSNetPacket
	{
		FSNetBufferHead Head;
		TSNetBufferWrapper<T> Body;
		FSNetBufferFoot Foot;

		// session id
		int32 sid;
		bool bFastIntegrity;

		bool IsValid();

		// check data integrity with fastcode
		static bool FastIntegrity(uint8* DataPtr, int32 DataLength, uint8 fastcode)
		{
			uint8 xor = 0;
			for (int32 i = 0; i < DataLength; ++i)
			{
				xor ^= DataPtr[i];
			}
			return xor == fastcode;
		}

		bool CheckIntegrity();

		TSNetPacket()
			:sid(0)
			, bFastIntegrity(false)
		{}

		/*
		* param BytesRead return the count of bytes been read. if BytesRead > BufferSize means failed
		*/
		TSNetPacket(uint8* Data, int32 BufferSize, int32& BytesRead, int32 InSyncword = DEFAULT_SYNCWORD_INT32)
		{
			Head.syncword = InSyncword;
			// 1. find syncword for head
			int32 index = BufferBufferSyncword(Data, BufferSize, Head.syncword);
			uint8 fastcode = 0;

			if (-1 == index)
			{
				// failed to find syncword in the whole buffer
				BytesRead = BufferSize + 1;
				return;
			}

			// 2. read head
			if (!Head.MemRead(Data + index, BufferSize - index))
			{
				// failed to read from the rest buffer
				BytesRead = BufferSize + 1;
				return;
			}

			index += FSNetBufferHead::MemSize();
			fastcode ^= Head.XOR();

			if (0 == Head.uid)
			{
				sid = Head.SessionID();
			}

			// 3. check and read body

			if (0 != Head.uid)
			{
				// it is not a heart beat packet
				//if (!Body.MemRead(Data + index, BufferSize - index, Head.size))
				if (!Body.Deserialize(Data + index, BufferSize - index))
				{
					// failed to read from the rest buffer
					BytesRead = BufferSize + 1;
					return;
				}
				index += Body.MemSize();
				fastcode ^= Body.XOR();
			}

			// 4. read foot
			if (!Foot.MemRead(Data + index, BufferSize - index))
			{
				// failed to read from the rest buffer
				BytesRead = BufferSize + 1;
				return;
			}

			index += FSNetBufferFoot::MemSize();
			fastcode ^= Foot.XOR();

			BytesRead = index;
			bFastIntegrity = 0 == fastcode;

			sid = Head.SessionID();

			return;
		}

		uint64 GetTimestamp();
		// static class template not need heartbeat
		//static TSNetPacket* CreateHeartbeat(int32 InSyncword = DEFAULT_SYNCWORD_INT32);
		void ReUse(uint8* Data, int32 BufferSize, int32& BytesRead, int32 InSyncword = DEFAULT_SYNCWORD_INT32);
		void ReUse(FSNetBufferHead & InHead, uint8 * Buffer, int32 BufferSize, int32 & BytesRead);
		void WriteToArray(std::vector<uint8>& InBufferArr);
		void OnDealloc();
		void OnAlloc();
		bool operator < (TSNetPacket<T> && Other);
		bool operator < (const TSNetPacket<T> & Other);
	};

	/**
	 * T::		the class  handle in TSNetPacket<T>
	 * //TODO: delete xx PtrMode::	the thread-safe mode of TSharedPtr
	 * //TODO: delete PoolMode:: the mdoe of pools
	 */
	template<typename T, SPPMode PoolMode = SPPMode::Fast>
	class TServoProtocol
	{
	protected:
		static TServoProtocol<T, PoolMode> * pSingleton;
		static std::mutex mCriticalSection;

		int32 Syncword;
		// force to push/pop TSharedPtr
		TNetPacketPool< TSNetPacket<T> >* PacketPool;
		int32 PacketPoolCount;
		Septem::TSharedRecyclePool< TSNetPacket<T> > RecyclePool;

	public:
		virtual ~TServoProtocol()
		{
			pSingleton = nullptr;
			delete PacketPool;
		}

		// thread safe; singleton will init when first call get()
		static TServoProtocol<T, PoolMode>* Get();
		// thread safe; singleton will init when first call getRef()
		static TServoProtocol<T, PoolMode>& GetRef();

		// danger call, but fast
		static TServoProtocol<T, PoolMode>* Singleton();
		// danger call, but fast
		static TServoProtocol<T, PoolMode>& SingletonRef();
		
		// push recv packet into packet pool
		bool Push(const std::shared_ptr< TSNetPacket<T> >& InNetPacket);
		// pop from packet pool
		bool Pop(std::shared_ptr< TSNetPacket<T> >& OutNetPacket);
		int32 PacketPoolNum();

		//=========================================
		//		Net Packet Pool Memory Management
		//=========================================
		static int32 RecyclePoolMaxnum;

		// please call ReUse or set value manulity after recycle alloc
		std::shared_ptr< TSNetPacket<T> > AllocNetPacket();
		// recycle dealloc
		void DeallockNetPacket(const std::shared_ptr< TSNetPacket<T> > & InSharedPtr, bool bForceRecycle = false);
		int32 RecyclePoolNum();

		//=========================================
		//		Net Packet Pool & Recycle Pool Union Control
		//=========================================

		// pop from packetpool to OutRecyclePacket, auto recycle
		bool PopWithRecycle(std::shared_ptr< TSNetPacket<T> >& OutRecyclePacket);

		//=========================================
		//		Events | Lambda | Delegates
		//=========================================
		void OnReceivedPacket(FSNetBufferHead& InHead, uint8* Buffer, int32 BufferSize, int32&ReceivedBytesRead);

	private:
		TServoProtocol()
			:Syncword(DEFAULT_SYNCWORD_INT32)
			, PacketPool(nullptr)
			, PacketPoolCount(0)
			, RecyclePool(RecyclePoolMaxnum)
		{
			check(pSingleton == nullptr && "Protocol singleton can't create 2 object!");
			pSingleton = this;

			switch (PoolMode)
			{
			case SPPMode::Stack:
				PacketPool = new TNetPacketStack< TSNetPacket<T> >();
				break;
			//case SPPMode::Heap:
				//PacketPool = new TNetPacketHeap< TSNetPacket<T> >();
				//break;
			default:
				PacketPool = new TNetPacketQueue< TSNetPacket<T> >();
				break;
			}
		}
	};

	template<typename T,  SPPMode PoolMode>
	TServoProtocol<T,  PoolMode>* TServoProtocol<T,  PoolMode>::pSingleton = nullptr;

	template<typename T,  SPPMode PoolMode>
	std::mutex TServoProtocol<T,  PoolMode>::mCriticalSection;

	template<typename T,  SPPMode PoolMode>
	int32 TServoProtocol<T,  PoolMode>::RecyclePoolMaxnum = 1024;

	template<typename T>
	inline bool TSNetPacket<T>::IsValid()
	{
		return bFastIntegrity && Head.size == Body.MemSize();
	}

	template<typename T>
	inline bool TSNetPacket<T>::CheckIntegrity()
	{
		if (0 == Head.uid)
		{
			return bFastIntegrity = (Head.XOR() ^ Foot.XOR()) == Head.fastcode;
		}
		return bFastIntegrity = (Head.XOR() ^ Body.XOR() ^ Foot.XOR()) == Head.fastcode;
	}

	template<typename T>
	inline uint64 TSNetPacket<T>::GetTimestamp()
	{
		return Foot.timestamp;
	}

	template<typename T>
	inline void TSNetPacket<T>::ReUse(uint8 * Data, int32 BufferSize, int32 & BytesRead, int32 InSyncword)
	{
		sid = 0;
		bFastIntegrity = false;

		Head.syncword = InSyncword;
		// 1. find syncword for head
		int32 index = BufferBufferSyncword(Data, BufferSize, Head.syncword);
		uint8 fastcode = 0;

		if (-1 == index)
		{
			// failed to find syncword in the whole buffer
			BytesRead = BufferSize;
			return;
		}

		// 2. read head
		if (!Head.MemRead(Data + index, BufferSize - index))
		{
			// failed to read from the rest buffer
			BytesRead = BufferSize;
			return;
		}

		index += FSNetBufferHead::MemSize();
		fastcode ^= Head.XOR();

		if (0 == Head.uid)
		{
			sid = Head.SessionID();
		}

		// 3. check and read body
		if (0 != Head.uid)
		{
			//if (!Body.MemRead(Data + index, BufferSize - index, Head.size))
			if (!Body.Deserialize(Data + index, BufferSize - index))
			{
				// failed to read from the rest buffer
				BytesRead = BufferSize;
				return;
			}
			index += Body.MemSize();
			fastcode ^= Body.XOR();
		}

		// 4. read foot
		if (!Foot.MemRead(Data + index, BufferSize - index))
		{
			// failed to read from the rest buffer
			BytesRead = BufferSize;
			return;
		}

		index += FSNetBufferFoot::MemSize();
		fastcode ^= Foot.XOR();

		BytesRead = index;
		bFastIntegrity = fastcode == 0;

		sid = Head.SessionID();

		return;
	}

	template<typename T>
	inline void TSNetPacket<T>::ReUse(FSNetBufferHead & InHead, uint8 * Buffer, int32 BufferSize, int32 & BytesRead)
	{
		sid = 0;
		bFastIntegrity = false;

		// 1. setup head
		Head = InHead;
		int32 index = 0;
		uint8 fastcode = 0;

		fastcode ^= Head.XOR();

		if (0 == Head.uid)
		{
			sid = Head.SessionID();
		}

		// 2. check and read body
		if (0 != Head.uid)
		{
			// size check
			if (Body.MemSize() != Head.size)
			{
				// failed to read from the rest buffer
				BytesRead = BufferSize;
				return;
			}

			if (!Body.Deserialize(Buffer + index, BufferSize - index))
			{
				// failed to read from the rest buffer
				BytesRead = BufferSize;
				return;
			}
			index += Body.MemSize();
			fastcode ^= Body.XOR();
		}

		// 3. read foot
		if (!Foot.MemRead(Buffer + index, BufferSize - index))
		{
			// failed to read from the rest buffer
			BytesRead = BufferSize;
			return;
		}

		index += FSNetBufferFoot::MemSize();
		fastcode ^= Foot.XOR();

		BytesRead = index;
		bFastIntegrity = fastcode == 0;

		sid = Head.SessionID();

		return;
	}

	template<typename T>
	inline void TSNetPacket<T>::WriteToArray(std::vector<uint8>& InBufferArr)
	{
		int32 BytesWrite = 0;
		int32 writeSize = sizeof(FSNetBufferHead) + sizeof(FSNetBufferFoot);
		if (Head.uid > 0ui16)
		{
			writeSize += Body.MemSize();
		}

		InBufferArr.resize(writeSize, 0u);
		uint8* DataPtr = InBufferArr.data();

		//1. write heads
		memcpy(DataPtr, &Head, FSNetBufferHead::MemSize());
		BytesWrite += sizeof(FSNetBufferHead);

		//2. write Body
		if (Head.uid != 0)
		{
			//FMemory::Memcpy(DataPtr + BytesWrite, Body.bufferPtr, Body.length);
			int32 outSize = 0;
			bool bSuccessSerialize = Body.Serialize(DataPtr + BytesWrite, InBufferArr.Num() - BytesWrite, outSize);

			check(bSuccessSerialize);

			BytesWrite += outSize;
		}

		//3. write Foot
		memcpy(DataPtr + BytesWrite, &Foot, FSNetBufferFoot::MemSize());
		BytesWrite += FSNetBufferFoot::MemSize();
	}

	template<typename T>
	inline void TSNetPacket<T>::OnDealloc()
	{
		sid = 0;
		bFastIntegrity = false;
		Body.Reset();
		Head.size = 0;
	}

	template<typename T>
	inline void TSNetPacket<T>::OnAlloc()
	{
		Head.Reset();
		Foot.Reset();

		// timestamp set now 
		Foot.SetNow();
	}

	template<typename T>
	inline bool TSNetPacket<T>::operator<(TSNetPacket<T>&& Other)
	{
		return Foot.timestamp < Other.Foot.timestamp;
	}

	template<typename T>
	inline bool TSNetPacket<T>::operator<(const TSNetPacket<T>& Other)
	{
		return Foot.timestamp < Other.Foot.timestamp;
	}

	template<typename T,  SPPMode PoolMode>
	inline TServoProtocol<T,  PoolMode>* TServoProtocol<T,  PoolMode>::Get()
	{
		if (nullptr == pSingleton) {
			FScopeLock lockSingleton(&mCriticalSection);
			pSingleton = new TServoProtocol<T,  PoolMode>();
		}

		return pSingleton;
	}

	template<typename T,  SPPMode PoolMode>
	inline TServoProtocol<T,  PoolMode>& TServoProtocol<T,  PoolMode>::GetRef()
	{
		if (nullptr == pSingleton) {
			FScopeLock lockSingleton(&mCriticalSection);
			pSingleton = new TServoProtocol<T,  PoolMode>();
		}

		return *pSingleton;
	}

	template<typename T,  SPPMode PoolMode>
	inline TServoProtocol<T,  PoolMode>* TServoProtocol<T,  PoolMode>::Singleton()
	{
		return pSingleton;
	}

	template<typename T,  SPPMode PoolMode>
	inline TServoProtocol<T,  PoolMode>& TServoProtocol<T,  PoolMode>::SingletonRef()
	{
		check(pSingleton);
		return *pSingleton;
	}

	template<typename T,  SPPMode PoolMode>
	inline bool TServoProtocol<T,  PoolMode>::Push(const std::shared_ptr< TSNetPacket<T> >& InNetPacket)
	{
		if (PacketPool->Push(InNetPacket))
		{
			++PacketPoolCount;
			return true;
		}

		return false;
	}

	template<typename T,  SPPMode PoolMode>
	inline bool TServoProtocol<T,  PoolMode>::Pop(std::shared_ptr< TSNetPacket<T> >& OutNetPacket)
	{
		if (PacketPool->Pop(OutNetPacket))
		{
			--PacketPoolCount;
			return true;
		}

		return false;
	}

	template<typename T,  SPPMode PoolMode>
	inline int32 TServoProtocol<T,  PoolMode>::PacketPoolNum()
	{
		return PacketPoolCount;
	}

	template<typename T,  SPPMode PoolMode>
	inline std::shared_ptr< TSNetPacket<T> > TServoProtocol<T,  PoolMode>::AllocNetPacket()
	{
		return RecyclePool.Alloc();
	}

	template<typename T,  SPPMode PoolMode>
	inline void TServoProtocol<T,  PoolMode>::DeallockNetPacket(const std::shared_ptr< TSNetPacket<T> >& InSharedPtr, bool bForceRecycle)
	{
		if (!InSharedPtr.IsValid())
			return;

		InSharedPtr->OnDealloc();
		if (bForceRecycle)
		{
			RecyclePool.DeallocForceRecycle(InSharedPtr);
		}
		else
		{
			RecyclePool.Dealloc(InSharedPtr);
		}
	}

	template<typename T,  SPPMode PoolMode>
	inline int32 TServoProtocol<T,  PoolMode>::RecyclePoolNum()
	{
		return RecyclePool.Num();
	}

	template<typename T,  SPPMode PoolMode>
	inline bool TServoProtocol<T,  PoolMode>::PopWithRecycle(std::shared_ptr< TSNetPacket<T> >& OutRecyclePacket)
	{
		std::shared_ptr< TSNetPacket<T> > newPacket;
		if (Pop(newPacket))
		{
			DeallockNetPacket(OutRecyclePacket);
			OutRecyclePacket = MoveTemp(newPacket);
			return true;
		}

		return false;
	}

	template<typename T,  SPPMode PoolMode>
	inline void TServoProtocol<T,  PoolMode>::OnReceivedPacket(FSNetBufferHead & InHead, uint8 * Buffer, int32 BufferSize, int32 & ReceivedBytesRead)
	{
		std::shared_ptr< TSNetPacket<T> > pPacket(AllocNetPacket());

		pPacket->ReUse(InHead, Buffer, BufferSize, ReceivedBytesRead);

		if (pPacket->IsValid())
		{
			Push(pPacket);
		}
		else {
			// packet is illegal, dealloc shared pointer
			DeallockNetPacket(pPacket);
		}
	}

}
