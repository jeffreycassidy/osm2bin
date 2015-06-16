/*
 * draw.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: jcassidy
 */

#include <iostream>
#include <fstream>

#include <string>
#include <cinttypes>
#include <utility>

#include "XercesUtils.hpp"
#include "LoadOSM.hpp"

#include "OSMDatabase.hpp"
#include "PathNetwork.hpp"

#include <boost/timer/timer.hpp>

#include <boost/archive/binary_oarchive.hpp>

#include "DrawPathNetwork.hpp"
#include "MidLatProjection.hpp"

using namespace std;
using namespace xercesc;

int main(int argc,char **argv)
{
	xercesc::XMLPlatformUtils::Initialize();

	string fn("hamilton_canada.osm.bz2");

	if (argc > 1)
		fn=argv[1];

	OSMDatabase db;

	db = loadOSM(fn);


	cout << "==== BUILDING NETWORK" << endl;

	OSMWayFilterRoads hwyFilt(db,db.nodes());

	auto G = buildNetwork(db,hwyFilt);

	cout << "==== DONE" << endl;



	// draw the paths to a .png surface

	std::array<unsigned,2> dims{2048,1536};

	Cairo::RefPtr<Cairo::ImageSurface> surface =
			Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, dims[0], dims[1]);

	Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

	cr->save();

	/** Cairo affine matrix is provided column-major as follows:
	 * [u] 		[	arg0	arg2	arg4]	[ x ]
	 * [v]	=	[	arg1	arg3	arg5]	[ y ]
	 * 										[ 1 ]
	 */

	/** Cairo graphics surface has +y downwards and (0,0) at upper left
	 *  Provide coordinate system in standard orientation:
	 * 		(0,0)	-> lower-left corner
	 * 		+x		-> rightwards
	 * 		+y		-> upwards
	 *
	 */

	Cairo::Matrix mat(1,0,0,-1,0,dims[1]);
	cr->set_matrix(mat);

	cr->set_source_rgb(1.0,1.0,1.0);			// paint it white
	cr->fill();

	cr->set_source_rgb(0.0,0.0,0.0);			// black 1px wide lines
	cr->set_line_width(1);

	DrawPathNetwork dpn;

	MidLatProjection proj;
	proj.centreWithin(db.bounds(),dims);		// centre bounding box within the drawing dimensions

	dpn.projection(&proj);						// set the projection
	dpn.network(&G);							// associate the network
	dpn.drawCurvePoints(true);					// draw intermediate points (not just intersection-intersection straight lines)

	dpn.draw(cr);								// draw it

	cr->set_source_rgb(1.0,0.0,0.0);			// draw without curve points
	cr->set_line_width(0.5);
	dpn.drawCurvePoints(false);
	dpn.draw(cr);

	cr->save();

	surface->write_to_png("network.png");



	// technically should close this up, but there are still some strings outstanding which will be deleted and cause trouble
	// program termination will free them anyway
	//xercesc::XMLPlatformUtils::Terminate();
}



