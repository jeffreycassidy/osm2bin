/*
 * PathNetwork.hpp
 *
 *  Created on: Jun 15, 2015
 *      Author: jcassidy
 */

#ifndef PATHNETWORK_HPP_
#define PATHNETWORK_HPP_

#include <vector>
#include <string>
#include <unordered_map>

#include <boost/range/algorithm.hpp>

#include <iomanip>
#include <iostream>

#include <boost/container/flat_set.hpp>

#include <unordered_map>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/filtered.hpp>


#include <boost/graph/adjacency_list.hpp>

#include "OSMEntity.hpp"
#include "OSMNode.hpp"
#include "OSMWay.hpp"

#include "KeyValueTable.hpp"
#include "OSMEntityFilter.hpp"

#include "OSMDatabase.hpp"



/** Specialization to pick out roads */

class OSMWayFilterRoads : public OSMEntityFilter<OSMWay> {
public:
	OSMWayFilterRoads(const OSMDatabase& db,const std::vector<OSMNode>& nodes) :
		OSMEntityFilter<OSMWay>(db),
		kvt_(db.wayTags()),
		kiName_(db.wayTags().getIndexForKeyString("name")),
		kiHighway_(db.wayTags().getIndexForKeyString("highway"))
	{
		for (const char * const *s=includeHighwayTagValueStrings_; *s != nullptr; ++s)
			includeHighwayTagValues_.insert(kvt_.getIndexForValueString(*s));
	}

	virtual bool operator()(const OSMWay& e) const override
	{
		unsigned vi = e.getValueForKey(kiHighway_);
		return vi != -1U && includeHighwayTagValues_.count(vi);
	}

private:
	const KeyValueTable& kvt_;
	unsigned kiName_=-1U;
	unsigned kiHighway_=-1U;

	boost::container::flat_set<unsigned> includeHighwayTagValues_;

	static const char * includeHighwayTagValueStrings_[];
};

struct NodeInfo {
	OSMID		osmid=-1ULL;
	LatLon 		latlon;

private:
	friend boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned ver){ ar & osmid & latlon; }
};

struct EdgeProperties {
	std::vector<LatLon>		curvePoints;
	unsigned long long 		wayOSMID=-1U;

	unsigned 				streetVectorIndex=-1U;
	unsigned				streetSegmentVectorIndex=-1U;		// <=== NOT SERIALIZED! MUST BE REBUILT BY PATHNETWORK
	float 					maxspeed=50.0;

	enum Oneway : uint8_t { Bidir=0, ToGreaterVertexNumber=1, ToLesserVertexNumber=2 };

	Oneway 					oneWay=Bidir;

private:
	friend boost::serialization::access;
	template<class Archive>void serialize(Archive& ar,const unsigned ver){ ar & curvePoints & wayOSMID & streetVectorIndex & maxspeed & oneWay; }
};

typedef boost::adjacency_list<
		boost::vecS,				// outedgelist
		boost::vecS,				// vertex list
		boost::undirectedS,			// dir/undir/bidir
		NodeInfo,
		EdgeProperties,
		boost::no_property,
		boost::vecS> PathNetwork;

#ifdef CLANG
#define STDPAIR std::__1::pair
#else
#define STDPAIR std::pair
#endif

template<class Iterator>Iterator begin(STDPAIR<Iterator,Iterator> p,
		typename std::iterator_traits<Iterator>::difference_type __sfinae=0)
	{ return p.first; }

template<class Iterator>Iterator end(STDPAIR<Iterator,Iterator> p,
		typename std::iterator_traits<Iterator>::difference_type __sfinae=0)
	{ return p.second; }


PathNetwork buildNetwork(const OSMDatabase& db,const OSMEntityFilter<OSMWay>& wayFilter);

// build street names for network
class OSMDatabase;
std::vector<std::string> assignStreets(OSMDatabase* db,PathNetwork& G);

void nameIntersections(PathNetwork& G);

#endif /* PATHNETWORK_HPP_ */
