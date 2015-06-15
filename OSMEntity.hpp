#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>

class OSMEntity {
public:
	OSMEntity(unsigned long long id_) : id_(id_){}

	OSMEntity(OSMEntity&&) 					= default;
	OSMEntity(const OSMEntity&) 			= default;
	OSMEntity& operator=(const OSMEntity&) 	= default;
	OSMEntity& operator=(OSMEntity&&)		= default;

	virtual ~OSMEntity(){}

	void addTag(unsigned k,unsigned v){ tags_.push_back(std::make_pair(k,v)); }

	unsigned long long id() const { return id_; }

	// comparison functions
	static bool IDEqual(const OSMEntity& a,const OSMEntity& b){ return a.id_==b.id_; }
	static bool IDOrder(const OSMEntity& a,const OSMEntity& b){ return a.id_<b.id_; }

	void id(unsigned long long newID){ id_=newID; }

	const std::vector<std::pair<unsigned,unsigned>>& tags() const { return tags_; }

	void sortTags(){ std::sort(tags_.begin(),tags_.end(),
			[](const std::pair<unsigned,unsigned> lhs,const std::pair<unsigned,unsigned> rhs){ return lhs.first<rhs.first; } ); }

private:
	unsigned long long id_;
	std::vector<std::pair<unsigned,unsigned>> tags_;

	template<class Archive>void serialize(Archive& ar,const unsigned ver){ ar & id_ & tags_; }
	friend class boost::serialization::access;
};
