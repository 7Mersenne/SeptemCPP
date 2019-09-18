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

#if UE_STYLE_CONTAINER
#include <vector>

namespace Septem
{

	template<typename T>
	class TArray
	{
	public:
		TArray()
		{}

		void Reset(int32 InNum = 0);

		int32 Num() const;
		std::size_t Size() const;

		const T& operator[](int32 Index) const;
		T& operator[](int32 Index);

		int32 Add(T&& Item);
		int32 Add(const T& Item);

	private:
		std::vector<T> m_vector;
	};
	template<typename T>
	inline void TArray<T>::Reset(int32 InNum)
	{
		m_vector.resize((SIZE_T)InNum);
		m_vector.clear();
	}
	template<typename T>
	inline int32 TArray<T>::Num() const
	{
		return m_vector.size();
	}
	template<typename T>
	inline std::size_t TArray<T>::Size() const
	{
		return m_vector.size();
	}
	template<typename T>
	inline const T & TArray<T>::operator[](int32 Index) const
	{
		return m_vector[Index];
	}
	template<typename T>
	inline T & TArray<T>::operator[](int32 Index)
	{
		return m_vector[Index];
	}
	template<typename T>
	inline int32 TArray<T>::Add(T && Item)
	{
		int32 ret = m_vector.size();
		m_vector.push_back(Item);
		return ret;
	}
	template<typename T>
	inline int32 TArray<T>::Add(const T & Item)
	{
		int32 ret = m_vector.size();
		m_vector.push_back(Item);
		return ret;
	}
}
#else
#include <vector>
#endif // UE_STYLE_CONTAINER
