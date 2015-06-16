/*
 * Projection.hpp
 *
 *  Created on: Dec 16, 2015
 *      Author: jcassidy
 */

#ifndef PROJECTION_HPP_
#define PROJECTION_HPP_

#include <array>
#include "OSMNode.hpp"

class Projection
{
public:
	Projection();

	std::array<float,2> project(LatLon ll) const;
	LatLon inverse(std::array<float,2> p) const;

	void origin(LatLon ll);
	LatLon origin() const { return m_origin; }

	void earthRadius(float Re);
	float earthRadius() const { return m_Re; }

	template<typename T>void centreWithin(std::pair<LatLon,LatLon> bounds,std::array<T,2> dims,
			typename std::enable_if< (std::is_convertible<T,float>::value), void*>::type __sfinae=nullptr)
	{
		centreWithin(bounds,std::array<float,2>{ float(dims[0]), float(dims[1]) });
	}

	void centreWithin(std::pair<LatLon,LatLon> bounds,std::array<float,2> dims)
		{ centreWithin(bounds.first,bounds.second,dims); }

	void centreWithin(LatLon llc,LatLon urc,std::array<float,2> dims);

	// provide projection-space matrix that gives vectors for north and east
	// virtual void projectionNorthEast(LatLon ll) const;

	// provide directions of local +x, +y relative to north
	//virtual void

private:
	// Handles the mechanics of an update
	virtual void 				update()=0;

	// Forward and inverse projection without an offset (defines the projection)
	virtual std::array<float,2> project_no_offset(LatLon ll) const=0;
	virtual LatLon 				inverse_no_offset(std::array<float,2> p) const=0;

	// Calculate the Jacobian (derivatives wrt (phi,lambda))
	//virtual std::pair<std::array<float,2>,std::array<float,2>> jacobian(LatLon ll) const =0;

	LatLon 				m_origin;
	float				m_Re=3440;			// default to nautical miles
	std::array<float,2> m_offset = std::array<float,2>{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::quiet_NaN()};
};



#endif /* PROJECTION_HPP_ */
