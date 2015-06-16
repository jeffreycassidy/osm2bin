/*
 * Projection.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: jcassidy
 */

#include "Projection.hpp"

Projection::Projection()
{
}

void Projection::origin(LatLon ll)
{
	m_origin = ll;
	update();
	m_offset = project_no_offset(ll);
}

void Projection::centreWithin(LatLon llc,LatLon urc,std::array<float,2> dims)
{
	m_origin = llc;
	m_Re = 1;
	update();

	std::array<float,2> xy_llc = project_no_offset(llc);
	std::array<float,2> xy_urc = project_no_offset(urc);

	float k = 1.0/std::max( std::abs(xy_urc[0]-xy_llc[0])/dims[0], std::abs(xy_urc[1]-xy_llc[1])/dims[1] );

	m_Re *= k;
	update();

	std::array<float,2> slack{ dims[0]-k*(xy_urc[0]-xy_llc[0]), dims[1]-k*(xy_urc[1]-xy_llc[1])};

	m_offset[0] = -xy_llc[0]*k + slack[0]/2;
	m_offset[1] = -xy_llc[1]*k + slack[1]/2;
}

LatLon Projection::inverse(std::array<float,2> p) const
{
	return inverse_no_offset(std::array<float,2>{ p[0]-m_offset[0], p[1]-m_offset[1]});
}

std::array<float,2> Projection::project(LatLon ll) const
{
	std::array<float,2> p = project_no_offset(ll);
	return std::array<float,2>{ p[0]+m_offset[0], p[1]+m_offset[1] };
}
