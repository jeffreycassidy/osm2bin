/*
 * NodePOIFilter.hpp
 *
 *  Created on: Dec 23, 2015
 *      Author: jcassidy
 */

#ifndef NODEPOIFILTER_HPP_
#define NODEPOIFILTER_HPP_

#include "POI.hpp"
#include "OSMNode.hpp"
#include "KeyValueTable.hpp"

#include <vector>
#include <string>

#include <boost/optional/optional.hpp>
#include <boost/container/flat_set.hpp>

// POI tags extracted for 2014:
// convenience_store, railway, amenity, microbrewery, beverages, building, cafe, social_facility, cinema

class NodePOIFilter
{
public:
	NodePOIFilter(const KeyValueTable& kvt) : m_keyValueTable(kvt)
	{
		m_tagKeyIndex_name = m_keyValueTable.getIndexForKeyString("name");
		m_tagKeyIndex_name_en = m_keyValueTable.getIndexForKeyString("name:en");
		m_tagKeyIndex_amenity = m_keyValueTable.getIndexForKeyString("amenity");

	}

	boost::optional<POI> operator()(const OSMNode& n) const
	{
		std::string type;
		std::string name;

		if (n.hasTag(m_tagKeyIndex_name_en))
			name = m_keyValueTable.getValue(n.getValueForKey(m_tagKeyIndex_name_en));
		else if (n.hasTag(m_tagKeyIndex_name))
			name = m_keyValueTable.getValue(n.getValueForKey(m_tagKeyIndex_name));


		// skip if no name provided
		if (name.empty())
			return boost::optional<POI>();


		unsigned amenityIdx;
		if ((amenityIdx= n.getValueForKey(m_tagKeyIndex_amenity)) != -1U)
		{
			type = m_keyValueTable.getValue(amenityIdx);

		}
		else if (false)
		{

		}
		else
			return boost::optional<POI>();


		// print name, type, and tags
		//std::cout << "POI: name='" << name << "' type='" << type << "'" << std::endl;

//		for(const auto kv : n.tags())
//			std::cout << "    " << m_keyValueTable.getKey(kv.first) << ": " << m_keyValueTable.getValue(kv.second) << std::endl;

		POI poi(n.id(),n.coords(),type,name);
		return poi;
	}

private:
	const KeyValueTable& m_keyValueTable;
	unsigned m_tagKeyIndex_amenity			= -1U;
	unsigned m_tagKeyIndex_name_en			= -1U;
	unsigned m_tagKeyIndex_name				= -1U;
};
#endif /* NODEPOIFILTER_HPP_ */
