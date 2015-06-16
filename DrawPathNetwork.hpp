/*
 * DrawPathNetwork.hpp
 *
 *  Created on: Dec 16, 2015
 *      Author: jcassidy
 */

#ifndef DRAWPATHNETWORK_HPP_
#define DRAWPATHNETWORK_HPP_

#include "PathNetwork.hpp"

#include <cairomm/context.h>

class Projection;

class DrawPathNetwork
{
public:

	void projection(Projection* proj){ m_proj=proj; }
	void network(PathNetwork* n){ m_network=n; }

	void draw(Cairo::RefPtr<Cairo::Context> cr) const;

	bool drawCurvePoints() const { return m_drawCurvePoints; }
	bool drawCurvePoints(bool en){ std::swap(m_drawCurvePoints,en); return en; }

private:
	Projection* 	m_proj=nullptr;
	PathNetwork* 	m_network=nullptr;

	// drawing options
	bool 			m_drawCurvePoints=false;
};

#endif /* DRAWPATHNETWORK_HPP_ */
