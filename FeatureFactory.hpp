/*
 * FeatureFactory.hpp
 *
 *  Created on: Jan 5, 2016
 *      Author: jcassidy
 */

#ifndef FEATUREFACTORY_HPP_
#define FEATUREFACTORY_HPP_

#include <string>

class OSMDatabase;
class OSMEntity;

#include "KeyValueTable.hpp"

class FeatureFactory
{
public:

	FeatureFactory(const OSMDatabase& db,const KeyValueTable& kvtTags) :
		m_db(db),
		m_kvtTags(kvtTags),
		m_kiName(kvtTags.getIndexForKeyString("name")),
		m_kiNameEn(kvtTags.getIndexForKeyString("name:en"))
	{
	}

protected:
	const OSMDatabase& m_db;

	const std::string& name(const OSMEntity* e) const;

private:
	const KeyValueTable& m_kvtTags;				// key-value table for the tags
	unsigned m_kiName=-1U;							// key index for name
	unsigned m_kiNameEn=-1U;						// key index for name:en

	static const std::string s_noname;
};



#endif /* FEATUREFACTORY_HPP_ */
