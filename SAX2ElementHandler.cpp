#include "SAX2ElementHandler.hpp"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <sstream>
#include <iostream>
#include <iomanip>

#include <string>

using namespace xercesc;
using namespace std;


void PrintSummary::discover(SAX2ElementHandler* h)
{
	os_ << std::setw(level_*2) << ' ' << h->getName() << ": " << h->instanceCount() << " instances" << std::endl;
	level_++;
}
void PrintSummary::finish(SAX2ElementHandler* h)
{
	level_--;
}


/** Asks the current SAX2ElementHandler who should handle the element passed, then pushes that handler onto the stack and invokes it
 *
 */

void SAX2ContentHandler::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs)
{
	// figure out what handler object to call
	SAX2ElementHandler *hCurr = elements_.back().elH;
	SAX2ElementHandler *hNext = hCurr->getHandlerForElement(uri,localname,qname);

	// push this element and its associated handler onto the stack
	elements_.emplace_back(XMLElementInfo { uri, localname, qname, attrs, hNext });

	if (hNext)
		hNext->startElement(uri,localname,qname,attrs);
	else
	{
		std::string ln = std::string(XMLChString(localname));
		std::cerr << "Failed to find a handler for XML element type '" << ln << "'" << std::endl;
		std::cerr << "XML element trace: " << std::endl;
		trace();
	}
}


/** Delegates endElement to the current handler, then pops it from the trace stack
 *
 */

void SAX2ContentHandler::endElement(const XMLCh* const uri,const XMLCh* const localname,const XMLCh *const qname)
{
	elements_.back().elH->endElement(uri,localname,qname);
	elements_.pop_back();
}


/** Prints a trace of the current XML element
 *
 */

void SAX2ContentHandler::trace() const
{
	cout << "XML element stack trace" << endl;
	for(int i=elements_.size()-1; i >= 0; --i)
		cout << setw(2) << i << " localname = " << std::string(XMLChString(elements_[i].localname)) << endl;
}


/** Summarizes the document parsing (# elements)
 *
 */

void SAX2ContentHandler::endDocument()
{
	cout << "Document parse completed; summary of XML element types: " << endl;

	//cout << " Unknown/ignored elements: " << defaultElementHandler_->instanceCount() << " instances" << endl;

//	dbb_->showNode(0);
//	dbb_->showNode(100);

	//dbb_->showNode(dbb_->nodeIndexFromOSMID(18996031));
//	dbb_->showNode(958);
}
