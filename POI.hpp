/*
 * POI.hpp
 *
 *  Created on: Dec 23, 2015
 *      Author: jcassidy
 */

#ifndef POI_HPP_
#define POI_HPP_

#include <string>
#include "OSMNode.hpp"
#include "LatLon.h"

class POI {
public:

    POI() {
    }

    POI(OSMID osmid, LatLon ll, std::string type, std::string name) :
    m_osmNodeID(osmid),
    m_pos(ll),
    m_name(name),
    m_type(type) {
    }

    const std::string& name() const {
        return m_name;
    }

    const std::string& type() const {
        return m_type;
    }

    OSMID osmNodeID() const {
        return m_osmNodeID;
    }

    LatLon pos() const {
        return m_pos;
    }

private:
    OSMID m_osmNodeID = -1U;
    LatLon m_pos;
    std::string m_name;
    std::string m_type;

    friend boost::serialization::access;

    template<class Archive>void serialize(Archive& ar, const unsigned ver) {
        ar & m_osmNodeID & m_pos & m_name & m_type;
    }
};




#endif /* POI_HPP_ */
