/*
 * explorer.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: jcassidy
 */


#include <iostream>
#include <fstream>

#include <string>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/graph/adj_list_serialize.hpp>

#include "StreetsDatabase.h"
#include "StreetsDatabaseAPI.h"

#include "OSMDatabaseAPI.h"

using namespace std;

// print all tags for a given entity
void showEntityTags(const OSMEntity* e);

// TODO: handle multipolygon relations (<relation> type=multipolygon with roles=inner|outer

// NOTE: Lake ontario is depicted as both <relation> type=multipolygon with role=inner|outer </relation>
// and <relation> type=collection </relation> holding the shoreline

// TODO: For all multipolygons, close on inner/outer
// for water=lake, inner->island, outer->lake

// other relation types
// 		route
// 		boundary

// NOTE: New York coastline is a set of ways, apparently without a relation to tie them together
// Closed coastline must necessarily be land.
// TODO:
// 		find all coastlines terminating at a boundary, close in oriented (clockwise?) fashion along bounds to give ocean/land
// 		find all closed coastlines and mark them as land



int main(int argc,char **argv)
{
	string fnPfx="hamilton_canada";

	if (argc >= 2)
		fnPfx = argv[1];

	string streetsFn = fnPfx+".streets.bin";

	loadStreetsDatabaseBIN(streetsFn);

	cout << "Loaded binary data from '" << streetsFn <<  "'" << endl;

	cout << "Road network has " << getNumberOfIntersections() << " intersections and " << getNumberOfStreetSegments() << " street segments" << endl;
	cout << "There are " << getNumberOfStreets() << " streets" << endl;

	cout << endl << endl;
	cout << "All streets: " << endl;

	for(unsigned i=0; i < getNumberOfStreets(); ++i)
		cout << "  " << setw(6) << i << " " << getStreetName(i) << endl;

	cout << endl << endl;
	cout << "All intersections: " << endl;

	cout << "Road network has " << getNumberOfIntersections() << " intersections and " << getNumberOfStreetSegments() << " street segments" << endl;
	cout << "There are " << getNumberOfStreets() << " streets" << endl;

	cout << endl << endl;
	cout << "There are " << getNumberOfPointsOfInterest() << " points of interest" << endl;
	cout << "POI 1 name='" << getPointOfInterestName(1) << "' type='" << getPointOfInterestType(1) << "' LatLon="
			<< getPointOfInterestPosition(1).lat << ',' << getPointOfInterestPosition(1).lon << " OSMID=" <<
			getPointOfInterestOSMNodeID(1) << endl;

	// display first 10 intersections
	const unsigned nToShow=10;
	cout << "Showing first " << nToShow << " intersections" << endl;
	for(unsigned i=0;i < getNumberOfIntersections() && i < nToShow; ++i)
	{
		cout << "  " << setw(6) << i << "  " << setw(50) << getIntersectionName(i) << " (OSM ID " << setw(9) << getIntersectionOSMNodeID(i) << ") at " << getIntersectionPosition(i) << " has " << getIntersectionStreetSegmentCount(i) << " incident street segments" << endl;
		for(unsigned j=0;j<getIntersectionStreetSegmentCount(i); ++j)
		{
			StreetSegmentInfo info = getStreetSegmentInfo(getIntersectionStreetSegment(i,j));
			cout << "        " << setw(20) << getStreetName(info.streetID) << " #" << setw(6) << info.streetID << " (OSM way ID " << setw(9) << info.wayOSMID << ") " <<
					(info.oneWay ? " one way " : "         ") << " limit " << info.speedLimit << endl;
		}
	}


	const unsigned nFeaturesToShow=10;
	cout << "Showing first " << nFeaturesToShow << " features" << endl;
	for(unsigned i=0;i<getNumberOfFeatures() && i < nFeaturesToShow; ++i)
	{
		OSMID id = getFeatureOSMID(i);
		FeatureType t = getFeatureType(i);
		OSMEntityType type = getFeatureOSMEntityType(i);
		string name = getFeatureName(i);

		cout << "  " << setw(6) << i << "  " << setw(50) << getFeatureName(i) << " feature of type '" << asString(t) << "' from OSM ";

		switch(type)
		{
		case Way:
			cout << "way"; break;
		case Node:
			cout << "node"; break;
		case Relation:
			cout << "relation"; break;
		default:
			cout << "<unknown-entity-type>"; break;
		}

		cout << " ID " << getFeatureOSMID(i);

		cout << "    Points: ";
		for(unsigned j=0;j<getFeaturePointCount(i);++j)
		{
			LatLon ll = getFeaturePoint(i,j);
			cout << fixed << setprecision(3) << setw(7) << ll.lat << ',' << setw(8) << ll.lon << ' ';
		}
		cout << endl;
	}



	cout << endl << endl;
	string osmFn = fnPfx+".osm.bin";
	cout << "Loading OSM binary data from '" << osmFn << "'" << endl;
	cout << "======== Layer-1 Queries" << endl;

	loadOSMDatabaseBIN(osmFn);

	cout << "  There are " << getNumberOfNodes() << " nodes, " << getNumberOfWays() << " ways and " << getNumberOfRelations() << " relations" << endl;


	for(unsigned i=0;i<100 && i<getNumberOfNodes();++i)
	{
		const OSMNode* n = getNodeByIndex(i);
		cout << "Node #" << i << " (ID " << n->id() << ')' << endl;
		showEntityTags(n);
	}

	for(unsigned i=0;i<100 && i<getNumberOfWays();++i)
	{
		const OSMWay* w = getWayByIndex(i);
		cout << "Way #" << i << " (ID " << w->id() << ')' << endl;
		showEntityTags(w);
	}

	for(unsigned i=0;i<100 && i<getNumberOfRelations();++i)
	{
		const OSMRelation* r = getRelationByIndex(i);
		cout << "Relation #" << i << " (ID " << r->id() << ")" << endl;
		showEntityTags(r);
	}
}

void showEntityTags(const OSMEntity* e)
{
	for(unsigned i=0;i<getTagCount(e); ++i)
	{
		pair<string,string> kv = getTagPair(e,i);
		cout << "    " << setw(20) << kv.first << ": " << kv.second << endl;
	}
}
