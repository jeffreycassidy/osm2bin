#include <iostream>
#include <fstream>

#include "XercesUtils.hpp"
#include "LoadOSM.hpp"

#include "OSMDatabase.hpp"
#include "BasicRelationFeatureFactory.hpp"
#include "BasicWayFeatureFactory.hpp"

#include <boost/archive/binary_oarchive.hpp>

#include <cairomm/context.h>
#include <cairomm/surface.h>

#include <array>

using namespace std;

#include "MultipolyCloser.hpp"

const std::vector<std::array<float,3>> colours{
	std::array<float,3>{ 1.0f, 0.0f, 0.0f },
	std::array<float,3>{ 0.0f, 1.0f, 0.0f },
	std::array<float,3>{ 0.0f, 0.0f, 1.0f },
	std::array<float,3>{ 0.0f, 1.0f, 1.0f },
	std::array<float,3>{ 1.0f, 0.0f, 1.0f },
	std::array<float,3>{ 1.0f, 1.0f, 0.0f },
	std::array<float,3>{ 0.5f, 0.0f, 0.0f },
	std::array<float,3>{ 0.0f, 0.5f, 0.0f },
	std::array<float,3>{ 0.0f, 0.0f, 0.5f },
	std::array<float,3>{ 0.0f, 0.5f, 0.5f },
	std::array<float,3>{ 0.5f, 0.0f, 0.5f },
	std::array<float,3>{ 0.5f, 0.5f, 0.0f }
};

#include <boost/math/constants/constants.hpp>

class MidLatProjection
{
public:
	MidLatProjection(LatLon llc) :
		m_llc(llc),
		m_cosphi(llc.lat*boost::math::constants::pi<float>()/180.0f)
	{}

	void scale(float s){ m_scale=s; }

	std::array<double,2> operator()(LatLon p) const
	{
		return std::array<double,2>
		{
			m_scale*(p.lon-m_llc.lon)*m_cosphi,
			m_scale*(p.lat-m_llc.lat)
		};
	}

private:
	LatLon 	m_llc;
	float	m_cosphi;
	float	m_scale=1.0f;
};
void drawPoly(Cairo::RefPtr<Cairo::Context> cr,MidLatProjection proj,const std::vector<LatLon>& poly,bool fill=false);

void drawPolys(Cairo::RefPtr<Cairo::Context> cr,MidLatProjection proj,const std::vector<std::vector<LatLon>>& polys,bool fill=false)
{
	cr->save();
	cr->set_line_width(0.1f);

	for(unsigned i=0;i<polys.size();++i)
		drawPoly(cr,proj,polys[i],fill);
	cr->restore();
}

void drawFeatures(Cairo::RefPtr<Cairo::Context> cr,MidLatProjection proj,const std::vector<Feature>& F)
{
	cr->save();
	cr->set_line_width(0.1f);

	cr->set_source_rgb(0.0f,0.4f,0.6f);
	for(const auto& f : F | boost::adaptors::filtered([](const Feature& f){ return f.type() == Lake || f.type() == River; }))
		drawPoly(cr,proj,f.points(),true);

	cr->set_source_rgb(0.8f,0.6f,0.2f);
	for(const auto& f : F | boost::adaptors::filtered([](const Feature& f){ return f.type() == Island; }))
			drawPoly(cr,proj,f.points(),true);

	cr->set_source_rgb(0.0f,0.4f,0.6f);
	for(const auto& f : F | boost::adaptors::filtered([](const Feature& f){ return f.type() == Stream; }))
		drawPoly(cr,proj,f.points(),false);


	cr->restore();
}



void drawPoly(Cairo::RefPtr<Cairo::Context> cr,MidLatProjection proj,const std::vector<LatLon>& poly,bool fill)
{
	array<double,2> xy;

	if (poly.size() > 0)
	{
		xy = proj(poly.front());
		cr->move_to(xy[0],xy[1]);
		for(const LatLon ll : poly)
		{
			xy = proj(ll);
			cr->line_to(xy[0],xy[1]);
		}
		if (fill)
		{
			cr->close_path();
			cr->fill();
		}
		else
			cr->stroke();
	}
}



