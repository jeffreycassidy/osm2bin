/*
 * StreetsDatabaseAPI.cpp
 *
 *  Created on: Dec 17, 2015
 *      Author: jcassidy
 */

#include "StreetsDatabaseAPI.h"
#include "StreetsDatabase.h"

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/binary_iarchive.hpp>

StreetsDatabase streetsDB;

using namespace std;

// load the layer-2 streets database

bool loadStreetsDatabaseBIN(const std::string fn) {
    ifstream is(fn.c_str());
    if (!is.good())
        return false;

    boost::archive::binary_iarchive ia(is);

    ia & streetsDB;

    return true;
}

void closeStreetDatabase() {
    streetsDB = StreetsDatabase();
}

// aggregate queries

unsigned getNumberOfStreets() {
    return streetsDB.streets().size();
}

unsigned getNumberOfStreetSegments() {
    return num_edges(streetsDB.roads());
}

unsigned getNumberOfIntersections() {
    return num_vertices(streetsDB.roads());
}

unsigned getNumberOfPointsOfInterest() {
    return streetsDB.getNumberOfPOIs();
}

unsigned getNumberOfFeatures() {
    return streetsDB.getNumberOfFeatures();
}


//------------------------------------------------
// Intersection information

std::string getIntersectionName(unsigned intersectionID) {
    std::set<unsigned> streetIDs;

    const auto& G = streetsDB.roads();
    const auto v = streetsDB.intersection(intersectionID);

    for (const auto e : out_edges(v, G))
        streetIDs.insert(G[e].streetVectorIndex);

    auto it = streetIDs.begin();
    stringstream ss;

    if (it != streetIDs.end())
        ss << streetsDB.streets().at(*(it++));

    for (; it != streetIDs.end(); ++it)
        ss << " & " << streetsDB.streets().at(*it);

    return ss.str();
}

LatLon getIntersectionPosition(unsigned intersectionID) {
    return (streetsDB.roads())[streetsDB.intersection(intersectionID)].latlon;
}

OSMID getIntersectionOSMNodeID(unsigned intersectionID) {
    return (streetsDB.roads())[streetsDB.intersection(intersectionID)].osmid;
}


//number of street segments at an intersection

unsigned getIntersectionStreetSegmentCount(unsigned intersectionID) {
    return out_degree(streetsDB.intersection(intersectionID), streetsDB.roads());
}

// find the street segments at an intersection. idx is from
// 0..streetSegmentCount-1 (at this intersection)

unsigned getIntersectionStreetSegment(unsigned intersectionID, unsigned idx) {
    const auto& G = streetsDB.roads();
    const auto u = streetsDB.intersection(intersectionID);

    const auto Es = out_edges(u, G);

    if (idx >= out_degree(u, G))
        throw std::out_of_range("getIntersectionStreetSegment: idx");

    const auto e = *(begin(Es) + idx);

    return G[e].streetSegmentVectorIndex;
}



//------------------------------------------------
// Street segment information

// return info struct for the requested street segment

StreetSegmentInfo getStreetSegmentInfo(unsigned streetSegmentID) {
    StreetSegmentInfo info;

    const PathNetwork& G = streetsDB.roads();
    const auto e = streetsDB.streetSegment(streetSegmentID);

    info.from = source(e, G);
    info.to = target(e, G);

    info.oneWay = G[e].oneWay != EdgeProperties::Bidir;

    // if should be going to greater vertex number (T) but to < from (F) then swap
    // also if should be going to lesser vertex number (F) but to > from (T) also swap

    if (info.oneWay && ((G[e].oneWay == EdgeProperties::ToGreaterVertexNumber) ^ (info.from < info.to)))
        std::swap(info.from, info.to);

    info.wayOSMID = G[e].wayOSMID;
    info.streetID = G[e].streetVectorIndex;
    info.speedLimit = G[e].maxspeed;
    info.curvePointCount = G[e].curvePoints.size();

    return info;
}

//fetch the latlon of the idx'th curve point

LatLon getStreetSegmentCurvePoint(unsigned streetSegmentID, unsigned idx) {
    const PathNetwork& G = streetsDB.roads();
    const auto e = streetsDB.streetSegment(streetSegmentID);

    return G[e].curvePoints.at(idx);
}



//------------------------------------------------
// Street information

std::string getStreetName(unsigned streetID) {
    return streetsDB.streets().at(streetID); // throws exception if out of bounds
}



//------------------------------------------------
// Points of interest

std::string getPointOfInterestType(unsigned pointOfInterestID) {
    return streetsDB.poi(pointOfInterestID).type();
}

std::string getPointOfInterestName(unsigned pointOfInterestID) {
    return streetsDB.poi(pointOfInterestID).name();

}

LatLon getPointOfInterestPosition(unsigned pointOfInterestID) {
    return streetsDB.poi(pointOfInterestID).pos();

}

OSMID getPointOfInterestOSMNodeID(unsigned pointOfInterestID) {
    return streetsDB.poi(pointOfInterestID).osmNodeID();
}



//------------------------------------------------
// Natural features

FeatureType getFeatureType(unsigned featureID) {
    return streetsDB.feature(featureID).type();
}

const string& getFeatureName(unsigned featureID) {
    return streetsDB.feature(featureID).name();
}

OSMID getFeatureOSMID(unsigned featureID) {
    return streetsDB.feature(featureID).id().first;

}

OSMEntityType getFeatureOSMEntityType(unsigned featureID) {
    return streetsDB.feature(featureID).id().second;
}

unsigned getFeaturePointCount(unsigned featureID) {
    return streetsDB.feature(featureID).pointCount();

}

LatLon getFeaturePoint(unsigned featureID, unsigned idx) {
    return streetsDB.feature(featureID).point(idx);
}

