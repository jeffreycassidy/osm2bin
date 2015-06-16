/*
 * LoadOSM.cpp
 *
 *  Created on: Aug 30, 2015
 *      Author: jcassidy
 */

#include "OSMDatabase.hpp"
#include <string>
#include <utility>

#include <boost/archive/binary_iarchive.hpp>

#include "XercesUtils.hpp"

#include "ParseOSM.hpp"

#include <string>

#include <xercesc/framework/LocalFileInputSource.hpp>
#include "CompressedFileInput.hpp"

using namespace std;


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

OSMDatabase loadOSM(const std::string fn)
{
	OSMDatabase db;
	string base,sfx;

	tie(base,sfx) = splitLast(fn,'.');

	xercesc::InputSource* src=nullptr;

	if (sfx == "bz2" || sfx == "gz" || sfx == "osm")
	{
		if (sfx == "osm")
		{
			std::cout << "Reading from uncompressed OSM XML file " << fn << std::endl;
			src = new xercesc::LocalFileInputSource(AutoXMLChPtr(fn.c_str()).get());
		}
		else
		{
			string mid = suffix(base);
			if (mid != "osm")
			{
				cerr << "Did not recognize second extension " << mid << " in ." << sfx << " file (expecting 'osm')" << endl;
				return OSMDatabase();
			}
			else if (sfx == "bz2" || sfx == "gz")
			{
				cout << "Reading from compressed OSM XML file " << fn << endl;
				src = new CompressedFileInputSource(fn);
			}
		}
		db = parseOSM(src);
	}
	else if (sfx == "bin")
	{
		std::cout << "Reading from binary file" << fn << std::endl;
		std::ifstream is(fn.c_str(),ios_base::in | ios_base::binary);
		boost::archive::binary_iarchive ia(is);

		ia & db;
	}
	else {
		std::cerr << "Invalid input source - closing" << std::endl;
	}


	delete src;

	cout << "Loaded database with " << db.nodes().size() << " nodes, " << db.ways().size() << " ways, and " << db.relations().size() << " relations" << endl;

	return db;

}
