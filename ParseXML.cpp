/*
 * ParseXML.cpp
 *
 *  Created on: Aug 30, 2015
 *      Author: jcassidy
 */

#include "ParseXML.hpp"

#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

using namespace xercesc;
using namespace std;

void parseXML(xercesc::InputSource* src,SAX2ContentHandler* handler);

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
