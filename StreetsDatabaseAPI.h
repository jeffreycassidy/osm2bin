#pragma once //protects against multiple inclusions of this header file

#include <string>
#include "LatLon.h"

#include "Feature.h"
#include "OSMEntityType.h"

typedef unsigned long long OSMID;

struct StreetSegmentInfo {
    OSMID wayOSMID; // OSM ID of the source way

    unsigned from, to; // intersection ID this segment runs from/to
    bool oneWay; // if true, then can only travel in from->to direction

    unsigned curvePointCount; // number of curve points between the ends
    float speedLimit;
    unsigned streetID; // index of street this segment belongs to
};



// load the layer-2 streets database
bool loadStreetsDatabaseBIN(std::string);
void closeStreetDatabase();

// aggregate queries
unsigned getNumberOfStreets();
unsigned getNumberOfStreetSegments();
unsigned getNumberOfIntersections();
unsigned getNumberOfPointsOfInterest();
unsigned getNumberOfFeatures(); // TO BE IMPLEMENTED


//------------------------------------------------
// Intersection information
// Throws std::out_of_range exception if intersectionID or idx are out of range

std::string getIntersectionName(unsigned intersectionID);
LatLon getIntersectionPosition(unsigned intersectionID);
OSMID getIntersectionOSMNodeID(unsigned intersectionID);

//number of street segments at an intersection
unsigned getIntersectionStreetSegmentCount(unsigned intersectionID);

// find the street segments at an intersection. idx is from 
// 0..streetSegmentCount-1 (at this intersection)
unsigned getIntersectionStreetSegment(unsigned intersectionID, unsigned idx);



//------------------------------------------------
// Street segment information
// Throws std::out_of_range exception if streetSegmentID or idx are out of range

//find the street to which this segment belongs
StreetSegmentInfo getStreetSegmentInfo(unsigned streetSegmentID);

//fetch the latlon of the idx'th curve point
LatLon getStreetSegmentCurvePoint(unsigned streetSegmentID, unsigned idx);



//------------------------------------------------
// Street information

std::string getStreetName(unsigned streetID);



//------------------------------------------------
// Points of interest

std::string getPointOfInterestType(unsigned pointOfInterestID);
std::string getPointOfInterestName(unsigned pointOfInterestID);
LatLon getPointOfInterestPosition(unsigned pointOfInterestID);
OSMID getPointOfInterestOSMNodeID(unsigned pointOfInterestID);



//------------------------------------------------
// Natural features
const std::string& getFeatureName(unsigned featureID);
FeatureType getFeatureType(unsigned featureID);
OSMID getFeatureOSMID(unsigned featureID);
OSMEntityType getFeatureOSMEntityType(unsigned featureID);
unsigned getFeaturePointCount(unsigned featureID);
LatLon getFeaturePoint(unsigned featureID, unsigned idx);
