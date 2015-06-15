#pragma once
#include "OSMEntity.hpp"

#include <vector>


class OSMWay : public OSMEntity {


public:
	OSMWay(const OSMWay&) = default;
	OSMWay(OSMWay&& w) = default;
	OSMWay& operator=(const OSMWay&) = default;

	OSMWay(unsigned long long id_=0) : OSMEntity(id_){}

	void addNode(unsigned long long id_){ ndrefs_.push_back(id_); }

	//unsigned getNRefs() const { return ndrefs.size(); }

	//const std::vector<unsigned long long>& getRefs() const { return ndrefs; }

	//std::pair<std::vector<unsigned long long>::const_iterator,std::vector<unsigned long long>::const_iterator> getRefsRange() const { return make_pair(ndrefs.begin(),ndrefs.end()); }

	const std::vector<unsigned long long>& ndrefs() const { return ndrefs_; }

private:
	std::vector<unsigned long long> ndrefs_;

	template<class Archive>void serialize(Archive& ar,unsigned int)
		{ ar & boost::serialization::base_object<OSMEntity>(*this) & ndrefs_; }
	friend boost::serialization::access;
};
