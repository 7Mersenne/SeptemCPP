// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.

#pragma once

#include <Core/Public/marco.h>

#include "NetPacketPool.hpp"
#include <Core/Templates/SeptemRecyclePool.hpp>
#include <vector>
#include <mutex>

//#define SERVO_PROTOCOL_SIGNATURE

#ifndef DEFAULT_SYNCWORD_INT32
#define DEFAULT_SYNCWORD_INT32 0xE6B7F1A2
#endif // !DEFAULT_SYNCWORD_INT32

/*
* Setting max in packet pool
* Defend memory boom
* Pool Healthy = pool.num() / max
* Healthy > 60% means dangerous 
* if max is too small, server will lose packets
*/
#ifndef SERVO_PROTOCOL_PACKET_POOL_MAX
#define SERVO_PROTOCOL_PACKET_POOL_MAX 1024
#endif // !SERVO_PROTOCOL_PACKET_POOL_MAX

namespace Septem
{

	/***************************************/
	// net buffer design:
	// buffer head
	// buffer body
	// (optional) buffer foot
	/***************************************/
#pragma pack(push, 1)
	struct FSNetBufferHead
	{
		int32 syncword; // combine with 4 char(uint8).  make them differents to get efficient  
		/*
		* version & 1 = 1 means serialize body, 0 means buffer body
		*/
		uint8 version;
		uint8 fastcode;  // xor value without this byte
		uint16 uid; // unique class id
		int32 size; // body size
		uint32 reserved; // 64x alignment 

		FSNetBufferHead() :
			syncword(DEFAULT_SYNCWORD_INT32),
			version(0),
			fastcode(0),
			uid(0),
			size(0),
			reserved(0)
		{
		}

		FSNetBufferHead& operator=(const FSNetBufferHead& Other);
		bool MemRead(uint8 *Data, int32 BufferSize);
		static int32 MemSize();
		uint8 XOR();
		void Reset();
		int32 SessionID();

		//-----------------------------------------------------------------------------------------
		//	version judgement
		//-----------------------------------------------------------------------------------------
		bool IsSerializedPacket();
	};

	union UnionBufferHead
	{
		uint8 byte[16]; // 4x 32bit
		FSNetBufferHead bufferHead;
	};

	union UnionSyncword
	{
		uint8 byte[4]; // 4 char
		int32 value;
	};

	union Union32
	{
		uint8 byte[4]; // 4 char
		int32 v_int32;
		float v_float;
	};

	//0xE6B7F1A2	little endian
	static UnionSyncword SyncwordDefault =
#if PLATFORM_LITTLE_ENDIAN > 0
	{ 162u, 241u, 183u, 230u };
#else
	{ 230u, 183u, 241u, 162u };
#endif

#pragma pack(pop)


	struct FSNetBufferBody
	{
		uint8* bufferPtr;
		int32 length; // lenght == BufferHead.size;

		FSNetBufferBody()
			: bufferPtr(nullptr)
			, length(0)
		{}

		~FSNetBufferBody()
		{
			Reset();
		}

		bool IsValid();
		bool MemRead(uint8 *Data, int32 BufferSize, int32 InLength);
		int32 MemSize();
		uint8 XOR();

		void Reset();

		bool GetInt32(int32 InIndex, int32& OutValue)
		{
			if (InIndex + 3 < length)
			{
				OutValue = 0;
				for (int32 i = 0; i < 4; ++i)
				{
					int32 n = bufferPtr[InIndex + i];
					OutValue += n << (i * 8);
				}
				return true;
			}
			return false;
		}

		bool GetFloat(int32 InIndex, float& OutValue)
		{
			Union32 m_float = { 0 };
			if (InIndex + 3 < length)
			{
				for (int32 i = 0; i < 4; ++i)
				{
					m_float.byte[i] = bufferPtr[InIndex + i];
				}
				OutValue = m_float.v_float;
				return true;
			}
			return false;
		}
	};

	/***************************************************/
	/*
		version 0:
			only timestamp;
		version 1:
			[ signature, timestamp]
	*/
	/***************************************************/
#pragma pack(push, 1)
	struct FSNetBufferFoot
	{
		// if use version, makesure #if SERVO_PROTOCOL_VERSION > 1 
#ifdef SERVO_PROTOCOL_SIGNATURE
		FSHA256Signature signature;
#endif // SERVO_PROTOCOL_SIGNATURE
		uint64 timestamp; // unix timestamp

		FSNetBufferFoot()
			: timestamp(0)
		{
		}

