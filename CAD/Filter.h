#pragma once
#include <set>

// To support objects from anywhere, which can be added later, I am removing GetMarkingMask for objects.
// Instead, this CFilter class will use the object's GetType
// if there are any types in the set, only those can be picked
// else if types set is empty, any object can be picked
class CFilter
{
	std::set<int> m_types;
public:
	CFilter(bool empty_means_none = false) :m_empty_means_none(empty_means_none){}
	bool m_empty_means_none;
	void Clear(){ m_types.clear(); }
	void AddType(int type){ m_types.insert(type); }
	bool IsTypeInFilter(int type)const{
		if (type == 0)return false;
		if (m_types.size() == 0)return !m_empty_means_none;
		return (m_types.find(type) != m_types.end());
	}
	unsigned int Size(){return m_types.size();}
};
