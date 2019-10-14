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
#include <set>
using namespace std;

#pragma pack(push, 1)

namespace Septem
{
	namespace GraphTheory
	{

		template<typename VT>
		struct TVertex
		{
			int32 Index;
			VT Value;
		};

		/*
		*	edge startId O -> O endId
		*	template weight
		*/
		template<typename ET>
		struct TEdge
		{
			int32 StartId;
			int32 EndId;
			ET Weight;
		};

		/*
		*	edge with adjust link list
		*/
		struct EdgeAdjustList
		{
			int32 Index;
			set<int32> AdjustVertexes;

			EdgeAdjustList(int32 InIndex = 0)
				:Index(0)
			{}
		};
		
	}
}
#pragma pack(pop)