		FSNetBufferFoot(uint64 InTimestamp)
			: timestamp(InTimestamp)
		{
		}

		bool MemRead(uint8 *Data, int32 BufferSize);
		static int32 MemSize();
		uint8 XOR();

		void Reset()
		{
#ifdef SERVO_PROTOCOL_SIGNATURE
			memset(signature, 0, sizeof(signature));
#endif // SERVO_PROTOCOL_SIGNATURE

			timestamp = 0ULL;
		}

		void SetNow();
	};

	/************************************************************/
	/*
			Heartbeat:
			[head]
				uid == 0
				reserved == heart beat ext data
				size == 0
				reserved = client input 32bit session

			recv buffer no body
			[foot]
				(signature)
				timestamp = FPlatformTime::Cycles64(); // when create
			[session]
	*/
	/************************************************************/
	struct FSNetPacket
	{
		FSNetBufferHead Head;
		FSNetBufferBody Body;
		FSNetBufferFoot Foot;

		// session id
		int32 sid;
		bool bFastIntegrity;

		bool IsValid();

		// check data integrity with fastcode
		static bool FastIntegrity(uint8* DataPtr, int32 DataLength, uint8 fastcode);

		bool CheckIntegrity();

		FSNetPacket()
			: sid(0)
			, bFastIntegrity(false)
		{
		}

		FSNetPacket(uint8* Data, int32 BufferSize, int32& BytesRead, int32 InSyncword = DEFAULT_SYNCWORD_INT32);
		uint64 GetTimestamp();

		static FSNetPacket* CreateHeartbeat(int32 InSyncword = DEFAULT_SYNCWORD_INT32);
		void ReUse(uint8* Data, int32 BufferSize, int32& BytesRead, int32 InSyncword = DEFAULT_SYNCWORD_INT32);
		void ReUse(FSNetBufferHead& InHead, uint8* Data, int32 BufferSize, int32& BytesRead);
		void WriteToArray(std::vector<uint8>& InBufferArr);
		void OnDealloc();
		void OnAlloc();
		void ReUseAsHeartbeat(int32 InSyncword = DEFAULT_SYNCWORD_INT32);
		bool operator < (const FSNetPacket& Other);
	};

#pragma pack(pop)

	/************************************************************/
	/*
			Help
			TSharedPtr<FSNetPacket> packet = MakeShareable(FSNetPacket::Create(Data, BufferSize, BytesRead));
			TSharedPtr<FSNetPacket, ESPMode::ThreadSafe> packet = MakeShared<FSNetPacket, ESPMode::ThreadSafe>(?);
	*/
	/************************************************************/

	/**
	 * the protocol of SeptemServo
	 * singleton for handle pools
	 * FORWARD: typedef singleton WorkPool<Part>
	 */
	class FServoProtocol
	{
	private:
		FServoProtocol();
	public:
		virtual ~FServoProtocol();

		// thread safe; singleton will init when first call get()
		static FServoProtocol* Get();
		// thread safe; singleton will init when first call getRef()
		static FServoProtocol& GetRef();

		// danger call, but fast
		static FServoProtocol* Singleton();
		// danger call, but fast
		static FServoProtocol& SingletonRef();

		// push recv packet into packet pool
		bool Push(const std::shared_ptr<FSNetPacket>& InNetPacket);
		// pop from packet pool
		bool Pop(std::shared_ptr<FSNetPacket>& OutNetPacket);
		int32 PacketPoolNum();

		//=========================================
		//		Net Packet Pool Memory Management
		//=========================================
		static int32 RecyclePoolMaxnum;

		// please call ReUse or set value manulity after recycle alloc
		std::shared_ptr<FSNetPacket> AllocNetPacket();
		std::shared_ptr<FSNetPacket> AllocHeartbeat();
		// recycle dealloc
		void DeallockNetPacket(const std::shared_ptr<FSNetPacket>& InSharedPtr, bool bForceRecycle = false);
		int32 RecyclePoolNum();

		//=========================================
		//		Net Packet Pool & Recycle Pool Union Control
		//=========================================

		// pop from packetpool to OutRecyclePacket, auto recycle
		bool PopWithRecycle(std::shared_ptr<FSNetPacket>& OutRecyclePacket);
	protected:
		static FServoProtocol* pSingleton;
		static std::mutex mCriticalSection;

		int32 Syncword;

		// force to push/pop TSharedPtr
		TNetPacketPool<FSNetPacket>* PacketPool;
		int32 PacketPoolCount;
		Septem::TSharedRecyclePool<FSNetPacket> RecyclePool;
	};


}