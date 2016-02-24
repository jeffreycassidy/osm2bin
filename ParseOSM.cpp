/*
 * ParseOSM.cpp
 *
 *  Created on: Aug 30, 2015
 *      Author: jcassidy
 */

#include "ParseXML.hpp"

#include "OSMDatabase.hpp"
#include "OSMDatabaseBuilder.hpp"

#include "OSMTagHandler.hpp"
#include "OSMElementHandler.hpp"

#include "SAX2ElementHandler.hpp"

#include <iostream>
#include <iomanip>

using namespace std;

OSMDatabase parseOSM(xercesc::InputSource *src)
{
	//  Builder for the basic OSM database
	OSMDatabaseBuilder dbb;

	// base element handler for all OSM entities (node, way, rel) with some common attribute rules and ID extraction
	OSMEntityHandlerBase baseEntityHandler("???",&dbb);

	baseEntityHandler.ignoreAttribute("timestamp");
	baseEntityHandler.ignoreAttribute("version");
	baseEntityHandler.ignoreAttribute("changeset");
	baseEntityHandler.ignoreAttribute("uid");
	baseEntityHandler.ignoreAttribute("user");

	baseEntityHandler.addAttributeHandler("id",
			makeTypedAttributeHandler<unsigned long long>([&dbb](unsigned long long id){dbb.currentEntity()->id(id); }));

	// Create document structure
	FlatMapSAX2ElementHandler rootHandler("(document)");
	rootHandler.setDefaultAttributeHandler(&unknownAttribute);

	FlatMapSAX2ElementHandler osmHandler("<osm>");
	osmHandler.setDefaultAttributeHandler(&unknownAttribute);

	FlatMapSAX2ElementHandler boundsHandler("<bounds>");
	boundsHandler.setDefaultAttributeHandler(&unknownAttribute);

	OSMEntityHandler<OSMNode> 		nodeHandler(baseEntityHandler);				// code node, way, relation from prototype
	OSMEntityHandler<OSMWay> 		wayHandler(baseEntityHandler);
	OSMEntityHandler<OSMRelation> 	relationHandler(baseEntityHandler);

	OSMStringTableTagHandler nodeTagStringTable(&dbb.nodeTags());
	OSMStringTableTagHandler wayTagStringTable(&dbb.wayTags());
	OSMStringTableTagHandler relationTagStringTable(&dbb.relationTags());

	OSMTagElementHandler nodeTagHandler("<tag>",			&nodeTagStringTable);
	OSMTagElementHandler wayTagHandler("<way>",				&wayTagStringTable);
	OSMTagElementHandler relationTagHandler("<relation>",	&relationTagStringTable);

	OSMNdElementHandler ndHandler(&dbb);
	OSMMemberElementHandler memberHandler(&dbb);

	// Document structure with element & attribute handlers
	rootHandler.addElementHandler("osm",&osmHandler);

	osmHandler.ignoreAttribute("generator");
	osmHandler.ignoreAttribute("version");
	osmHandler.ignoreAttribute("timestamp");

	osmHandler.addElementHandler("bounds",&boundsHandler);

	boundsHandler.ignoreAttribute("origin");
	boundsHandler.addAttributeHandler("minlat",makeTypedAttributeHandler<float>([&dbb](double minlat){ dbb.bounds.first.lat=minlat; }));
	boundsHandler.addAttributeHandler("minlon",makeTypedAttributeHandler<float>([&dbb](double minlon){ dbb.bounds.first.lon=minlon; }));
	boundsHandler.addAttributeHandler("maxlat",makeTypedAttributeHandler<float>([&dbb](double maxlat){ dbb.bounds.second.lat=maxlat; }));
	boundsHandler.addAttributeHandler("maxlon",makeTypedAttributeHandler<float>([&dbb](double maxlon){ dbb.bounds.second.lon=maxlon; }));

	osmHandler.addElementHandler("node",&nodeHandler);

	nodeHandler.addElementHandler("tag",&nodeTagHandler);
	WarnAttribute warnNode("<node>");

	nodeHandler.setDefaultAttributeHandler(&warnNode);

	nodeHandler.addAttributeHandler("lat",
			makeTypedAttributeHandler<double>([&dbb](double lat){dbb.currentNode()->coords().lat=lat; }));
	nodeHandler.addAttributeHandler("lon",
			makeTypedAttributeHandler<double>([&dbb](double lon){dbb.currentNode()->coords().lon=lon; }));

	nodeTagHandler.addKey("name",&nodeTagStringTable);

	std::vector<std::string> nodeTagKeysToIgnore{
		"source",
		"created_by",
		"crossing",
		"gates",
		"lights",
		"go_zone",
		"crossing_ref",
		"red_light_camera",
		"button",
		"wheelchair",
		"fixme",
		"noexit",
		"guidepost",
		"layer",
		"power",
		"aeroway",
		"drive_through",
		"dispensing",
		"alt_name",
		"drive_thru",
		"brand",
		"opening_hours",
		"tower:type",
		"phone",
		"barrier",
		"traffic_calming",
		"emergency",
		"note",
		"fee",
		"fireplace",
		"FIXME",
		"indoor",
		"old_name",
		"internet_access",
		"building:levels",
		"board_type",
		"information",
		"office",
		"route",
		"attribution",
		"material",
		"contents",
		"height",
		"works:type",
		"pipeline",
		"landuse",
		"content",
		"color",
		"trim",
		"bench",
		"countdown_signal",
		"covered",
		"hiking",
		"map_size",
		"map_type",
		"entrance",
		"toilets",
		"signal",
		"colour",
		"designation",
		"motor_vehicle",
		"vehicle",
		"disused",
		"vending",
		"email",
		"smoking",
		"capacity",
		"computer",
		"backrest",
		"street_lamp",
		"booth",
		"banquet",
		"crossing:barrier",
		"crossing:bell",
		"supervised",
		"lanes",
		"surface",
		"motorcar",
		"bollard",
		"url",
		"services",
		"seats",
		"tactile_paving"
	};

	std::vector<std::string> wayTagKeysToIgnore{
		"source",
		"electrified",
		"gauge",
		"line",
		"operator",
		"created_by",
		"handrail:right",
		"handrail:left",
		"attribution",
		"note",
		"FIXME",
		"alt_name",
		"wikipedia",
		"website",
		"voltage",
		"fee",
		"park_ride",
		"iata",
		"parking:condition:area",
		"validate:no_name",
		"trail_visibility",
		"color",
		"evangelical",
		"start_date",
		"fireplace",
		"fax",
		"roof:height"
	};

	nodeTagHandler.addRule(new OSMTagRegexKeyRule("addr:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("name:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("is_in.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("payment:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("contact:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("currency:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("canvec:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("generator:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("toilets:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("wetap:.*",&ignoreTag));


	for(const auto& k : nodeTagKeysToIgnore)
		nodeTagHandler.ignoreTagWithKey(k);

	osmHandler.addElementHandler("way",&wayHandler);

	wayHandler.addElementHandler("tag",&wayTagHandler);
	wayHandler.addElementHandler("nd",&ndHandler);

	WarnAttribute warnWay("<way>");
	wayHandler.setDefaultAttributeHandler(&warnWay);

	wayTagHandler.addKey("name",&wayTagStringTable);
	wayTagHandler.addKey("name:en",&wayTagStringTable);


	wayTagHandler.addRule(new OSMTagRegexKeyRule("geobase:.*",&ignoreTag));
	wayTagHandler.addRule(new OSMTagRegexKeyRule("canvec:.*",&ignoreTag));
	wayTagHandler.addRule(new OSMTagRegexKeyRule("statscan:.*",&ignoreTag));
	wayTagHandler.addRule(new OSMTagRegexKeyRule("addr:.*",&ignoreTag));
	wayTagHandler.addRule(new OSMTagRegexKeyRule("name:.*",&ignoreTag));
	wayTagHandler.addRule(new OSMTagRegexKeyRule("payment:.*",&ignoreTag));
	wayTagHandler.addRule(new OSMTagRegexKeyRule("capacity:.*",&ignoreTag));

	for(const auto& k : wayTagKeysToIgnore)
		wayTagHandler.ignoreTagWithKey(k);

	osmHandler.addElementHandler("relation",&relationHandler);
	relationHandler.addElementHandler("tag",&relationTagHandler);
	relationHandler.addElementHandler("member",&memberHandler);

	WarnAttribute relWarn("<relation>");
	relationHandler.setDefaultAttributeHandler(&relWarn);

	relationHandler.ignoreAttribute("timestamp");
	relationHandler.ignoreAttribute("version");
	relationHandler.ignoreAttribute("changeset");
	relationHandler.ignoreAttribute("uid");
	relationHandler.ignoreAttribute("user");

	std::vector<std::string> relationTagKeysToIgnore{
		"wikipedia",
		"note",
		"attribution",
		"is_in",
		"fixme",
		"day_off",
		"day_on",
		"hour_off",
		"hour_on",
		"FXIME",
		"FIXME",
		"source"
	};

	relationTagHandler.addKey("name",&relationTagStringTable);
	relationTagHandler.addKey("name:en",&relationTagStringTable);

	relationTagHandler.addRule(new OSMTagRegexKeyRule("name:.*",&ignoreTag));
	relationTagHandler.addRule(new OSMTagRegexKeyRule("canvec:.*",&ignoreTag));
	relationTagHandler.addRule(new OSMTagRegexKeyRule("wikipedia:.*",&ignoreTag));
	relationTagHandler.addRule(new OSMTagRegexKeyRule("addr:.*",&ignoreTag));

	for(const auto& k : relationTagKeysToIgnore)
		relationTagHandler.ignoreTagWithKey(k);

	SAX2ContentHandler handler(&rootHandler);

	parseXML(src,&handler);


	PrintSummary summ(std::cout);
	rootHandler.visitElementHandlers(&summ);

	vector<pair<string,unsigned>> v = nodeTagHandler.tagKeys();
	cout << "Node tag keys: " << endl;
	for(const auto p : v)
		cout << "  " << setw(30) << p.first << "  " << p.second << endl;

	v = relationTagHandler.tagKeys();
	cout << "Relation tag keys: " << endl;
	for(const auto p : v)
		cout << "  " << setw(30) << p.first << "  " << p.second << endl;

	v = wayTagHandler.tagKeys();
	cout << "Way tag keys: " << endl;
	for(const auto p : v)
		cout << "  " << setw(30) << p.first << "  " << p.second << endl;


	return dbb.getDatabase();
}
