/*
 * PathNetwork.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: jcassidy
 */

#include "PathNetwork.hpp"

#include "OSMDatabase.hpp"

#include <boost/container/flat_map.hpp>

using namespace std;

const char* OSMWayFilterRoads::includeHighwayTagValueStrings_[] = {
			"residential",
			"primary",
			"primary_link",
			"secondary",
			"secondary_link",
			"tertiary",
			"tertiary_link",
			"motorway",
			"motorway_link",
			"service",
			nullptr
	};


PathNetwork buildNetwork(const OSMDatabase& db,const OSMEntityFilter<OSMWay>& wayFilter)
{
	PathNetwork G;

	float defaultMax=50.0f;

	struct NodeWithRefCount
	{
		PathNetwork::vertex_descriptor 	graphVertexDescriptor=-1U;
		unsigned 						nodeVectorIndex=-1U;
		unsigned 						refcount=0;
	};

	std::unordered_map<unsigned long long,NodeWithRefCount> nodesByOSMID;



	// create OSM ID -> vector index mapping for all nodes
	std::cout << "Computing index map" << std::endl;

	for(unsigned i=0;i<db.nodes().size(); ++i)
		nodesByOSMID[db.nodes()[i].id()].nodeVectorIndex = i;



	// traverse the listed ways, marking node reference counts
	std::cout << "Computing reference counts" << std::endl;
	for(const OSMWay& w : db.ways() | boost::adaptors::filtered(std::cref(wayFilter)))
		for(const unsigned long long nd : w.ndrefs())
		{
			auto it = nodesByOSMID.find(nd);
			if (it == nodesByOSMID.end())
				std::cerr << "WARNING: Dangling reference to node " << nd << " in way " << w.id() << std::endl;
			else
				it->second.refcount++;
		}



	// calculate and print reference-count (# way refs for each node) histogram for fun
	std::vector<unsigned> refhist;
	std::cout << "Computing reference-count frequency histogram" << std::endl;
	for(const auto ni : nodesByOSMID | boost::adaptors::map_values)
	{
		if (ni.refcount >= refhist.size())
			refhist.resize(ni.refcount+1,0);
		refhist[ni.refcount]++;
	}

	for(unsigned i=0;i<refhist.size();++i)
		if (refhist[i] > 0)
			std::cout << "  " << std::setw(3) << i << " refs: " << refhist[i] << " nodes" << std::endl;




	// iterate over ways, creating edges in the intersection graph with associated curvepoints)

	size_t nCurvePoints=0;

	boost::container::flat_map<unsigned,float> speedValues;

	unsigned k_maxspeed=db.wayTags().getIndexForKeyString("maxspeed");

	enum OneWayType { Bidir,Forward,Backward,Reversible,Unknown };

	unsigned k_oneway=db.wayTags().getIndexForKeyString("oneway");
		unsigned v_oneway_yes=db.wayTags().getIndexForValueString("yes");
		unsigned v_oneway_one=db.wayTags().getIndexForValueString("1");
		unsigned v_oneway_minus_one=db.wayTags().getIndexForValueString("-1");
		unsigned v_oneway_reversible=db.wayTags().getIndexForValueString("reversible");
		unsigned v_oneway_no=db.wayTags().getIndexForValueString("no");

	unsigned nOnewayForward=0, nOnewayBackward=0,nOnewayReversible=0,nBidir=0,nUnknown=0;
	unsigned nWays=0;

	for(const auto& w : db.ways() | boost::adaptors::filtered(std::cref(wayFilter)))
	{
		++nWays;
		PathNetwork::vertex_descriptor segmentStartVertex = -1ULL, segmentEndVertex=-1ULL;
		vector<LatLon> curvePoints;

		// max speed is per way; mark as NaN by default (change NaNs to other value later)
		float speed=std::numeric_limits<float>::quiet_NaN();

		unsigned v;

		if ((v=w.getValueForKey(k_maxspeed)) != -1U)		// has maxspeed tag
		{
			boost::container::flat_map<unsigned,float>::iterator it;
			bool inserted;

			tie(it,inserted) = speedValues.insert(make_pair(v,std::numeric_limits<float>::quiet_NaN()));

			if (inserted)		// need to map this string value to a number
			{
				float spd;
				string str = db.wayTags().getValue(v);
				stringstream ss(str);
				ss >> spd;
				if (ss.fail())
				{
					ss.clear();
					cout << "truncating '" << str << "' to substring '";
					str = str.substr(str.find_first_of(':')+1);
					cout << str << "'" << endl;
					ss.str(str);

					ss >> spd;
				}

				if (ss.fail())
					std::cout << "Failed to convert value for maxspeed='" << str << "'" << endl;
				else if (!ss.eof())
				{
					string rem;
					ss >> rem;
					if (rem == "kph")
						it->second = spd;
					else if (rem == "mph")
						it->second = spd*1.609f;
					else
					{
						std::cout << "Warning: trailing characters in maxspeed='" << str << "'" << endl;
						std::cout << "  remaining: '" << rem << "'" << endl;
					}
				}
				else
					it->second = spd;

				if (!isnan(it->second))
					std::cout << "Added new speed '" << str << "' = " << it->second << endl;
				else
					std::cout << "Failed to convert speed '" << str << "'" << endl;
			}
			speed = it->second;
		}

		// assume bidirectional if not specified

		OneWayType oneway = Unknown;

		if (k_oneway == -1U)
			{ oneway=Bidir; ++nBidir; }
		else
		{
			unsigned v_oneway=w.getValueForKey(k_oneway);

			if (v_oneway == -1U || v_oneway==v_oneway_no)
				{ oneway=Bidir; ++nBidir; }
			else if (v_oneway == v_oneway_one)
				{ oneway=Forward; ++nOnewayForward; }
			else if (v_oneway == v_oneway_yes)
				{ oneway=Forward; ++nOnewayForward; }
			else if (v_oneway == v_oneway_minus_one)
				{ oneway=Backward; ++nOnewayBackward; }
			else if (v_oneway == v_oneway_reversible)
				{ oneway=Reversible; ++nOnewayReversible; }
			else
			{
				cout << "Unrecognized tag oneway='" << db.wayTags().getValue(v_oneway);
				++nUnknown;
			}
		}


		// hamilton_canada and newyork: k=oneway v=-1 | 1 | yes | no | yes;-1 | reversible


		for(unsigned i=0;i<w.ndrefs().size();++i)
		{
			const auto currNodeIt = nodesByOSMID.find(w.ndrefs()[i]);

			LatLon prevCurvePoint;

			if (currNodeIt == nodesByOSMID.end())			// dangling node reference: discard
			{
				cout << "WARNING: Dangling node with OSM ID " << w.ndrefs()[i] << " on way with OSM ID " << w.id() << endl;
				segmentEndVertex = -1U;
				continue;
			}
			else if (currNodeIt->second.refcount == 1)		// referenced by only 1 way -> curve point
			{
				LatLon ll = db.nodes().at(currNodeIt->second.nodeVectorIndex).coords();
				curvePoints.push_back(ll);
			}
			else if (currNodeIt->second.refcount > 1 || i == 0 || i==w.ndrefs().size()-1)
				// referenced by >1 way or first/last node ref in a way -> node is an intersection
			{
				segmentEndVertex = currNodeIt->second.graphVertexDescriptor;

				// if vertex hasn't been added yet, add now
				if (segmentEndVertex == -1U)
				{
					currNodeIt->second.graphVertexDescriptor = segmentEndVertex = add_vertex(G);
					G[segmentEndVertex].latlon = db.nodes().at(currNodeIt->second.nodeVectorIndex).coords();
					G[segmentEndVertex].osmid = currNodeIt->first;
				}

				// arriving at an intersection node via a way
				if (segmentStartVertex != -1ULL)
				{
					PathNetwork::edge_descriptor e;
					bool inserted;
					assert(segmentStartVertex < num_vertices(G));
					assert(segmentEndVertex < num_vertices(G));


					std::tie(e,inserted) = add_edge(segmentStartVertex,segmentEndVertex,G);

					assert(inserted && "Duplicate edge");
					G[e].wayOSMID=w.id();

					if (oneway == Forward || oneway == Backward)
					{
						// goes towards end vertex; if greater then goes towards greater
						// flip it
						bool towardsGreater = (segmentEndVertex > segmentStartVertex) ^ (oneway==Backward);

						G[e].oneWay = towardsGreater ?
								EdgeProperties::ToGreaterVertexNumber :
								EdgeProperties::ToLesserVertexNumber;

					}
					else
						G[e].oneWay = EdgeProperties::Bidir;

					G[e].maxspeed=speed;

					nCurvePoints += curvePoints.size();

					G[e].curvePoints = std::move(curvePoints);
				}

				segmentStartVertex = segmentEndVertex;
			}

			prevCurvePoint = db.nodes().at(currNodeIt->second.nodeVectorIndex).coords();
		}
	}

	std::cout << "PathNetwork created with " << num_vertices(G) << " vertices and " << num_edges(G) << " edges, with " << nCurvePoints <<  " curve points" << std::endl;
	std::cout << "  Used " << nWays << "/" << db.ways().size() << " ways" << endl;
	std::cout << "  One-way streets: " << (nOnewayForward+nOnewayBackward) << "/" << nWays << " (" << nOnewayReversible << " reversible and " << nUnknown << " unknown)" << std::endl;
	assert(nOnewayForward+nOnewayBackward+nOnewayReversible+nBidir+nUnknown == nWays);

	// create/print edge-degree histogram
	std::cout << "Vertex degree histogram:" << std::endl;
	std::vector<unsigned> dhist;

	for(auto v : boost::make_iterator_range(vertices(G)))
	{
		unsigned d = out_degree(v,G);
		if (d >= dhist.size())
			dhist.resize(d+1,0);
		dhist[d]++;
	}

	unsigned Ndefault=0;
	for (auto e : boost::make_iterator_range(edges(G)))
	{
		if (isnan(G[e].maxspeed))
		{
			++Ndefault;
			G[e].maxspeed=defaultMax;
		}
	}

	cout << "Assigned " << Ndefault << "/" << num_edges(G) << " street segments to default max speed for lack of information" << endl;

	for(unsigned i=0;i<dhist.size();++i)
		if (dhist[i] > 0)
			std::cout << "  " << std::setw(3) << i << " refs: " << dhist[i] << " nodes" << std::endl;


	return G;
}





