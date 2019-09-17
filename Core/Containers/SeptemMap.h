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
#include <map>
#include <Core/Containers/SeptemArray.h>

namespace Septem
{
	template<typename KeyType, typename ValueType>
	class TMap
	{
	public:
		bool Contains(const KeyType& Key);
		void Add(KeyType& Key, ValueType& Value);
		void Reset();
#if 1	
		int32 GetKeys(TArray<KeyType>& OutKeyTypeArray);
#endif

		int32 Num() const;
		std::size_t Size() const;

		ValueType& operator[](KeyType& Key);
		const ValueType& operator[](KeyType& Key) const;


	private:
		std::map< KeyType, ValueType> m_map;
	};

	template<typename KeyType, typename ValueType>
	inline bool TMap<KeyType, ValueType>::Contains(const KeyType & Key)
	{
		return m_map.find(Key) != m_map.end();
	}
	template<typename KeyType, typename ValueType>
	inline void TMap<KeyType, ValueType>::Add(KeyType & Key, ValueType & Value)
	{
		m_map.insert(std::pair<KeyType, ValueType>(Key, Value));
	}
	template<typename KeyType, typename ValueType>
	inline void TMap<KeyType, ValueType>::Reset()
	{
		m_map.clear();
	}
	template<typename KeyType, typename ValueType>
	inline int32 TMap<KeyType, ValueType>::GetKeys(TArray<KeyType>& OutKeyTypeArray)
	{
		int32 ret = m_map.size();
		OutKeyTypeArray.Reset();
		for (auto itr = m_map.begin(); itr != m_map.end(); itr++)
		{
			OutKeyTypeArray.Add(itr->first);
		}
		return ret;
	}
	template<typename KeyType, typename ValueType>
	inline int32 TMap<KeyType, ValueType>::Num() const
	{
		return m_map.size();
	}
	template<typename KeyType, typename ValueType>
	inline std::size_t TMap<KeyType, ValueType>::Size() const
	{
		return m_map.size();
	}
	template<typename KeyType, typename ValueType>
	inline ValueType & TMap<KeyType, ValueType>::operator[](KeyType & Key)
	{
		return m_map[Key];
	}
	template<typename KeyType, typename ValueType>
	inline const ValueType & TMap<KeyType, ValueType>::operator[](KeyType & Key) const
	{
		return m_map[Key];
	}
}

