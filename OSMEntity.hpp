#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>

class OSMEntity {
public:
	OSMEntity(unsigned long long id_) : id_(id_){}

	OSMEntity(OSMEntity&&) 					= default;
	OSMEntity(const OSMEntity&) 			= default;
	OSMEntity& operator=(const OSMEntity&) 	= default;

	virtual ~OSMEntity(){}

	void addTag(unsigned k,unsigned v){ tags_.push_back(std::make_pair(k,v)); }

	unsigned long long id() const { return id_; }

	// comparison functions
	static bool IDEqual(const OSMEntity& a,const OSMEntity& b){ return a.id_==b.id_; }
	static bool IDOrder(const OSMEntity& a,const OSMEntity& b){ return a.id_<b.id_; }

	//template<class Archive>void serialize(Archive& ar,unsigned int version){ ar & id_ & tags_; }

	void id(unsigned long long newID){ id_=newID; }

	const std::vector<std::pair<unsigned,unsigned>>& tags() const { return tags_; }

	//typedef std::vector<std::pair<unsigned,unsigned>>::const_iterator tag_const_iterator;

private:
	unsigned long long id_;
	std::vector<std::pair<unsigned,unsigned>> tags_;

	//friend class OSMEntityHandler;
	friend class OSMNodeElementHandler;
	friend class OSMAttributeHandler;

	template<class Archive>void serialize(Archive& ar,const unsigned ver){ ar & id_ & tags_; }
	friend class boost::serialization::access;
};