class StreetEdgeVisitor
{
public:

	StreetEdgeVisitor(PathNetwork* G)
		: m_G(G)
	{
	}

	std::vector<std::string> assign()
	{
		m_streets.clear();
		m_streets.push_back("<unknown>");

		for(const auto e : edges(*m_G))
			if ((*m_G)[e].streetVectorIndex==0)
				startWithEdge(e);

		return std::move(m_streets);
	}

	// pick up the street name from the specified edge, add it to the vertex, and expand the source/target vertices
	void startWithEdge(PathNetwork::edge_descriptor e);

	// inspect all out-edges of the vertex, assigning and recursively expanding any which match the way ID or street name
	void expandVertex(PathNetwork::vertex_descriptor v,OSMID wayID);

	void osmDatabase(OSMDatabase* p)
	{
		m_osmDatabase=p;
		m_keyIndexName=p->wayTags().getIndexForKeyString("name");
		m_keyIndexNameEn=p->wayTags().getIndexForKeyString("name:en");
	}

private:
	const std::string streetNameForWay(OSMID osmid)
	{
		const OSMWay& w = m_osmDatabase->wayFromID(osmid);
		unsigned valTag = -1U;

		if ((valTag = w.getValueForKey(m_keyIndexNameEn)) == -1U)
			valTag = w.getValueForKey(m_keyIndexName);

		if (valTag == -1U)
			return std::string();
		else
			return m_osmDatabase->wayTags().getValue(valTag);
	}

