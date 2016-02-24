#pragma once
#include "OSMEntity.hpp"

#include <array>
#include <utility>
#include <cmath>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include "LatLon.h"

#include <limits>

class OSMNode : public OSMEntity {
public:
    OSMNode(unsigned long long id_ = 0, float lat = std::numeric_limits<float>::quiet_NaN(), float lon = std::numeric_limits<float>::quiet_NaN()) : OSMEntity(id_), coords_{lat, lon}
    {
    }
    OSMNode(OSMNode&&) = default;
    OSMNode(const OSMNode&) = default;
    OSMNode& operator=(OSMNode&&) = default;

    void coords(LatLon pos) {
        coords_ = pos;
    }

    LatLon coords() const {
        return coords_;
    }

    LatLon& coords() {
        return coords_;
    }

    friend std::ostream& operator<<(std::ostream&, const OSMNode&);

private:
    LatLon coords_;

    template<class Archive>void serialize(Archive& ar, const unsigned) {
        ar & boost::serialization::base_object<OSMEntity>(*this) & coords_;
    }
    friend boost::serialization::access;
};

std::ostream& operator<<(std::ostream&, const OSMNode&);
