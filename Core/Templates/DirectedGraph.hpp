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

#include "GraphType.hpp"

#if UE_STYLE_CONTAINER
#include <Core/Containers/SeptemArray.h>
#include <Core/Containers/SeptemMap.h>
#else
#include <vector>
#include <map>
#endif // UE_STYLE_CONTAINER

#include <string.h>


namespace Septem
{
	namespace GraphTheory
	{
		/*
		*	Directed Graph
		*	No Thread Safe
		*/
		template<typename VT, typename ET>
		class TDirectedGraph
		{
		public:
			virtual void AddVertex(VT& InVT);
			virtual void AddVertex(VT&& InVT);
			virtual bool AddEdge(TEdge<ET>& InEdge);
			virtual void Reset();
			/*
			* unsafe call to get vertex&
			* need to check IsValidVertexIndex before
			*/
			TVertex<VT>& GetVertex(int32 InIndex);
			/*
			* unsafe call to get edge&
			* need to check IsValidEdge before 
			*/
			TEdge<ET>& GetEdge(int32 InStartIndex, int32 InEndIndex);
			/*
			* unsafe call to get edge&
			* need to check IsValidEdge before
			*/
			TEdge<ET>& GetEdge(uint64 InKey);
			
#if UE_STYLE_CONTAINER
			/*
			* get keys array of edge map
			* @ return	the count of the keys
			*/
			int32 GetEdgeKeys(TArray<uint64>& OutKeys);
#else
			/*
			* get keys array of edge map
			* @ return	the count of the keys
			*/
			int32 GetEdgeKeys(std::vector<uint64>& OutKeys);
#endif
			bool IsValidVertexIndex(int32 InIndex);
			bool IsValidEdge(int32 InStartId, int32 InEndId);
			int32 VertexCount();
			SIZE_T VertexSize();
			int32 EdgeCount();
			SIZE_T EdgeSize();
			static uint64 HashEdgeKey(uint64 InStartId, uint64 InEndId);

			void Seriallize(uint8* OutBuffer, SIZE_T& OutSize);
			void Deseriallize(uint8* InBuffer, SIZE_T InSize);
		protected:
			/// can direct to self , default = false
			bool bDirectSelf;

#if UE_STYLE_CONTAINER
			TArray<TVertex<VT>> VertexArray;
			TMap<uint64, TEdge<ET> > EdgeMap;
#else
			std::vector<TVertex<VT>> VertexArray;
			std::map<uint64, TEdge<ET> > EdgeMap;
#endif

		public:
			TDirectedGraph()
				:bDirectSelf(false)
			{
			}
			virtual ~TDirectedGraph() {}
		};

		template<typename VT, typename ET>
		inline void TDirectedGraph<VT, ET>::AddVertex(VT & InVT)
		{
#if UE_STYLE_CONTAINER
			TVertex<VT> vertex
				= { VertexArray.Num(),InVT };
			VertexArray.Add(vertex);
#else
			TVertex<VT> vertex
				= { (int32)VertexArray.size(),InVT };
			VertexArray.push_back(vertex);
#endif
		}

		template<typename VT, typename ET>
		inline void TDirectedGraph<VT, ET>::AddVertex(VT && InVT)
		{
#if UE_STYLE_CONTAINER
			TVertex<VT> vertex 
				= {VertexArray.Num(),InVT};
			VertexArray.Add(vertex);
#else
			TVertex<VT> vertex
				= { (int32)VertexArray.size(),InVT };
			VertexArray.push_back(vertex);
#endif
		}

		template<typename VT, typename ET>
		inline bool TDirectedGraph<VT, ET>::AddEdge(TEdge<ET>& InEdge)
		{
			if (InEdge.StartId == InEdge.EndId)
			{
				if (!bDirectSelf)
					return false;
			}

			if (IsValidVertexIndex(InEdge.StartId) && IsValidVertexIndex(InEdge.EndId))
			{
				uint64 key = HashEdgeKey(InEdge.StartId, InEdge.EndId);//((uint64)InEdge.StartId << 32LL) | (uint64)InEdge.EndId;
#if UE_STYLE_CONTAINER
				if (EdgeMap.Contains(key))
					return false;
				EdgeMap.Add(key, InEdge);
#else
				if (EdgeMap.find(key)!= EdgeMap.end())
					return false;
				EdgeMap.insert(std::pair< uint64, TEdge<ET> >(key, InEdge));
#endif
				return true;
			}

			return false;
		}

		template<typename VT, typename ET>
		inline void TDirectedGraph<VT, ET>::Reset()
		{
#if UE_STYLE_CONTAINER
			VertexArray.Reset();
			EdgeMap.Reset();
#else
			EdgeMap.clear();
			VertexArray.clear();
#endif
		}

		template<typename VT, typename ET>
		inline TVertex<VT> & TDirectedGraph<VT, ET>::GetVertex(int32 InIndex)
		{
			return VertexArray[InIndex];
		}
		template<typename VT, typename ET>
		inline TEdge<ET>& TDirectedGraph<VT, ET>::GetEdge(int32 InStartIndex, int32 InEndIndex)
		{
			uint64 key = HashEdgeKey(InStartIndex, InEndIndex);
			return EdgeMap[key];
		}

