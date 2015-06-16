/*
 * MultipolyCloser.cpp
 *
 *  Created on: Jan 14, 2016
 *      Author: jcassidy
 */

#include <array>
#include "LatLon.h"
#include "MultipolyCloser.hpp"

#include <boost/range/algorithm.hpp>

#include <boost/math/constants/constants.hpp>

using namespace std;


/// Convenience function for cosine in degrees
float cosd(float theta){ return cos(boost::math::constants::two_pi<float>()/360.0f*theta); }


template<std::size_t I>struct CompareTupleElement
{
	template<typename T>static const T& extract(const T& t){ return t; }
	template<typename... Types>static const tuple_element<I,tuple<Types...>>& extract(const tuple<Types...>& t){ return get<I>(t); }

	template<typename T,typename U>bool operator()(const T& lhs,const U& rhs) const
	{
		return extract(lhs) < extract(rhs);
	}
};


/** Computes the distance (metres) from a point to the north/east/south/west bounds (in that order)
 * 		@param		p		The point to compute
 * 		@param		bounds	The latlon bounds (SW, NE corners)
 */

array<float,4> MultipolyCloser::distanceToBounds(LatLon p) const
{
	float cosphi=cosd(p.lat);
	return array<float,4>
	{
		metresPerDegreeLat*fabs(m_bounds.second.lat - p.lat),			// north edge
		metresPerDegreeLat*fabs((m_bounds.second.lon - p.lon)*cosphi),	// east edge
		metresPerDegreeLat*fabs(m_bounds.first.lat - p.lat),				// south edge
		metresPerDegreeLat*fabs((m_bounds.first.lon - p.lon)*cosphi)		// west edge
	};
}



/** Closes the end of a poly to the specified boundary.
 * 		@param		p 		The point to close
 * 		@param		which	The boundary to close to
 * 		@param		bounds	The (min,max) bounds
 */

LatLon MultipolyCloser::closeToBoundary(LatLon p,CompassCardinal which) const
{
	switch(which)
	{
		case CompassCardinal::N:	return(LatLon(m_bounds.second.lat,p.lon));
		case CompassCardinal::E: 	return(LatLon(p.lat,m_bounds.second.lon));
		case CompassCardinal::S: 	return(LatLon(m_bounds.first.lat,p.lon));
		case CompassCardinal::W:	return(LatLon(p.lat,m_bounds.first.lon));
	}

	throw std::logic_error("closeToBoundary: invalid boundary requested");
}




/** Computes the nearest point on the boundary */

MultipolyCloser::EdgePoint MultipolyCloser::nearestPointOnEdge(LatLon p) const
{
	array<float,4> dToBounds = distanceToBounds(p);
	const auto nearestBound = boost::min_element(dToBounds,CompareTupleElement<0>());

	CompassCardinal edge(nearestBound-dToBounds.begin());

	LatLon pBound = closeToBoundary(p,edge);

	float ew=m_bounds.second.lon-m_bounds.first.lon;
	float ns=m_bounds.second.lat-m_bounds.first.lat;

	float cwd=0;		// clockwise distance around bounds from NW

	switch(edge)
	{
		case CompassCardinal::N: cwd=pBound.lon-m_bounds.first.lon; break;
		case CompassCardinal::E: cwd=ew+m_bounds.second.lat-pBound.lat; break;
		case CompassCardinal::S: cwd=ew+ns+m_bounds.second.lon-pBound.lon; break;
		case CompassCardinal::W: cwd=2*ew+ns+pBound.lat-m_bounds.first.lat; break;
	}

	return { pBound, *nearestBound, edge, cwd };
}



LatLon MultipolyCloser::corner(CompassIntercardinal c) const
{
	switch(c)
	{
		case CompassIntercardinal::NE: return m_bounds.second;
		case CompassIntercardinal::SE: return LatLon(m_bounds.first.lat,m_bounds.second.lon);
		case CompassIntercardinal::SW: return m_bounds.first;
		case CompassIntercardinal::NW: return LatLon(m_bounds.second.lat,m_bounds.first.lon);
	}

	throw std::logic_error("corner called with an invalid intercardinal point");
}



