/*
 * Feature.h
 *
 *  Created on: Dec 30, 2015
 *      Author: jcassidy
 */

#ifndef FEATURE_HPP_
#define FEATURE_HPP_

#include <vector>
#include <utility>
#include <string>

#include "LatLon.h"

#include "OSMEntityType.h"

enum FeatureType {
    Unknown = 0,
    Park,
    Beach,
    Lake,
    River,
    Island,
    Shoreline,
    Building,
    Greenspace,
    Golfcourse,
    Stream
};

const std::string& asString(FeatureType t);

class Feature {
public:

    Feature() {
    }

    Feature(OSMID id, OSMEntityType osmType, FeatureType type, std::string&& name, std::vector<LatLon>&& pts, bool bounded = true) :
    m_id(id),
    m_osmType(osmType),
    m_name(name),
    m_points(pts),
    m_type(type),
    m_bounded(bounded) {
    }

    unsigned pointCount() const {
        return m_points.size();
    }

    LatLon point(unsigned i) const {
        return m_points.at(i);
    }

    const std::vector<LatLon>& points() const {
        return m_points;
    }

    std::pair<OSMID, OSMEntityType> id() const {
        return std::make_pair(m_id, m_osmType);
    }

    FeatureType type() const {
        return m_type;
    }

    bool bounded() const {
        return m_bounded;
    } // true if does not go out of bounds

    bool isWater() const {
        return m_type == Lake || m_type == River;
    }

    const std::string& name() const {
        return m_name;
    }

private:
    OSMID m_id = -1ULL;
    OSMEntityType m_osmType = Invalid;
    std::string m_name = "";
    std::vector<LatLon> m_points;
    FeatureType m_type;

    bool m_bounded = true;

    template<class Archive>void serialize(Archive& ar, const unsigned ver) {
        ar & m_id & m_name & m_points & m_type & m_osmType;
    }

    friend class boost::serialization::access;
};




#endif /* FEATURE_HPP_ */
