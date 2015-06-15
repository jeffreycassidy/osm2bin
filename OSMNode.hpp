#pragma once
#include "OSMEntity.hpp"

#include <array>
#include <utility>
#include <cmath>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

struct LatLon {
	double lat;
	double lon;

	friend std::ostream& operator<<(std::ostream& os,LatLon ll);

private:
	template<class Archive>void serialize(Archive& ar,unsigned int ver){ ar & lat & lon; }
	friend class boost::serialization::access;
};

class OSMNode : public OSMEntity {
public:
	OSMNode(unsigned long long id_=0,double lat=NAN,double lon=NAN) : OSMEntity(id_),coords_{lat,lon}{}
	OSMNode(OSMNode&&) = default;
	OSMNode(const OSMNode&) = default;
	OSMNode& operator=(OSMNode&&) = default;

	void coords(LatLon pos){ coords_=pos; }
	LatLon coords() const { return coords_; }
	LatLon& coords(){ return coords_; }

	friend std::ostream& operator<<(std::ostream&,const OSMNode&);

private:
	LatLon coords_;

	template<class Archive>void serialize(Archive& ar,const unsigned)
		{ ar & boost::serialization::base_object<OSMEntity>(*this) & coords_; }
	friend boost::serialization::access;
};

std::ostream& operator<<(std::ostream&,const OSMNode&);
