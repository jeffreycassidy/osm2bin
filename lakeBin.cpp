#include <iostream>
#include <fstream>

#include "StreetsDatabase.h"
#include "Feature.hpp"

#include <boost/archive/binary_iarchive.hpp>

#include <boost/graph/adj_list_serialize.hpp>

#include <cairomm/context.h>
#include <cairomm/surface.h>

#include <array>

using namespace std;

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
	string fn("hamilton_canada.streets.bin");
	string oFn("hamilton_canada.water.pdf");

	if (argc > 1)
		fn=argv[1];

	if (argc > 2)
		oFn = argv[2];

	StreetsDatabase db;

	{
		ifstream is(fn);
		boost::archive::binary_iarchive ia(is);

		ia & db;
	}

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

	LatLon llc(90.f,180.f), urc(-90.f,-180.f);

	for(unsigned i=0;i<db.getNumberOfFeatures();++i)
		for(const LatLon ll : db.feature(i).points())
		{
			llc.lat = std::min(llc.lat,ll.lat);
			llc.lon = std::min(llc.lon,ll.lon);
			urc.lat = std::max(urc.lat,ll.lat);
			urc.lon = std::max(urc.lon,ll.lon);
		}


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

	vector<Feature> F;

	for(unsigned i=0;i<db.getNumberOfFeatures(); ++i)
		F.push_back(db.feature(i));

	cout << "Database has " << F.size() << " features" << endl;

	drawFeatures(cr,proj,F);
}
