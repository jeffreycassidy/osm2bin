/*
 * BasicFeatureFilter.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: jcassidy
 */

#ifndef BASICWAYFEATUREFACTORY_HPP_
#define BASICWAYFEATUREFACTORY_HPP_

#include "OSMWay.hpp"

#include "Feature.hpp"
#include <boost/optional/optional.hpp>

class BasicWayFeatureFactory
{
public:
	BasicWayFeatureFactory(const OSMDatabase& db);

	// returns a valid value if the specified way generates a feature, else empty
	boost::optional<Feature> operator()(const OSMWay& w) const;

private:
	const OSMDatabase& m_db;
	const std::string& name(const OSMWay& w) const;

	static const std::string s_noname;

	// key indices
	unsigned m_kiNatural=-1U;
	unsigned m_kiHighway=-1U;
	unsigned m_kiWaterway=-1U;
	unsigned m_kiBuilding=-1U;
	unsigned m_kiAmenity=-1U;
	unsigned m_kiTourism=-1U;
	unsigned m_kiLeisure=-1U;
	unsigned m_kiPlace=-1U;
	unsigned m_kiWater=-1U;

	unsigned m_kiName=-1U;
	unsigned m_kiNameEn=-1U;

	// value indices
	//unsigned m_viRiver=-1U;
	unsigned m_viLake=-1U;				// water=
	unsigned m_viPond=-1U;
	unsigned m_viReservoir=-1U;

	unsigned m_viPark=-1U;				// leisure=
	unsigned m_viNatureReserve=-1U;

	unsigned m_viStream=-1U;			// waterway=
	unsigned m_viRiver=-1U;
	unsigned m_viRiverbank=-1U;

	unsigned m_viWetland=-1U;			// natural=
	unsigned m_viWood=-1U;
	unsigned m_viWater=-1U;
	unsigned m_viScrub=-1U;
	unsigned m_viBeach=-1U;

	unsigned m_viIsland=-1U;			// place=
	unsigned m_viGolfCourse=-1U;
};


#endif /* BASICWAYFEATUREFACTORY_HPP_ */