		template<typename VT, typename ET>
		inline TEdge<ET>& TDirectedGraph<VT, ET>::GetEdge(uint64 InKey)
		{
			return EdgeMap[InKey];
		}

#if UE_STYLE_CONTAINER		
		template<typename VT, typename ET>
		inline int32 TDirectedGraph<VT, ET>::GetEdgeKeys(TArray<uint64>& OutKeys)
		{
			return EdgeMap.GetKeys(OutKeys);
		}
#else
		template<typename VT, typename ET>
		inline int32 TDirectedGraph<VT, ET>::GetEdgeKeys(std::vector<uint64>& OutKeys)
		{
			int32 ret = (int32)EdgeMap.size();
			OutKeys.clear();
			for (auto itr = EdgeMap.begin(); itr != EdgeMap.end(); itr++)
			{
				OutKeys.push_back(itr->first);
			}
			return ret;
		}
#endif

		template<typename VT, typename ET>
		inline bool TDirectedGraph<VT, ET>::IsValidVertexIndex(int32 InIndex)
		{
#if UE_STYLE_CONTAINER
			return InIndex >= 0 && InIndex < VertexArray.Num();
#else
			return InIndex >= 0 && InIndex < (int32)VertexArray.size();
#endif
		}
		template<typename VT, typename ET>
		inline bool TDirectedGraph<VT, ET>::IsValidEdge(int32 InStartId, int32 InEndId)
		{
			uint64 key = HashEdgeKey((uint64)InStartId, (uint64)InEndId);
#if UE_STYLE_CONTAINER
			return EdgeMap.Contains(key);
#else
			return EdgeMap.find(key) != EdgeMap.end();
#endif
		}
		template<typename VT, typename ET>
		inline int32 TDirectedGraph<VT, ET>::VertexCount()
		{
#if UE_STYLE_CONTAINER
			return VertexArray.Num();
#else
			return (int32)VertexArray.size();
#endif
		}

		template<typename VT, typename ET>
		inline SIZE_T TDirectedGraph<VT, ET>::VertexSize()
		{
#if UE_STYLE_CONTAINER
			return (SIZE_T)VertexArray.Num();
#else
			return VertexArray.size();
#endif
		}

		template<typename VT, typename ET>
		inline int32 TDirectedGraph<VT, ET>::EdgeCount()
		{
#if UE_STYLE_CONTAINER
			return EdgeMap.Num();
#else
			return (int32)EdgeMap.size();
#endif
		}

		template<typename VT, typename ET>
		inline SIZE_T TDirectedGraph<VT, ET>::EdgeSize()
		{
#if UE_STYLE_CONTAINER
			return (SIZE_T)EdgeMap.Num();
#else
			return EdgeMap.size();
#endif
		}

		template<typename VT, typename ET>
		uint64 TDirectedGraph<VT, ET>::HashEdgeKey(uint64 InStartId, uint64 InEndId)
		{
			return (InStartId << 32LL) | InEndId;
		}

		/*
		*	{ Graph Memory }
		*	+ sizeof(bDirectSelf)
		*	+	VertexArray.Num()
		*	+	[	VertexArray		]	x	VertexArray.Num()
		*	+	EdgeMap.Num()
		*	+	[	EdgeMapKeys	]	x	EdgeMap.Num()
		*	+	[	EdgeMapValues	]	x	EdgeMap.Num()
		*	Total Size = 
		*		sizeof(VertexArray.Num()) + sizeof(TVertex<VT>) * VertexArray.Num()	+ sizeof(EdgeMap.Num()) + sizeof(uint64)*EdgeMap.Num() + sizeof(TEdge<ET>)*EdgeMap.Num() + sizeof(bDirectSelf)
		*/
		template<typename VT, typename ET>
		inline void TDirectedGraph<VT, ET>::Seriallize(uint8 * OutBuffer, SIZE_T & OutSize)
		{
			//SIZE_T _tvertexSize = sizeof(TVertex<VT>);
			//SIZE_T _tedgeSize = sizeof(TEdge<ET>);
			OutSize = sizeof(bool) + sizeof(int32) + sizeof(TVertex<VT>) * VertexCount() + sizeof(int32) + sizeof(uint64)*EdgeCount() + sizeof(TEdge<ET>)* EdgeCount();
			OutBuffer = (uint8*)malloc(OutSize);
			SIZE_T _index = 0;
			SIZE_T _Size = 0;

			// *	{ Graph Memory }

			// *	+ sizeof(bDirectSelf)
			_Size = sizeof(bDirectSelf);
			memcpy(OutBuffer + _index, &bDirectSelf, _Size);
			_index += _Size;

			// *	+	VertexArray.Num()
			int32 _VertexCount = VertexCount();
			_Size = sizeof(int32);
			memcpy(OutBuffer + _index, &_VertexCount, _Size);
			_index += _Size;
			// *	+	[	VertexArray		]	x	VertexArray.Num()
			_Size = sizeof(TVertex<VT>);
			for (int32 i = 0; i < _VertexCount; ++i)
			{
				memcpy(OutBuffer + _index, &VertexArray[i], _Size);
				_index += _Size;
			}
			// *	+	EdgeMap.Num()
#if UE_STYLE_CONTAINER
			TArray<uint64> _edgekeys;
			int32 _EdgeCount = EdgeMap.GetKeys(_edgekeys);
			
#else
			std::vector<uint64> _edgekeys;
			int32 _EdgeCount = GetEdgeKeys(_edgekeys);
#endif
			_Size = sizeof(int32);
			memcpy(OutBuffer + _index, &_EdgeCount, _Size);
			_index += _Size;
			
			// *	+	[	EdgeMapKeys	]	x	EdgeMap.Num()
			_Size = sizeof(uint64);
			for (int32 i = 0; i < _EdgeCount; ++i)
			{
				memcpy(OutBuffer + _index, &_edgekeys[i], _Size);
				_index += _Size;
			}
			// *	+	[	EdgeMapValues	]	x	EdgeMap.Num()
			_Size = sizeof(TEdge<ET>);
			for (int32 i = 0; i < _EdgeCount; ++i)
			{
				memcpy(OutBuffer + _index, &EdgeMap[_edgekeys[i]], _Size);
				_index += _Size;
			}

		}