int main(int argc,char **argv)
{
	XMLPlatform xmlp;

	string fn("/Users/jcassidy/src/ECE297/maps/hamilton_canada.osm.gz");
	string oFn("lake.pdf");

	if (argc > 1)
		fn=argv[1];

	if (argc > 2)
		oFn = argv[2];

	OSMDatabase db;

	db = loadOSM(fn);

	bool mapIsIsland=true;
	bool mapHasUnboundedLake=false;

	cout << "INFO: Starting on assumption that map is an island" << endl;

	bool image=false;

	////// Setup Cairo canvas


	std::array<unsigned,2> dims;

	if(image)
		dims = array<unsigned,2>{2048,1536};
	else
		dims = array<unsigned,2>{11*72,17*72/2};

	Cairo::RefPtr<Cairo::PdfSurface> surf = Cairo::PdfSurface::create(oFn,dims[0],dims[1]);

	Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surf);

	cr->save();

	double k;
	std::array<double,2> ddims{(double)dims[0],(double)dims[1]};
	std::array<double,2> offset;

	LatLon llc, urc;

	tie(llc,urc) = db.bounds();

	MidLatProjection proj(llc);
	pair<array<double,2>,array<double,2>> bb;
	bb = make_pair(
			proj(llc),
			proj(urc));

	assert(urc.lon > llc.lon);
	assert(urc.lat > llc.lat);


	double fillfactor=0.9;
	array<double,2> mapsize{bb.second[0]-bb.first[0], bb.second[1]-bb.first[1]};

	k = fillfactor/std::max(mapsize[0]/ddims[0], mapsize[1]/ddims[1]);

	proj.scale(k);

	bb = make_pair(
				proj(llc),
				proj(urc));

	offset = std::array<double,2>{ (ddims[0]-(bb.second[0]-bb.first[0]))*0.5, (ddims[1]-(bb.second[1]-bb.first[1]))*0.5 };

	cout << "  offset=" << offset[0] << ',' << offset[1] << " scale=" << k << endl;

	/** Cairo affine matrix is provided column-major as follows:
	 * [u] 		[	arg0	arg2	arg4]	[ x ]
	 * [v]	=	[	arg1	arg3	arg5]	[ y ]
	 * 										[ 1 ]
	 */


	Cairo::Matrix mat;

	mat = Cairo::Matrix(1,0,0,-1,offset[0],ddims[1]-offset[1]);

	cr->set_matrix(mat);

	cr->set_source_rgb(1.0,1.0,1.0);
	cr->fill();

	bb = make_pair(proj(llc), proj(urc));

	cr->set_source_rgb(0.0f,0.0f,0.0f);

	cr->rectangle(bb.first[0],bb.first[1],bb.second[0]-bb.first[0],bb.second[1]-bb.first[1]);
	cr->stroke();

	{
		array<double,2> uv;
		uv = proj(llc);
		mat.transform_point(uv[0],uv[1]);

		cout << "llc: (" << llc.lat << ',' << llc.lon << ") -> (" << uv[0] << ',' << uv[1] << ")" << endl;

		uv = proj(urc);
		mat.transform_point(uv[0],uv[1]);

		cout << "urc: (" << urc.lat << ',' << urc.lon << ") -> (" << uv[0] << ',' << uv[1] << ")" << endl;
	}


	////// Extract way water features

	vector<Feature> features;

	BasicWayFeatureFactory WF(db);

	for(const auto& w : db.ways())
	{
		auto f = WF(w);
		if (f)
			features.emplace_back(*f);
	}

	////// Extract lake multipolys and draw them

	cr->set_line_width(0.1f);
	cr->set_source_rgb(.0f,0.4f,0.6f);

	BasicRelationFeatureFactory FF(db);


	for(const auto& r : db.relations())
	{
		for(auto& feat : FF(r))
		{
			cout << "  Created feature (" << asString(feat.type()) << ") from relation ID " << r.id() << endl;

			if (!feat.bounded() && feat.isWater())
			{
				mapIsIsland=false;
				mapHasUnboundedLake=true;
				cout << "INFO: Found an unbounded water feature, so we conclude this is not an island" << endl;
			}
			features.emplace_back(std::move(feat));
		}
	}



	vector<const OSMWay*> coastline;
	unsigned m_kiNatural=db.wayTags().getIndexForKeyString("natural");
	unsigned m_viCoastline=db.wayTags().getIndexForValueString("coastline");


	// extract coastlines
	for(const auto& w : db.ways())
		if (w.hasTagWithValue(m_kiNatural,m_viCoastline))
			coastline.push_back(&w);

	cout << "Extracted " << coastline.size() << " ways with coastline tag" << endl;

	MultipolyCloser C(db,coastline);
	C.direction(MultipolyCloser::CCW);
	vector<pair<vector<LatLon>,bool>> F = C.loops(MultipolyCloser::All);

	mapIsIsland &= coastline.size()>0;

	for(const auto& poly : F)
		mapIsIsland &= poly.second;		// if none of the coastlines is unbounded, then we're looking at an island

	if (F.size() == 0)
	{
		cout << "INFO: No coastlines, so I conclude this is not an island" << endl;
		mapIsIsland=false;
	}

	if (mapIsIsland)
		cout << "INFO: There are " << F.size() << " coastline ways, none unbounded so I still think the map is an island" << endl;

	if (mapIsIsland)
	{
		cout << "INFO: Concluded the map is an island for lack of contradictory evidence" << endl;
		features.emplace_back(0,Relation,Lake,"<big ocean>",db.corners());
	}

	for(auto& poly : F)
	{
		if (poly.second)	// was originally closed
			features.emplace_back(0,Way,Island,"<unspecified>",std::move(poly.first));
		else
			features.emplace_back(0,Way,Lake,"<unspecified>",std::move(poly.first));
	}

	drawFeatures(cr,proj,features);
}
