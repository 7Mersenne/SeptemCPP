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

#include "Core/Public/marco.h"

namespace Septem
{
	// Get Fail Array 
	// like getnext in KMP
	// private call for BufferBuffer
	// if you need multi match buffer , need  Aho-Corasick_automation with tire tree
	// make sure: buffer.len == fail.len
	template<typename T>
	static void BufferFailArray(T* buffer, int32* fail, const int32 length)
	{
		check(length > 0);
		check(buffer);
		check(fail);

		fail[0] = -1;
		int32 index = 0;
		int32 failIndex = -1;

		while (index + 1 < length)
		{
			if (-1 == failIndex || buffer[index] == buffer[failIndex])
			{
				
				if (buffer[++index] == buffer[++failIndex])
				{
					fail[index] = fail[failIndex];
				}
				else {
					fail[index] = failIndex;
				}
			}
			else {
				//here must be: failIndex < fail[failIndex];
				failIndex = fail[failIndex];
			}
		}

	}

	// template for buffer strstr
	// find N in M, return the first index or  -1 when faied.
	// BufferBuffer<TChar> ( strP1, strlen1, strP2, strlen2);
	// BufferBuffer<uint8> (Buf1, BLen1, Buf2, Blen2);
	// input:
	// MBuffer		:Buffer M
	// MLength		:Buffer M length
	// NBuffer			:Buffer N
	// NLength		:Buffer N length
	// return int32	: the first index or  -1 when faied.
	template<typename T>
	static int32 BufferBuffer(T * MBuffer, const int32 MLength, T * NBuffer, const int32 NLength)
	{
		const int32 FAIL_CODE = -1;
		int32 i = 0; // index for MBuffer
		int32 j = 0; // index for NBuffer;
		int32 Fail[NLength] = { 0 };

		if (MLength <= 0 || NLength <= 0)
			return FAIL_CODE;

		check(MBuffer);
		check(NBuffer);

		// get fail array
		BufferFailArray(NBuffer, Fail, NLength);

		// Nlength - j <= MLength - i
		// makesure i<MLength && j < NLength
		while (i < MLength && j < NLength)
		{
			if (-1 == j || MBuffer[i] == NBuffer[j])
			{
				// restart match or get match
				++i; ++j;
			}
			else {
				// match failed
				j = Fail[j];
			}
		}

		if (NLength == j)
		{
			return i - NLength;
		}

		return FAIL_CODE;
	}

	// find the first syncword index in buffer
	// return -1 or BufferSize when failed
	static int32 BufferBufferSyncword(uint8* Buffer, int32 BufferSize, int32 Syncword)
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