/** Returns clockwise "distance" (dlat+dlon along bounds) from NW to corner point
 */

float MultipolyCloser::cornerCWDistance(CompassIntercardinal c) const
{
	float ew=m_bounds.second.lon-m_bounds.first.lon;
	float ns=m_bounds.second.lat-m_bounds.first.lat;

	switch(c)
	{
	case CompassIntercardinal::NE: return ew;
	case CompassIntercardinal::SE: return ew+ns;
	case CompassIntercardinal::SW: return 2.0f*ew+ns;
	case CompassIntercardinal::NW: return 0.0f;
	}
	throw std::logic_error("cornerCWDistance: invalid intercardinal point");
}



/** Returns clockwise "distance" (dlat+dlon along bounds) from NW to corner point
 */

float cornerCWDistance(CompassPrincipal p,pair<LatLon,LatLon> bounds)
{
	unsigned u = unsigned(p);
	if((u%2)!=1)
		throw std::logic_error("cornerCWDistance: invalid compass principal point");
	return cornerCWDistance(CompassIntercardinal(u>>1),bounds);
}



/** Joins a set of ways end-to-end, and closes to the boundary where they terminate at a boundary.
 * Returns a vector of pair<vector,bool>. The bool indicates if the way way originally closed (ie. did not need bounding points added to close)
 *
 */

MultipolyCloser::MultipolyCloser(const OSMDatabase& db,const vector<const OSMWay*>& ways) :
		m_db(db),
		m_ways(ways),
		m_bounds(db.bounds()),
		m_waySegments(ways.size()),
		m_wsAdded(ways.size(),false)
{
	////// Extract polygons and form list of endpoints

	for(unsigned i=0;i<ways.size();++i)
	{
		// extract the polygons
		WaySegment& ws = m_waySegments[i] = WaySegment(m_db, m_ways[i], i);

		// insert endpoints into list
		m_endpoints.push_back(Endpoint { &ws, ws.nodeRefFirst(), true,  i});
		m_endpoints.push_back(Endpoint { &ws, ws.nodeRefLast(),  false, i});
	}

	matchEndpointsByNodeID();
	closeEndpointsToBoundary();
	closeAroundBoundary();
	buildLoops();
}


void MultipolyCloser::matchEndpointsByNodeID()
{
	m_endpoints.sort(Endpoint::NodeIDOrder());
	list<Endpoint>::iterator l=m_endpoints.begin(),m,u=m_endpoints.begin();

	while(l != m_endpoints.end())
	{
		// advance u until it is a different node ID
		unsigned d;
		for(d=1,u++; u != m_endpoints.end() && Endpoint::NodeIDEqual(*u,*l); ++u,++d)
			{}

		if (d > 2)
			cout << "Not sure what to do where >2 points match!" << endl;
		else if (d == 2)
		{
			m = l;
			m++;
			if (l->waySegment == m->waySegment)		// segment closes itself
			{
				l->waySegment->linkSelf();
				cout << "Way " << l->way << " closes itself" << endl;

				m_endpoints.erase(l,u);
			}
			else						// end of way l->way -> start of way m->way
			{
				// ordering choice: endpoints before startpoints (==> l is end || both are starts)
				assert(!l->isWayFirst || m->isWayFirst);

				if (!l->isWayFirst && m->isWayFirst)		// normal case: l is an end, m is a start
				{
					cout << " INFO: Connecting ways " << setw(3) << l->way << " and " << setw(3) << m->way << endl;
					l->waySegment->append(m->waySegment);
				}
				else if (l->isWayFirst == m->isWayFirst)	// abnormal: start-start or end-end connection
				{
					// l startpoint ==> m startpoint due to ordering choice (endpoints order before startpoints)

					cout << "  WARNING: Orientation change between ways " << setw(3) << l->way << " and " << setw(3) << m->way;
					if (l->isWayFirst)
						cout << " (start->start connection)" << endl;
					else
						cout << " (  end->end   connection)" << endl;

					// each must have space to link at either next or prev
					assert(!l->waySegment->hasNext() || !l->waySegment->hasPrev());
					assert(!m->waySegment->hasNext() || !m->waySegment->hasPrev());

					if (l->isWayFirst)
					{
						l->waySegment->reverse(true);
						l->waySegment->append(m->waySegment);
						cout << "         : Reversing way " << setw(3) << l->way << endl;
					}
					else
					{
						m->waySegment->reverse(true);
						l->waySegment->prepend(m->waySegment);
						cout << "         : Reversing way " << setw(3) << m->way << endl;
					}
				}
				else
					assert(false);	// impossible case

				m_endpoints.erase(l,u);
			}
		}
		else if (d == 1)
			cout << "Segment terminal " << l->way << " " << (l->isWayFirst ? "start":"end") << "  is lonely" << endl;

		l=u;			// u is past-the-end, so points to next endpoint
	}
}

