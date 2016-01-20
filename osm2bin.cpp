/*
 * osm2bin.cpp
 *
 *  Created on: Jun 10, 2015
 *      Author: jcassidy
 */

#include <iostream>
#include <fstream>

#include <string>
#include <cinttypes>
#include <utility>

#include "NodePOIFilter.hpp"
#include "MultipolyCloser.hpp"

#include "XercesUtils.hpp"
#include "LoadOSM.hpp"

#include "OSMDatabase.hpp"
#include "PathNetwork.hpp"

#include <boost/timer/timer.hpp>

// path network serializations
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/range/algorithm.hpp>

#include "BasicWayFeatureFactory.hpp"
#include "BasicRelationFeatureFactory.hpp"
#include "StreetsDatabase.h"

using namespace std;

int main(int argc,char **argv)
{
	// init/terminate the xerces XMLPlatform using RAII
	XMLPlatform plat;

	string oFnRoot;
	string fn("maps/hamilton_canada.osm.gz");

	if (argc > 1)
		fn=argv[1];

	if (argc > 2)
		oFnRoot = argv[2];

	OSMDatabase db;

	db = loadOSM(fn);

	if(!oFnRoot.empty())
	{
		string oBin = oFnRoot + ".osm.bin";

		cout << "Writing to binary file " << oBin << endl;

		{
			boost::timer::auto_cpu_timer t;
			std::ofstream os(oBin.c_str(),ios_base::out | ios_base::binary);
			boost::archive::binary_oarchive oa(os);
			oa & db;
		}

		cout << "Done" << endl;
	}


	db.print();

	db.showNode(958);

	db.showWay(1);

	db.showRelation(4);

	cout << "All node tag keys: " << endl;
	for(const auto p : db.nodeTagKeys())
		cout << "  " << setw(30) << p.first << " " << p.second << endl;


	string k="highway";
	cout << "All tag values for node key " << k << endl;
	for(const auto p : db.nodeTagValuesForKey(k))
		cout << "  " << setw(30) << p.first << " " << p.second << endl;

	cout << "All way tag keys: " << endl;
		for(const auto p : db.wayTagKeys())
			cout << "  " << setw(30) << p.first << " " << p.second << endl;

	k="highway";
	cout << "All tag values for way key " << k << endl;
	for(const auto p : db.wayTagValuesForKey(k))
		cout << "  " << setw(30) << p.first << " " << p.second << endl;


	cout << "All relation tag keys: " << endl;
		for(const auto p : db.relationTagKeys())
			cout << "  " << setw(30) << p.first << " " << p.second << endl;

	k="water";
	cout << "All tag values for relation key " << k << endl;
	for(const auto p : db.relationTagValuesForKey(k))
		cout << "  " << setw(30) << p.first << " " << p.second << endl;


	cout << "==== BUILDING NETWORK" << endl;

	OSMWayFilterRoads hwyFilt(db,db.nodes());

	auto G = buildNetwork(db,hwyFilt);

	cout << "==== DONE" << endl;


	cout << "==== Extracting points of interest" << endl;
	NodePOIFilter poiFilt(db.nodeTags());

	std::vector<POI> pois;

	for(const OSMNode& n : db.nodes())
	{
		auto optPOI = poiFilt(n);
		if (optPOI)
			pois.push_back(*optPOI);
	}



	cout << "==== DONE" << endl;


	std::vector<std::string> streets = assignStreets(&db,G);

	cout << "From " << num_edges(G) << " ways, created " << streets.size() << " streets: " << endl;

	if (true)
		cout << "<skipped display>";
	else

		for(const auto& s : streets)
			cout << "  " << s << endl;

	StreetsDatabase sdb(G,streets);
	sdb.pois(std::move(pois));


	cout << "==== Extracting basic features" << endl;



	bool mapIsIsland=true;
	bool mapHasUnboundedLake=false;

	cout << "INFO: Starting on assumption that map is an island" << endl;

	vector<Feature> features;
	BasicWayFeatureFactory WF(db);
	for(const auto& w : db.ways())
	{
		auto feat = WF(w);

		if (feat)
			features.emplace_back(std::move(*feat));
	}

	BasicRelationFeatureFactory FF(db);
	for(const auto& r : db.relations())
	{
		// call the factory
		auto feats = FF(r);
		for(const auto f : feats)
		{
			cout << "  Created feature (" << asString(f.type()) << ") from relation ID " << r.id() << endl;

			if (!f.bounded() && f.isWater())
			{
				mapIsIsland=false;
				mapHasUnboundedLake=true;
				cout << "INFO: Found an unbounded water feature, so we conclude this is not an island" << endl;
			}
			features.emplace_back(std::move(f));
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

	sdb.features(std::move(features));

	cout << "Total " << sdb.getNumberOfFeatures() << " features" << endl;


	cout << "==== DONE" << endl;

	if (!oFnRoot.empty())
	{

		string oBin = oFnRoot+".streets.bin";
		cout << "Writing to binary file " << oBin << endl;

		{
			boost::timer::auto_cpu_timer t;
			std::ofstream os(oBin.c_str(),ios_base::out | ios_base::binary);
			boost::archive::binary_oarchive oa(os);
			oa & sdb;
		}
	}

	cout << "Road network has " << num_edges(G) << " edges" << endl;
}
