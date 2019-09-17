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

#ifndef UE_STYLE_CONTAINER
#include <set>
#else
namespace Septem
{
	template<typename T>
	class TSet
	{
	public:
		void Add(T& InT);
	private:
		std::set<T> m_set;
	};
	template<typename T>
	inline void TSet<T>::Add(T & InT)
	{
		m_set.insert(InT);
	}
}
#endif // !UE_STYLE_CONTAINER

