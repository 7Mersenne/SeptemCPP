// Copyright (c) 2013-2019 7Mersenne All Rights Reserved.


#include "StaticNetBodyBase.h"
#include <memory.h>

namespace Septem {
	void FStaticNetBodyBase::Reset()
	{
	}

	bool FStaticNetBodyBase::Serialize(uint8* Data, int32 BufferSize, int32& OutSize)
	{
		OutSize = MemSize();
		if (BufferSize < OutSize)
		{
			OutSize = 0;
			return false;
		}

		memcpy(Data, this, OutSize);

		return true;
	}


	bool FStaticNetBodyBase::Deserialize(uint8* Data, int32 BufferSize)
	{
		if (BufferSize < MemSize())
			return false;

		memcpy(this, Data, MemSize());

		return true;
	}

	int32 FStaticNetBodyBase::MemSize()
	{
		// return sizeof(T);
		return 0;
	}

	uint8 FStaticNetBodyBase::XOR()
	{
		return 0u;
	}
}