void MultipolyCloser::closeEndpointsToBoundary()
{
	////// Map unresolved terminals (points where segments start/end at boundary)
	if(m_endpoints.size())
		cout << "Unresolved terminals with bounds " << m_bounds.first.lat << ',' << m_bounds.first.lon << "  " << m_bounds.second.lat << ',' << m_bounds.second.lon << ": " << endl;
	else
		cout << "All terminals resolved" << endl;

	for(auto it = m_endpoints.begin(); it != m_endpoints.end(); ++it)
	{
		assert(it->way != -1U);
		assert(m_ways[it->way]);

		LatLon p = it->pos();
		auto res = nearestPointOnEdge(p);

		// create new boundary point at edge
		BoundaryPointSegment bps(res.edge,res.p,res.edgeDistance,res.cwDistance,it->isWayFirst);

		LatLon pEdge = bps.first();

		cout << "  Segment " << setw(3) << it->way << " " << (it->isWayFirst ? "starts":"ends  ") << " at " << p.lat << ',' << p.lon << endl;
		cout << "    Boundary point " << pEdge.lat << "," << pEdge.lon << " (" << bps.m_edgeDistance << "m from " << bps.m_location << ")" << endl;

		if (bps.m_edgeDistance < m_maxBoundaryDistance)
		{
			auto newIt = m_boundpoints.insert(m_boundpoints.begin(),bps);
			if (it->isWayFirst)
				newIt->append(it->waySegment);
			else
				newIt->prepend(it->waySegment);

			m_endpoints.erase(it);
		}
		else
			cout << "WARNING: Distance too far to edge (maxBoundaryDistance=" << m_maxBoundaryDistance << ")" << endl;
	}
}

