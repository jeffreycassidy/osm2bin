/*
 * bz2read.cpp
 *
 *  Created on: Jun 10, 2015
 *      Author: jcassidy
 */

#include <iostream>
#include <fstream>

#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include "XercesUtils.hpp"
#include "OSMElementHandler.hpp"
#include "OSMDatabaseBuilder.hpp"

#include "BoostBZ2Input.hpp"
#include "SAX2ElementHandler.hpp"

using namespace std;
using namespace xercesc;

void parseXML(xercesc::InputSource* src,SAX2ContentHandler* handler);
OSMDatabase parseOSM(xercesc::InputSource *src);

std::string suffix(const std::string s,const char c='.')
{
	size_t pos = s.find_last_of(c);

	if (pos == string::npos)
		return std::string();
	else
		return s.substr(pos+1,string::npos);
}

std::pair<std::string,std::string> splitLast(const std::string s,const char c)
{
	size_t pos = s.find_last_of(c);

	if (pos == string::npos)
		return make_pair(s,string());
	else
		return make_pair(s.substr(0,pos),s.substr(pos+1,string::npos));
}

int main(int argc,char **argv)
{
	xercesc::XMLPlatformUtils::Initialize();

	string fn("hamilton_canada.osm.bz2");

	if (argc > 1)
		fn=argv[1];

	string base,sfx;

	tie(base,sfx) = splitLast(fn,'.');

	InputSource* src=nullptr;

	if (sfx == "bz2")
	{
		string mid = suffix(base);
		if (mid == "osm")
		{
			std::cout << "Reading from bzip2-compressed OSM XML file" << fn << std::endl;
			src = new BoostBZ2FileInputSource(fn);
		}
		else
			std::cerr << "Did not recognize second extension " << mid << " in .bz2 file (expecting 'osm')" << std::endl;
	}
	else if (sfx == "osm")
	{
		std::cout << "Reading from uncompressed OSM XML file " << fn << std::endl;
		src = new LocalFileInputSource(AutoXMLChPtr(fn.c_str()).get());
	}
	else
		std::cerr << "Type not recognized for file " << fn << std::endl;

	if (!src)
	{
		std::cerr << "Invalid input source - closing" << std::endl;
		return -1;
	}

	parseOSM(src);


	delete src;

	// technically should close this up, but there are still some strings outstanding which will be deleted
	//xercesc::XMLPlatformUtils::Terminate();
}



void parseXML(xercesc::InputSource* src,SAX2ContentHandler* handler)
{
	SAX2XMLReader::ValSchemes    valScheme    = SAX2XMLReader::Val_Always;
	bool                         doNamespaces = true;
	bool                         doSchema = true;
	bool                         schemaFullChecking = false;
	bool                         identityConstraintChecking = true;
	bool                         errorOccurred = false;
	bool                         namespacePrefixes = false;
	//bool                         recognizeNEL = false;
	char                         localeStr[64];
	memset(localeStr, 0, sizeof localeStr);

	//
	//  Create a SAX parser object. Then, according to what we were told on
	//  the command line, set it to validate or not.
	//
	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, doNamespaces);
	parser->setFeature(XMLUni::fgXercesSchema, doSchema);
	parser->setFeature(XMLUni::fgXercesHandleMultipleImports, true);
	parser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
	parser->setFeature(XMLUni::fgXercesIdentityConstraintChecking, identityConstraintChecking);
	parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, namespacePrefixes);

	if (valScheme == SAX2XMLReader::Val_Auto)
	{
		parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
		parser->setFeature(XMLUni::fgXercesDynamic, true);
	}
	if (valScheme == SAX2XMLReader::Val_Never)
	{
		parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
	}
	if (valScheme == SAX2XMLReader::Val_Always)
	{
		parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
		parser->setFeature(XMLUni::fgXercesDynamic, false);
	}

	parser->setContentHandler(handler);
	//parser->setErrorHandler(handler);

	XMLPScanToken token;

	try
	{
		parser->parse(*src);
	}
	catch (const OutOfMemoryException&)
	{
		std::cerr << "OutOfMemoryException" << std::endl;
		errorOccurred = true;
	}
	catch (const XMLException& e)
	{
		std::cerr << "\nError during parsing; Exception message is:  \n"
				<< e.getMessage() << "\n" << std::endl;
		errorOccurred = true;
	}
	catch (const std::exception& e)
	{
		std::cerr << std::endl << "std::exception caught during parsing of bz2 stream: what='" << e.what() << "'" << std::endl;
		errorOccurred = true;
	}
	catch (...)
	{
		std::cerr << "\nUnexpected exception during parsing of bz2 stream\n";
		errorOccurred = true;
	}

	if(errorOccurred)
		cerr << "Error in parsing" << endl;
}


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
	nodeHandler.addAttributeHandler("lat",
			makeTypedAttributeHandler<double>([&dbb](double lat){dbb.currentNode()->coords().lat=lat; }));
	nodeHandler.addAttributeHandler("lon",
			makeTypedAttributeHandler<double>([&dbb](double lon){dbb.currentNode()->coords().lon=lon; }));

	nodeTagHandler.addKey("name",&nodeTagStringTable);
	nodeTagHandler.ignoreTagWithKey("wikipedia");
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("name:.*",&ignoreTag));
	nodeTagHandler.addRule(new OSMTagRegexKeyRule("is_in.*",&ignoreTag));

	osmHandler.addElementHandler("way",&wayHandler);

	wayHandler.addElementHandler("tag",&wayTagHandler);
	wayHandler.addElementHandler("nd",&ndHandler);

	osmHandler.addElementHandler("relation",&relationHandler);
	relationHandler.addElementHandler("tag",&relationTagHandler);
	relationHandler.addElementHandler("member",&memberHandler);

	relationHandler.ignoreAttribute("timestamp");
	relationHandler.ignoreAttribute("version");
	relationHandler.ignoreAttribute("changeset");
	relationHandler.ignoreAttribute("uid");
	relationHandler.ignoreAttribute("user");

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


	OSMDatabase db = dbb.getDatabase();


	db.print();

	db.showNode(958);

	db.showWay(1);

	db.showRelation(4);

	return db;
}


