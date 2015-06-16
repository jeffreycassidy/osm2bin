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
		FF(r);
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
//
//	for (const auto e : edges(G))
//		assert(source(e,G) > target(e,G));

//	auto E = edges(G);
//
//	auto e = *E.first;
//	auto v = target(e,G);
//	auto u = source(e,G);
//
//	cout << "Edge e(" << u << "," << v << ")" << endl;
//
//	cout << "Out-edges of u=" << u << endl;
//	for(const auto oe : out_edges(u,G))
//		cout << "  " << setw(6) << source(oe,G) << " <-> " << target(oe,G) << endl;
//
//
//	cout << "Out-edges of v=" << v << endl;
//	for(const auto oe : out_edges(v,G))
//		cout << "  " << setw(6) << source(oe,G) << " <-> " << target(oe,G) << endl;

}
