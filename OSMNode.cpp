/*
 * OSMNode.cpp
 *
 *  Created on: May 10, 2015
 *      Author: jcassidy
 */

#include "OSMNode.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

ostream& operator<<(ostream& os,const OSMNode& n)
{
	return os << "Node id " << setw(12) << n.id_ << " at coords " << setprecision(6) << n.coords();
}

ostream& operator<<(ostream& os,const LatLon ll)
{
	std::size_t p = os.precision();
	os << setw(p+4) << fabs(ll.lat) << (ll.lat < 0 ? " S  " : " N  ") << setw(p+4) << fabs(ll.lon) << (ll.lon < 0 ? " W  " : " E  ");
	return os;
}
