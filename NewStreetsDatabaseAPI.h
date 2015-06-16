#pragma once //protects against multiple inclusions of this header file

#include <string>
#include "LatLon.h"
#include "StreetSegmentEnds.h"

typedef unsigned long long OSMid;

//class
//{
//public:
//	const OSMDatabase& osmDatabase() const { return osmdb_; }
//}

//std::map<std::string,std::string> getAllNodeTagKeys();
//std::map<std::string,std::string> getAllWayTagKeys();
//std::map<std::string,std::string> getAllRelationTagKeys();
//
//std::vector<std::string> getAllTagValuesForKey(std::string);
//
//std::map<std::string,std::string> getAllTagsForObject(OSMid id);

// aggregate queries
unsigned long long getNumberOfStreets();
unsigned long long getNumberOfStreetSegments();
unsigned long long getNumberOfIntersections();
unsigned long long getNumberOfPointsOfInterest(); 

// load the database
bool loadStreetDatabaseBIN(const std::string&);
void closeStreetDatabase();

//------------------------------------------------
// Intersection information

std::string getIntersectionName(unsigned intersectionID);
LatLon getIntersectionPosition(unsigned intersectionID);
//number of street segments at an intersection
unsigned getIntersectionStreetSegmentCount(unsigned intersectionID);
// find the street segments at an intersection. idx is from 
// 0..streetSegmentCount-1 (at this intersection)
unsigned getIntersectionStreetSegment(unsigned intersectionID,unsigned idx);

//------------------------------------------------
// Street segment information

//find the street to which this segment belongs
unsigned getStreetSegmentStreetID(unsigned streetSegmentID);
//find the from/to intersections for this street segment
StreetSegmentEnds getStreetSegmentEnds(unsigned streetSegmentID);
//find the number of curve points at a street segment
unsigned getStreetSegmentCurvePointCount(unsigned streetSegmentID);
//fetch the latlon of the idx'th curve point
LatLon getStreetSegmentCurvePoint(unsigned streetSegmentID,unsigned idx);
double getStreetSegmentSpeedLimit(unsigned streetSegmentID);
bool getStreetSegmentOneWay(unsigned streetSegmentID);

//street information
std::string getStreetName(unsigned streetID);

// points of interest
std::string getPointOfInterestName(unsigned pointOfInterestID);
LatLon getPointOfInterestPosition(unsigned pointOfInterestID);

// natural features
unsigned getFeatureCount();
std::string getFeatureAttribute(unsigned feature_id ,const std::string&);
unsigned getFeaturePointCount(unsigned feature_id);
LatLon getFeaturePoint(unsigned feature_id, unsigned idx);
