/*
 * Compass.cpp
 *
 *  Created on: Jan 14, 2016
 *      Author: jcassidy
 */

#include "Compass.hpp"

using namespace std;

#include <boost/static_assert.hpp>

const string CompassPrincipal::s_names[8]{
	"north",
	"northeast",
	"east",
	"southeast",
	"south",
	"southwest",
	"west",
	"northwest"
};


CompassIntercardinal operator+(const CompassIntercardinal& c,int dir)
{
	return CompassIntercardinal((int(c)+dir+4)%4);
}

CompassIntercardinal& operator += (CompassIntercardinal& c,int dir)
{
	int t = (int(c)+dir) % 4;

	// check proper function of modulus
	BOOST_STATIC_ASSERT((-6) % 4 == -2);

	c = CompassIntercardinal(t);
	return c;

}


ostream& operator<<(ostream& os,CompassPrincipal c)
{
	return os << CompassPrincipal::s_names[unsigned(c)];
}

ostream& operator<<(ostream& os,CompassCardinal c)
{
	return os << CompassPrincipal(c);
}

ostream& operator<<(ostream& os,CompassIntercardinal c)
{
	return os << CompassPrincipal(c);
}