	static const std::string np;


	PathNetwork*					m_G=nullptr;				// the path network

	OSMDatabase*					m_osmDatabase=nullptr;		// OSM database with source data

	unsigned 						m_keyIndexName=-1U,m_keyIndexNameEn=-1U;		// cached values for key-value tables

	std::vector<std::string>		m_streets;					// street name vector being built
	unsigned						m_currStreetVectorIndex=0;	// index of current street
};

const std::string StreetEdgeVisitor::np={"<name-not-present>"};

void StreetEdgeVisitor::startWithEdge(PathNetwork::edge_descriptor e)
{
	// grab OSM ID of this way, look up its name
	OSMID wID = (*m_G)[e].wayOSMID;

	std::string streetName = streetNameForWay(wID);

	if (!streetName.empty())
	{
		// insert the street name
		(*m_G)[e].streetVectorIndex = m_currStreetVectorIndex = m_streets.size();
		m_streets.push_back(streetName);

		// expand the vertices at both ends
		expandVertex(source(e,*m_G),wID);
		expandVertex(target(e,*m_G),wID);
	}
}

/** Recursively expand the specified vertex, along out-edges which have way OSMID == currWayID, or have the same street name */

void StreetEdgeVisitor::expandVertex(PathNetwork::vertex_descriptor v,OSMID currWayID)
{
	for(const auto e : out_edges(v,*m_G))			// for all out-edges of this vertex
	{
		if ((*m_G)[e].streetVectorIndex != 0)			// skip if already assigned (prevents infinite recursion back to origin)
			continue;

		OSMID wID = (*m_G)[e].wayOSMID;

		assert (wID != 0 && wID != -1U);

		if (wID == currWayID)											// (connected,same way -> same name) -> same street
			(*m_G)[e].streetVectorIndex = m_currStreetVectorIndex;
		else if (streetNameForWay(wID) == m_streets.back())				// (connected,same name) -> same street
			(*m_G)[e].streetVectorIndex = m_currStreetVectorIndex;
		else															// different way, different name -> done expanding
			continue;

		// this is the fall-through case for the if/elseif above (terminates if reaches a different street)
		expandVertex(target(e,*m_G), wID);
	}
}

std::vector<std::string> assignStreets(OSMDatabase* db,PathNetwork& G)
{
	StreetEdgeVisitor sev(&G);
	sev.osmDatabase(db);

	// blank out existing street names
	for(const auto e : edges(G))
		G[e].streetVectorIndex=0;

	return sev.assign();
}
