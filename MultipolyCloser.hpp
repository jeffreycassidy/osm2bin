/*
 * MultipolyCloser.hpp
 *
 *  Created on: Jan 14, 2016
 *      Author: jcassidy
 */

#ifndef MULTIPOLYCLOSER_HPP_
#define MULTIPOLYCLOSER_HPP_

#include "LatLon.h"
#include "Compass.hpp"
#include <array>

#include <limits>

#include <vector>
#include <list>

#include "OSMDatabase.hpp"


/** Stitches multi-way relations together by joining endpoints and optionally closing around.
 *
 */

class MultipolyCloser
{
public:
	MultipolyCloser(const OSMDatabase& db, const std::vector<const OSMWay*>& ways);

	enum Direction 	{ CCW=-1,None=0,CW=1 };			///< Closing orientation (None=do not add boundary points to close loops)
	void direction(Direction dir){ m_dir=dir; }		///< Set direction for closing around boundaries (CW/CCW/None)

	/// Return vector of coordinates for each unresolved way
	std::vector<std::vector<LatLon>> unresolvedWays() const;

	enum Boundedness { Bounded, Unbounded, All };

	/// Return vector of coordinates for, and a flag that indicates if the loop is bounded
	std::vector<std::pair<std::vector<LatLon>,bool>> 	loops(Boundedness=All) const;
	unsigned											loopCount(Boundedness=All) const;


private:


	/// Where exactly two endpoints start or end at a single OSM Node ID, link them together and remove them from the unresolved list
	void matchEndpointsByNodeID();

	/// For each remaining endpoint, close to the boundary subject to m_maxBoundaryDistance
	void closeEndpointsToBoundary();

	/// Traverse boundary, forming links between segments and adding corner points as needed
	void closeAroundBoundary();

	/// Create a list of distinct closed loops from the segment links
	void buildLoops();


	/// Family of classes to represent links between segments and boundary points
	class Segment;
		class WaySegment;
		class BoundaryPointSegment;

	/// Class to hold unresolved references to endpoints
	struct Endpoint;

	static constexpr float metresPerDegreeLat = 1852.0f * 60.0f;

	std::array<float,4> 	distanceToBounds(LatLon p) const;
	LatLon 					closeToBoundary(LatLon p,CompassCardinal which) const;


	struct EdgePoint {
		LatLon 			p;				// edge point
		float			edgeDistance;	// distance from origin point
		CompassCardinal	edge;			// which edge
		float 			cwDistance;		// distance clockwise from NW (upper-left) corner
	};

	EdgePoint 				nearestPointOnEdge(LatLon p) const;
	LatLon 					corner(CompassIntercardinal c) const;
	float					cornerCWDistance(CompassIntercardinal c) const;

	Direction						m_dir=CW;

	const OSMDatabase&				m_db;
	std::vector<const OSMWay*>		m_ways;
	std::pair<LatLon,LatLon> 		m_bounds;

	std::vector<bool>				m_wsAdded;		///< Flag per way segment indicating it's been added to output

	float							m_maxBoundaryDistance=std::numeric_limits<float>::infinity();


	/** invariant: each way segment is referenced at least once in either endpoints, or in a waysegment loop pointed to by
	 * a member of endpoints
	 */

	std::list<Endpoint> 			m_endpoints;	///< List of unresolved endpoints

	std::vector<WaySegment> 		m_waySegments;		// NOTE: algorithm relies on stable iterators since we use pointers to elements
														// must reserve correct size and cannot insert/delete

	std::vector<Segment*>			m_loops;

	std::list<BoundaryPointSegment>	m_boundpoints;
};


class MultipolyCloser::Segment
{
public:
	// query start/end points
	virtual LatLon first() const=0;
	virtual LatLon last() const=0;

	// add the segment to a vector
	virtual void add(std::vector<LatLon>& v) const=0;

	virtual void print(std::ostream& os) const=0;

	bool prepend(Segment* s)
	{
		if(m_prev || !s || s->m_next)
			return false;
		s->m_next=this;
		m_prev=s;
		return true;
	}

	bool append(Segment* s)
	{
		if (m_next || !s ||s->m_prev)
			return false;
		m_next=s;
		s->m_prev=this;
		return true;
	}

	bool hasNext() const	{ return m_next; }
	bool hasPrev() const 	{ return m_prev; }

	bool isMemberOfClosedLoop() const
	{
		// traverse to beginning/end, returning false if we hit a null ref
		for(const Segment* s=m_next; s != this; s=s->m_next)
			if (s==nullptr)
				return false;

		for(const Segment* s=m_prev; s != this; s=s->m_prev)
			if (s==nullptr)
				return false;

		return true;
	}

	Segment* nextEndingAt(const Segment* e) const { assert(m_next); return m_next == e ? nullptr : m_next; }
	Segment* prevEndingAt(const Segment* e) const { assert(m_prev); return m_prev == e ? nullptr : m_prev; }

protected:
	Segment 	*m_next=nullptr,*m_prev=nullptr;

	friend std::ostream& operator<<(std::ostream& os,const MultipolyCloser::Segment& s)
	{
		s.print(os);
		return os;
	}
};


