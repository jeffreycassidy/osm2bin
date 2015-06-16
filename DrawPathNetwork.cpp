/*
 * DrawPathNetwork.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: jcassidy
 */

#include "Projection.hpp"
#include "PathNetwork.hpp"
#include "DrawPathNetwork.hpp"

#include <array>

#include <cairomm/context.h>

class Projection;

void DrawPathNetwork::draw(Cairo::RefPtr<Cairo::Context> cr) const
{
	cr->save();
	for(const auto& e : edges(*m_network))
	{
		const auto u = source(e,*m_network);
		const auto v = target(e,*m_network);

		std::array<float,2> p0 = m_proj->project((*m_network)[u].latlon);
		std::array<float,2> p1 = m_proj->project((*m_network)[v].latlon);

		cr->move_to(p0[0],p0[1]);

		if (m_drawCurvePoints)
			for(const auto cp : (*m_network)[e].curvePoints)
			{
				std::array<float,2> p = m_proj->project(cp);
				cr->line_to(p[0],p[1]);
			}

		cr->line_to(p1[0],p1[1]);
		cr->stroke();
	}
	cr->restore();
}
