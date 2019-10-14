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

#include "SeptemBuffer.h"

namespace Septem {
	int32 BufferBufferSyncword(uint8 * Buffer, int32 BufferSize, int32 Syncword)
	{
		int32 index = 0;
		int32 maxdex = BufferSize - 4;
		while (index < maxdex)
		{
			int32* ptr = (int32*)(Buffer + index);
			if (*ptr == Syncword)
				return index;

			++index;
		}

		return -1;
	}
}


