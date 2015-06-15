#pragma once
#include "OSMEntity.hpp"

#include <boost/serialization/base_object.hpp>

#include <vector>


class OSMWay : public OSMEntity {
//
//	friend boost::serialization::access;
//	template<class Archive>void serialize(Archive& ar,unsigned int version){
//		ar & boost::serialization::base_object<OSMEntity>(*this);
//		ar & ndrefs;
//	}

public:

//	bool is_closed() const { return ndrefs.front() == ndrefs.back(); }

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
};