		/*
		*	{ Graph Memory }
		*	bool					+ sizeof(bDirectSelf)
		*	int32					+	VertexArray.Num()
		*	TVertex<VT>	+	[	VertexArray		]	x	VertexArray.Num()
		*	int32					+	EdgeMap.Num()
		*	uint64				+	[	EdgeMapKeys	]	x	EdgeMap.Num()
		*	TEdge<ET>	+	[	EdgeMapValues	]	x	EdgeMap.Num()
		*	Total Size =
		*		sizeof(VertexArray.Num()) + sizeof(TVertex<VT>) * VertexArray.Num()	+ sizeof(EdgeMap.Num()) + sizeof(uint64)*EdgeMap.Num() + sizeof(TEdge<ET>)*EdgeMap.Num() + sizeof(bDirectSelf)
		*/
		template<typename VT, typename ET>
		inline void TDirectedGraph<VT, ET>::Deseriallize(uint8 * InBuffer, SIZE_T InSize)
		{
			//SIZE_T _tvertexSize = sizeof(TVertex<VT>);
			//SIZE_T _tedgeSize = sizeof(TEdge<ET>);

			SIZE_T _index = 0;
			SIZE_T _Size = 0;

			Reset();

			// *	{ Graph Memory }

			// *	+ sizeof(bDirectSelf)
			_Size = sizeof(bDirectSelf);
			memcpy(&bDirectSelf, InBuffer + _index, _Size);
			_index += _Size;

			// *	+	VertexArray.Num()

			int32 _VertexCount = 0;// VertexArray.Num();
			_Size = sizeof(int32);
			memcpy(&_VertexCount, InBuffer + _index, _Size);
			_index += _Size;
#if UE_STYLE_CONTAINER
			VertexArray.Reset(_VertexCount);
#else
			VertexArray.resize((SIZE_T)_VertexCount);
#endif

			// *	+	[	VertexArray		]	x	VertexArray.Num()
			_Size = sizeof(TVertex<VT>);
			TVertex<VT> _tvertex;
			for (int32 i = 0; i < _VertexCount; ++i)
			{
				
				memcpy(&_tvertex, InBuffer + _index, _Size);
				_index += _Size;
				AddVertex(_tvertex.Value);
			}

			// *	+	EdgeMap.Num()
#if UE_STYLE_CONTAINER
			TArray<uint64> _edgekeys;
#else
			std::vector<uint64> _edgekeys;
#endif
			int32 _EdgeCount = 0;
			_Size = sizeof(int32);
			memcpy(&_EdgeCount, InBuffer + _index, _Size);
			_index += _Size;

			// *	+	[	EdgeMapKeys	]	x	EdgeMap.Num()
#if UE_STYLE_CONTAINER
			_edgekeys.Reset(_EdgeCount);
#else
			_edgekeys.clear();
#endif
			_Size = sizeof(uint64);
			uint64 _key;
			for (int32 i = 0; i < _EdgeCount; ++i)
			{
				memcpy(&_key, InBuffer + _index, _Size);
				_index += _Size;
#if UE_STYLE_CONTAINER
				_edgekeys.Add(_key);
#else
				_edgekeys.push_back(_key);
#endif
			}

			// *	+	[	EdgeMapValues	]	x	EdgeMap.Num()
#if UE_STYLE_CONTAINER
			EdgeMap.Reset();
#else
			EdgeMap.clear();
#endif
			_Size = sizeof(TEdge<ET>);
			TEdge<ET> _value;
			for (int32 i = 0; i < _EdgeCount; ++i)
			{
				memcpy(&_value, InBuffer + _index, _Size);
				_index += _Size;
				AddEdge(_value);
			}
		}

	}
}