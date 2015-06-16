/*
 * BasicFeatureFilter.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: jcassidy
 */

#ifndef BASICRELATIONFEATUREFACTORY_HPP_
#define BASICRELATIONFEATUREFACTORY_HPP_

#include "OSMRelation.hpp"

#include "Feature.hpp"
#include <boost/optional/optional.hpp>

#include "FeatureFactory.hpp"

#include <utility>



class BasicRelationFeatureFactory : public FeatureFactory
{
public:
	BasicRelationFeatureFactory(const OSMDatabase& db);

	// returns a vector of features generated from the relation
	std::vector<Feature> operator()(const OSMRelation& r) const;

private:
	unsigned m_kiWater=-1U;
		unsigned m_viLake=-1U;
		unsigned m_viRiver=-1U;

	unsigned m_kiWaterway=-1U;

	unsigned m_kiType;
		unsigned m_viMultipolygon=-1U;

	unsigned m_kiNatural=-1U;
	unsigned m_viCoastline=-1U;

	unsigned m_relroleInner=-1U;
	unsigned m_relroleOuter=-1U;
	unsigned m_relroleFrom=-1U;
	unsigned m_relroleTo=-1U;
	unsigned m_relroleVia=-1U;
	unsigned m_relroleForward=-1U;
	unsigned m_relroleBackward=-1U;
	unsigned m_relroleBlank=-1U;
	unsigned m_relroleMainstream=-1U;
};

#endif /* BASICRELATIONFEATUREFACTORY_HPP_ */
