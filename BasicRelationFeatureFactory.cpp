/*
 * BasicRelationFeatureFactory.cpp
 *
 *  Created on: Dec 30, 2015
 *      Author: jcassidy
 */

#include "OSMDatabase.hpp"

#include "Feature.h"


#include <list>
#include <stack>
#include <iostream>
#include <utility>
#include "BasicRelationFeatureFactory.hpp"

#include "Compass.hpp"

#include "OSMNode.hpp"

#include "MultipolyCloser.hpp"

#include <boost/math/constants/constants.hpp>

using namespace std;

BasicRelationFeatureFactory::BasicRelationFeatureFactory(const OSMDatabase& db) : FeatureFactory(db,db.relationTags()),
		m_kiWater(db.relationTags().getIndexForKeyString("water")),
		m_viLake(db.relationTags().getIndexForValueString("lake")),
		m_viRiver(db.relationTags().getIndexForValueString("river")),

		m_kiWaterway(db.relationTags().getIndexForValueString("waterway")),

		m_kiNatural(db.relationTags().getIndexForKeyString("natural")),
		m_viCoastline(db.relationTags().getIndexForValueString("coastline")),

		m_relroleInner(db.relationRoles().getIndexOfValue("inner")),
		m_relroleOuter(db.relationRoles().getIndexOfValue("outer")),
		m_relroleFrom(db.relationRoles().getIndexOfValue("from")),
		m_relroleTo(db.relationRoles().getIndexOfValue("to")),
		m_relroleVia(db.relationRoles().getIndexOfValue("via")),
		m_relroleForward(db.relationRoles().getIndexOfValue("forward")),
		m_relroleBackward(db.relationRoles().getIndexOfValue("backward")),
		m_relroleBlank(db.relationRoles().getIndexOfValue("")),
		m_relroleMainstream(db.relationRoles().getIndexOfValue("main_stream"))
{
}

vector<Feature> BasicRelationFeatureFactory::operator()(const OSMRelation& r) const
{
	std::vector<Feature> features;

	// skip anything with no tags (no name/type)
	if (r.tags().size() < 1)
		return features;

	string n = name(&r);

	if (r.hasTagWithValue(m_kiWater,m_viLake) || r.hasTagWithValue(m_kiNatural,m_viCoastline) || r.hasTagWithValue(m_kiWater,m_viRiver) || r.hasTagWithValue(m_kiWaterway,m_viRiver))
	{
		if (r.hasTagWithValue(m_kiWater,m_viLake))
			cout << "Found a lake";
		else if (r.hasTagWithValue(m_kiNatural,m_viCoastline))
			cout << "Found a coastline";
		else if (r.hasTagWithValue(m_kiWaterway,m_viRiver))
			cout << "Found a waterway=river";

		cout << " relation ID " << r.id() <<" with name'" << n << "'" << endl;

		vector<pair<unsigned,const OSMWay*>> waysByRole;

		// loop over all way members
		for(const auto m : r.members() | boost::adaptors::filtered([](OSMRelation::Member m){ return m.type == OSMRelation::Way; }))
		{
			if (m.role == m_relroleInner || m.role == m_relroleOuter || m.role == m_relroleBlank || m.role == m_relroleMainstream)
			{
				cout << "  Member way ID " << m.id << " is an ";
				if(m.role == m_relroleInner)
					cout << " inner poly" << endl;
				else if (m.role == m_relroleOuter)
					cout << " outer poly" << endl;
				else if (m.role == m_relroleBlank)
					cout << " unspecified role" << endl;
				else if (m.role == m_relroleMainstream)
					cout << " main_stream" << endl;

				if (const OSMWay* w = m_db.wayPtrFromID(m.id))
					waysByRole.push_back(make_pair(m.role,w));
				else
					cout << "    Skipping due to dangling way reference (ID" << m.id << ")" << endl;
			}
			else if (m.role == m_relroleForward || m.role == m_relroleBackward || m.role == m_relroleFrom || m.role == m_relroleTo || m.role == m_relroleVia)
				cout << "    Ignoring member way ID " << m.id << " with role " << m_db.relationRoles().getValue(m.role) << endl;
			else
				cout << "    Ignoring member way ID " << m.id << " with unexpected role " << m_db.relationRoles().getValue(m.role) << endl;
		}

		vector<const OSMWay*> v;

		boost::copy(
				waysByRole
					| boost::adaptors::filtered([this](pair<unsigned,const OSMWay*> p){ return m_relroleOuter==p.first; })
					| boost::adaptors::map_values,
				std::back_inserter(v));

		cout << "  Relation holds " << v.size() << " outer way references for poly closing" << endl;

		{
			MultipolyCloser C(m_db,v);
			C.direction(MultipolyCloser::CW);

			for(auto& f : C.loops(MultipolyCloser::All))
				features.emplace_back(r.id(),Relation,Lake,string(name(&r)),std::move(f.first),f.second);
		}


		v.clear();
		boost::copy(
				waysByRole
					| boost::adaptors::filtered([this](pair<unsigned,const OSMWay*> p){ return m_relroleInner==p.first; })
					| boost::adaptors::map_values,
				std::back_inserter(v));

		cout << "  Relation holds " << v.size() << " inner way references for poly closing" << endl;

		{
			MultipolyCloser C(m_db,v);
			C.direction(MultipolyCloser::CCW);

			for(auto& f : C.loops(MultipolyCloser::All))
			{
				features.emplace_back(r.id(),Relation,Island,string(name(&r)),std::move(f.first));
				if (!f.second)
					cout << "  WARNING: Unbounded inner way (in relation " << r.id() << ") is a bit unusual" << endl;
			}
		}
	}
	else
	{
	//	cout << "Skipping relation ID " << r.id() << " with name '" << n << '\'' << endl;
	}

	return features;
}





