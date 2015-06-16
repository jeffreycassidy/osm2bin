/*
 * FeatureFactory.cpp
 *
 *  Created on: Jan 5, 2016
 *      Author: jcassidy
 */

#include "FeatureFactory.hpp"

#include "OSMEntity.hpp"
#include "OSMDatabase.hpp"

const std::string FeatureFactory::s_noname="<noname>";

const std::string& FeatureFactory::name(const OSMEntity* e) const
{
	unsigned val=-1U;
	if ((val = e->getValueForKey(m_kiNameEn)) != -1U)
		{}
	else if ((val = e->getValueForKey(m_kiName)) != -1U)
		{}

	return val == -1U ? s_noname : m_kvtTags.getValue(val);
}
