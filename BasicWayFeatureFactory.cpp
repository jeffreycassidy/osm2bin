/*
 * BasicWayFeatureFactory.cpp
 *
 *  Created on: Dec 30, 2015
 *      Author: jcassidy
 */

#include "OSMDatabase.hpp"

#include "Feature.hpp"

#include <iostream>
#include <utility>
#include "BasicWayFeatureFactory.hpp"

using namespace std;

BasicWayFeatureFactory::BasicWayFeatureFactory(const OSMDatabase& db): m_db(db),
		m_kiNatural(db.wayTags().getIndexForKeyString("natural")),
		m_kiHighway(db.wayTags().getIndexForKeyString("highway")),
		m_kiWaterway(db.wayTags().getIndexForKeyString("waterway")),
		m_kiBuilding(db.wayTags().getIndexForKeyString("building")),
		m_kiAmenity(db.wayTags().getIndexForKeyString("amenity")),
		m_kiTourism(db.wayTags().getIndexForKeyString("tourism")),
		m_kiLeisure(db.wayTags().getIndexForKeyString("leisure")),
		m_kiPlace(db.wayTags().getIndexForKeyString("place")),
		m_kiName(db.wayTags().getIndexForKeyString("name")),
		m_kiNameEn(db.wayTags().getIndexForKeyString("name:en")),
		m_kiWater(db.wayTags().getIndexForKeyString("water")),

		m_viLake(db.wayTags().getIndexForKeyString("lake")),
		m_viPond(db.wayTags().getIndexForKeyString("pond")),
		m_viReservoir(db.wayTags().getIndexForKeyString("reservoir")),

		m_viPark(db.wayTags().getIndexForValueString("park")),
		m_viNatureReserve(db.wayTags().getIndexForValueString("nature_reserve")),

		m_viStream(db.wayTags().getIndexForValueString("stream")),
		m_viRiver(db.wayTags().getIndexForValueString("river")),
		m_viRiverbank(db.wayTags().getIndexForValueString("riverbank")),

		m_viWetland(db.wayTags().getIndexForValueString("wetland")),
		m_viWood(db.wayTags().getIndexForValueString("wood")),
		m_viWater(db.wayTags().getIndexForValueString("water")),
		m_viScrub(db.wayTags().getIndexForValueString("scrub")),
		m_viBeach(db.wayTags().getIndexForValueString("beach")),
		m_viIsland(db.wayTags().getIndexForValueString("island"))
{
}

const std::string BasicWayFeatureFactory::s_noname("<noname>");

const std::string& BasicWayFeatureFactory::name(const OSMWay& w) const
{
	unsigned val=-1U;
	if ((val = w.getValueForKey(m_kiNameEn)) != -1U)
		{}
	else if ((val = w.getValueForKey(m_kiName)) != -1U)
		{}

	return val == -1U ? s_noname : m_db.wayTags().getValue(val);
}

boost::optional<Feature> BasicWayFeatureFactory::operator()(const OSMWay& w) const
{
	unsigned val;

	// skip anything with no tags (no name/type)
	if (w.tags().size() < 1)
		return boost::optional<Feature>();

	FeatureType type=Unknown;

	string n = name(w);

	if ((val = w.getValueForKey(m_kiHighway)) != -1U)
	{
		//cout << "Skipping way named '" << n << "' because it has highway tag" << endl;
	}
	else if ((val = w.getValueForKey(m_kiBuilding)) != -1U)
	{
		type = Building;
		//cout << "Adding way named '" << n << "' with tag building=" << m_db.wayTags().getValue(val) << endl;
	}
	else if ((val = w.getValueForKey(m_kiNatural)) != -1U)
	{
		if (val == m_viWood ||
				val == m_viWetland ||
				val == m_viScrub)
			type = Greenspace;
		else if (val == m_viWater)
			type = Lake;
		else if (val == m_viBeach)
			type = Beach;

//		if (type != Unknown)
//			cout << "Adding way named '" << n << "' with tag natural=" << m_db.wayTags().getValue(val) << endl;
	}
	else if ((val = w.getValueForKey(m_kiTourism)) != -1U)
	{
		//cout << "Skipping way named '" << n << "' with tag tourism=" << m_db.wayTags().getValue(val) << endl;
	}
	else if ((val = w.getValueForKey(m_kiAmenity)) != -1U)
	{
		//cout << "Skipping way named '" << n << "' with tag amenity=" << m_db.wayTags().getValue(val) << endl;
	}
	else if ((val = w.getValueForKey(m_kiWater)) != -1U)
	{
		if (val == m_viRiver)
			type = River;
		else if (val == m_viLake || val == m_viPond || val == m_viReservoir)
			type = Lake;
		else if (w.isClosed())
			type = River;
		else
			type = Stream;
	}
	else if ((val = w.getValueForKey(m_kiWaterway)) != -1U)
	{
		if (w.isClosed())
			type = River;
		else
			type = Stream;
//		if (val == m_viStream ||
//				val == m_viRiver ||
//				val == m_viRiverbank)
//			type = R;
			//cout << "Skipping way named '" << n << "' with tag waterway=" << m_db.wayTags().getValue(val) << endl;
	}
	else if ((val = w.getValueForKey(m_kiLeisure)) != -1U)
	{
		if (val==m_viPark ||
				val==m_viNatureReserve)
			type = Park;
		else
		{
		//	cout << "Skipping way named '" << n << "' with tag leisure=" << m_db.wayTags().getValue(val) << endl;
		}
	}
	else if ((val = w.getValueForKey(m_kiPlace)) != -1U)
	{
		if (val == m_viIsland)
			type = Island;
		else
		{
			//cout << "Skipping way named '" << n << "' with tag place=" << m_db.wayTags().getValue(val) << endl;
		}

	}
	else
	{
//		cout << "Skipping way named '" << name(w) << "' with tags: ";
//		for(const auto p : w.tags())
//			cout << m_db.wayTags().getKey(p.first) << ' ';
//		cout << endl;
	}

	if (type == Unknown)
		return boost::optional<Feature>();

	std::vector<LatLon> pts = m_db.extractPoly(w);

	Feature f(w.id(),OSMEntityType::Way,type,std::move(n),std::move(pts));

//	cout << "Created feature with OSMID=" << f.id().first << " name='" << f.name() << "' and points:" << endl;
//	for(unsigned i=0;i<f.pointCount();++i)
//	{
//		LatLon ll = f.point(i);
//		cout << "  " << ll.lat << ' ' << ll.lon << endl;
//	}

	return f;
}

