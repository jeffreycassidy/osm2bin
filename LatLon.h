/*
 * LatLon.h
 *
 *  Created on: Dec 17, 2015
 *      Author: jcassidy
 */

#ifndef LATLON_H_
#define LATLON_H_

#include <limits>

#include <iostream>
#include <iomanip>
#include <cmath>

// forward declaration to avoid pulling the whole header in
namespace boost {
    namespace serialization {
        class access;
    }
}

/** Latitude and longitude in decimal degrees
 *
 */

struct LatLon {
    float lat = std::numeric_limits<float>::quiet_NaN();
    float lon = std::numeric_limits<float>::quiet_NaN();

    LatLon() {
    }

    explicit LatLon(float ilat, float ilon) : lat(ilat), lon(ilon) {
    }

    friend std::ostream& operator<<(std::ostream& os, const LatLon ll) {
        std::size_t p = os.precision();
        os << std::setw(p + 4) << std::fabs(ll.lat) << (ll.lat < 0 ? " S  " : " N  ") << std::setw(p + 4) << std::fabs(ll.lon) << (ll.lon < 0 ? " W  " : " E  ");
        return os;
    }

private:

    template<class Archive>void serialize(Archive& ar, unsigned int ver) {
        ar & lat & lon;
    }
    friend boost::serialization::access;
};



#endif /* LATLON_H_ */
