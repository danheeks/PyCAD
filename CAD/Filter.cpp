// Filter.cpp
// Copyright (c) 2022, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.
#include "stdafx.h"
#include "Filter.h"

bool CFilter::IsTypeInFilter(int type)const{
	if (type == 0)return false;
	if (m_types.size() == 0)return !m_empty_means_none;
	return (m_types.find(type) != m_types.end());
}

ostream & operator<<(ostream &os, const CFilter &f)
{
	if (f.m_types.size() == 0)
	{
		if (f.m_empty_means_none)
			os << "allow none";
		else
			os << "allow all";
	}
	else
	{
		int i = 0;
		for (std::set<int>::iterator It = f.m_types.begin(); ; It++, i++)
		{
			if (i > 5)
			{
				os << "...";
				break;
			}
			if (It == f.m_types.end())break;
			int t = *It;
			if (i > 0)
				os << " ";
			os << t;
		}
	}
	return os;
}