class MultipolyCloser::BoundaryPointSegment : public MultipolyCloser::Segment
{
public:
	BoundaryPointSegment(CompassPrincipal c,LatLon p,float distanceToEdge,float cwDistance,bool isStart=false,bool isEnd=false) :
		m_location(c),
		m_latlon(p),
		m_edgeDistance(distanceToEdge),
		m_cwDistance(cwDistance),
		m_isStart(isStart),
		m_isEnd(isEnd)
	{
	}

	BoundaryPointSegment(CompassIntercardinal corner,LatLon p,float cwDistance) :
		m_location(corner),
		m_latlon(p),
		m_cwDistance(cwDistance)
	{}

	BoundaryPointSegment(CompassCardinal edge,LatLon p,float distanceToEdge,float cwDistance,bool isStart) :
		m_location(edge),
		m_latlon(p),
		m_edgeDistance(distanceToEdge),
		m_cwDistance(cwDistance),
		m_isStart(isStart),
		m_isEnd(!isStart)
	{
	}



	bool isStart() 	const { return m_isStart; 	}
	bool isEnd() 	const { return m_isEnd; 	}

	virtual LatLon first() const override { return m_latlon; }
	virtual LatLon last()  const override { return m_latlon; }

	virtual void add(std::vector<LatLon>& v) const override
	{
		v.push_back(m_latlon);
	}

	virtual void print(std::ostream& os) const override
	{
		os << "[ " << (m_isStart ? "start" : "") << (m_isEnd ? "end" : "") << "point (" << m_latlon.lat << ',' << m_latlon.lon << ')' << " " << m_location << "]";
	}

	struct OrientedOrder
	{
		OrientedOrder(Direction dir) :
			m_dir(dir)
		{}
		bool operator()(const BoundaryPointSegment& lhs,const BoundaryPointSegment& rhs) const
			{ return m_dir == CW ? lhs.m_cwDistance < rhs.m_cwDistance : rhs.m_cwDistance < lhs.m_cwDistance; }

		Direction m_dir=CW;
	};

	CompassPrincipal	m_location;				// edge (N/S/E/W) if terminal of a way, else corner
	LatLon				m_latlon;				// coordinates
	float				m_edgeDistance=0.0f;	// distance from point to edge
	float				m_cwDistance;			// clockwise "distance" (sum of angles) along bounds from NW corner

	bool 				m_isStart=false;
	bool				m_isEnd=false;

};

class MultipolyCloser::WaySegment : public MultipolyCloser::Segment
{
public:
	WaySegment(){}
	WaySegment(const OSMDatabase& db,const OSMWay* w,unsigned idx=-1U) :
		m_wayPtr(w),
		m_points(db.extractPoly(*w)),
		m_way(idx)
	{
	}

	OSMID nodeRefFirst() 	const { return m_wayPtr->ndrefs().front(); 	}
	OSMID nodeRefLast() 	const { return m_wayPtr->ndrefs().back(); 	}

	void linkSelf()
	{
		assert(!m_next);
		assert(!m_prev);
		m_next=m_prev=this;
	}

	virtual LatLon first() const override { return m_points.front(); }
	virtual LatLon last() const override { return m_points.back(); }

	void reverse(bool r)
	{
		m_reverse=r;
		if (m_reverse != r)
			std::swap(m_next,m_prev);
	}

	virtual void add(std::vector<LatLon>& v) const override
	{
		if (m_reverse)
			v.insert(v.end(),m_points.rbegin(),m_points.rend());
		else
			v.insert(v.end(),m_points.begin(),m_points.end());
	}

	virtual void print(std::ostream& os) const override
	{
		os << "S" << m_way;
		if (m_reverse)
			os << 'R';
	}

	unsigned index() const { return m_way; }

private:
	bool				m_reverse=false;
	unsigned			m_way=-1U;
	std::vector<LatLon>	m_points;
	const OSMWay*		m_wayPtr=nullptr;
};


struct MultipolyCloser::Endpoint
{
	OSMID		osmNodeID;
	bool		isWayFirst;
	WaySegment* waySegment=nullptr;
	unsigned	way=-1U;

	Endpoint(WaySegment* s,OSMID nID,bool isStart_,unsigned way_) : osmNodeID(nID),isWayFirst(isStart_),waySegment(s),way(way_){}

	LatLon pos() const { return isWayFirst ? waySegment->first() : waySegment->last(); }

	LatLon			pBound;														// nearest point on boundary
	CompassCardinal	edge;
	float 			cwPerimeter=std::numeric_limits<float>::quiet_NaN();		// clockwise distance from NW along perimeter
	float 			distanceToEdge=std::numeric_limits<float>::quiet_NaN();		// distance from bounding point to terminal way point

	// Order endpoints by OSMNodeID, placing endpoints before startpoints
	struct NodeIDOrder
	{
		bool operator()(const Endpoint& lhs,const Endpoint& rhs) 	const { return lhs.osmNodeID < rhs.osmNodeID || (lhs.osmNodeID == rhs.osmNodeID && rhs.isWayFirst); 	}
	};

	static bool NodeIDEqual(const Endpoint& lhs,const Endpoint& rhs) 	{ return lhs.osmNodeID == rhs.osmNodeID; 	}
	static bool EndpointEqual(const Endpoint& lhs,const Endpoint& rhs)	{ return lhs.osmNodeID == rhs.osmNodeID && lhs.isWayFirst == rhs.isWayFirst; }
};

#endif /* MULTIPOLYCLOSER_HPP_ */