void MultipolyCloser::closeAroundBoundary()
{
	// sort boundary points in clockwise order
	m_boundpoints.sort(BoundaryPointSegment::OrientedOrder(m_dir));

	// insert corner points as needed
	int delta = m_dir == CW ? 2 : -2;
	int edgeToCorner = m_dir == CW ? 1 : -1;		// value to add to edge to get the corner after the edge
	CompassPrincipal currentCorner(CompassPrincipal::NW);

	list<BoundaryPointSegment>::iterator it=m_boundpoints.begin();

	// put the NW corner in
	m_boundpoints.insert(it,BoundaryPointSegment(currentCorner,corner(currentCorner),cornerCWDistance(currentCorner)));
	currentCorner=currentCorner+delta;

	while(it != m_boundpoints.end() || currentCorner != CompassPrincipal::NW)
	{
		if (it == m_boundpoints.end())
			cout << "Done with boundary points, currently at " << currentCorner << " corner" << endl;
		else
			cout << "Boundary point is on " << it->m_location << " edge, looking at " << currentCorner << " corner" << endl;

		for(; currentCorner != CompassPrincipal::NW && (it == m_boundpoints.end() || (it->m_location+edgeToCorner != currentCorner)); currentCorner=currentCorner+delta)
		{
			m_boundpoints.insert(it,BoundaryPointSegment(currentCorner,corner(currentCorner),cornerCWDistance(currentCorner)));
		}

		if (it != m_boundpoints.end())
		{
			++it;
			if (it != m_boundpoints.end())
				cout << "Moving to point on " << it->m_location << " edge" << endl;
		}
	}

	cout << "Boundary points: " << endl;
	for(const auto bp : m_boundpoints)
		cout << bp << endl;

	// advance to the first endpoint
	Segment* last=nullptr;
	for(it = m_boundpoints.begin(); it != m_boundpoints.end() && !it->isEnd(); ++it){}

	// make connections around edge
	for(; it != m_boundpoints.end(); ++it)
	{
		if (it->isStart())
		{
			if (last)					// hit start of segment -> prepend last segment to it
			{
				cout << "INFO: Prepending to " << *it << endl;
				if (!it->prepend(last))
					cout << "ERROR: Failed to prepend!" << endl;
				last=nullptr;
			}
			else
			{
				cout << "WARNING: Found a segment start but no curve currently being processed (incorrect orientation?)" << endl;
				cout << "       : " << *it << endl;
			}
		}
		else if (it->isEnd())
		{
			if (last)
			{
				cout << "WARNING: Found a segment end while traversing boundary, but there is already a segment being processed" << endl;
				cout << "       : " << *it << endl;
			}
			else
			{
				cout << "INFO: Picked up new endpoint " << *it << endl;
				last=&(*it);
			}
		}
		else if (last)
		{
			cout << "INFO: Traversing boundary point " << *it << " and appending" << endl;
			if (last->append(&(*it)))
				last = &(*it);
			else
				cout << "ERROR: Failed to add!" << endl;
		}
		else
			cout << "INFO: Skipping corner point " << *it << endl;
	}

	cout << "Continuing through north to get last startpoints" << endl;

	// finish off last few connections (must be startpoints or would have been)
	for(it = m_boundpoints.begin(); it != m_boundpoints.end() && !it->isEnd(); ++it)
	{
		if (it->isStart())
		{
			if (last)					// hit start of segment -> prepend last segment to it
			{
				cout << "INFO: Prepending to " << *it << endl;
				if (!it->prepend(last))
					cout << "ERROR: Failed to prepend!" << endl;
				last=nullptr;
			}
			else
			{
				cout << "WARNING: Found a segment start but no curve currently being processed (incorrect orientation?)" << endl;
				cout << "       : " << *it << endl;
			}
		}
		else if (last)
		{
			cout << "INFO: Traversing boundary point " << *it << " and appending" << endl;
			if (last->append(&(*it)))
				last = &(*it);
			else
				cout << "ERROR: Failed to add!" << endl;
		}
		else
			cout << "INFO: Skipping corner point " << *it << endl;
	}
}

void MultipolyCloser::buildLoops()
{
	m_loops.clear();
	m_wsAdded.clear();
	m_wsAdded.resize(m_waySegments.size(),false);

	// build loop entries serially
	cout << "Building loop indices" << endl;
	for(unsigned i=0;i<m_waySegments.size();++i)
	{
		if (m_wsAdded.at(i))
			continue;

		cout << "checking segment " << i << endl;

		if (m_waySegments[i].isMemberOfClosedLoop())
		{
			for(const Segment* s=&m_waySegments[i]; s != nullptr; s=s->nextEndingAt(&m_waySegments[i]))
			{
				if (const WaySegment* ws = dynamic_cast<const WaySegment*>(s))
				{
					if (m_wsAdded.at(ws->index()))
						cout << "WARNING: Way segment " << ws->index() << " multiple inclusion" << endl;
					m_wsAdded[ws->index()] = true;
				}
			}
			m_loops.push_back(&m_waySegments[i]);
		}
		else
			cout << "WARNING: Segment " << i << " is not part of a closed loop" << endl;
	}
}

vector<pair<vector<LatLon>,bool>> MultipolyCloser::loops(Boundedness b) const
{
#warning "Boundedness parameter in MultipolyCloser::loops currently ignored"

	cout << " Constructing return loops" << endl;

	vector<pair<vector<LatLon>,bool>> o;

	for(const Segment* l : m_loops)
	{
		o.emplace_back(vector<LatLon>(),true);
		for(const Segment* s=l; s != nullptr; s=s->nextEndingAt(l))
		{
			s->add(o.back().first);
			if (dynamic_cast<const BoundaryPointSegment*>(s))
				o.back().second=false;
		}
	}
	return o;
}


