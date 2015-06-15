#pragma once
#include "OSMEntity.hpp"

#include <array>
#include <utility>
#include <cmath>

struct LatLon {
	double lat;
	double lon;

	template<class Archive>void serialize(Archive& ar,unsigned int ver){ ar & lat & lon; }

	friend std::ostream& operator<<(std::ostream& os,LatLon ll);
};

class OSMNode : public OSMEntity {

	//friend boost::serialization::access;

	// TODO: Do correct base-class serialization (see Boost serialize docs)
//	template<class Archive>void serialize(Archive& ar,unsigned int version)
//		{ OSMEntity::serialize(ar,version); ar & coords_; }

public:
	OSMNode(unsigned long long id_=0,double lat=NAN,double lon=NAN) : OSMEntity(id_),coords_{lat,lon}{}

	void coords(LatLon pos){ coords_=pos; }
	LatLon coords() const { return coords_; }
	LatLon& coords(){ return coords_; }

	friend std::ostream& operator<<(std::ostream&,const OSMNode&);

private:
	LatLon coords_;
};

std::ostream& operator<<(std::ostream&,const OSMNode&);
