#pragma once
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax/HandlerBase.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stack>

#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>

#include <unordered_map>
#include <iostream>

#include "XercesUtils.hpp"

#include "SAX2AttributeHandler.hpp"
#include "OSMDatabaseBuilder.hpp"

class SAX2ElementHandler;



class SAX2ElementHandler : public xercesc::ContentHandler {
public:
	SAX2ElementHandler(const SAX2ElementHandler& h) : xercesc::ContentHandler(),Ninst_(h.Ninst_),name_(h.name_) {};
	SAX2ElementHandler(const std::string name=std::string("")) : name_(name){}

	std::string getName() const { return name_; }
	void name(const std::string n){ name_=n; }

	unsigned instanceCount() const { return Ninst_; }

	// ask this class who should handle a given element type
	virtual SAX2ElementHandler* getHandlerForElement(const XMLCh* const uri,const XMLCh* const localname, const XMLCh* const qname)
		{ return nullptr; }

	// virtual methods inherited from ContentHandler, defaulted to nops
	virtual void characters(const XMLCh* const chars, const XMLSize_t length){ }
	virtual void endDocument(){ }
	virtual void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname){ endElement_(uri,localname,qname); }
	virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length){};

	virtual void processingInstruction(const XMLCh* const target,const XMLCh* const data){}

	virtual void setDocumentLocator(const xercesc::Locator* const locator){}

	virtual void startDocument(){}

	virtual void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc::Attributes& attrs)
	{
		++Ninst_;
		startElement_(uri,localname,qname,attrs);
	}

	virtual void startPrefixMapping(const XMLCh* const prefix,const XMLCh* const uri){}
	virtual void endPrefixMapping(const XMLCh* const prefix){}

	virtual void skippedEntity(const XMLCh* const name){}



	/** Class to visit all of the element handlers installed within this instance
	 *
	 */

	class Visitor {
	public:
		virtual void discover(SAX2ElementHandler*)=0;
		virtual void finish(SAX2ElementHandler*)=0;
	};

	virtual void visitElementHandlers(Visitor* v){ v->discover(this); v->finish(this); }

private:

	virtual void startElement_(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc::Attributes& attrs)=0;
	virtual void endElement_(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname){};
	unsigned Ninst_=0;
	std::string name_;
};


class PrintSummary : public SAX2ElementHandler::Visitor {

public:
	PrintSummary(std::ostream& os) : os_(os){}

	virtual void discover(SAX2ElementHandler* h) override;
	virtual void finish(SAX2ElementHandler* h) override;

private:
	unsigned level_=1;
	std::ostream& os_;
};



class FlatMapSAX2ElementHandler : public SAX2ElementHandler {

public:
	FlatMapSAX2ElementHandler(const FlatMapSAX2ElementHandler&) = default;

	FlatMapSAX2ElementHandler(const std::string name = std::string("")) : SAX2ElementHandler(name){}
	void addElementHandler(const std::string elType,SAX2ElementHandler* h)
	{
		const XMLCh* xmlch = xercesc::XMLString::transcode(elType.c_str());
		auto p = elementMap_.insert(std::make_pair(xmlch,h));
		if (!p.second)
			xercesc::XMLString::release((XMLCh**)&xmlch);
		else
			p.first->second = h;
	}

	void addAttributeHandler(const std::string attrname,SAX2AttributeHandler* h)
	{
		const XMLCh* xmlch = xercesc::XMLString::transcode(attrname.c_str());
		auto p = attributeMap_.insert(std::make_pair(xmlch,h));
		if (!p.second)
			xercesc::XMLString::release((XMLCh**)&xmlch);
		else
			p.first->second=h;
	}

	void ignoreAttribute(const std::string attrname)
	{
		addAttributeHandler(attrname,&::ignoreAttribute);
	}

	void setDefaultElementHandler(SAX2ElementHandler* h){ defaultElementHandler_ = h; }
	void setDefaultAttributeHandler(SAX2AttributeHandler* h){ defaultAttributeHandler_=h; }

	void processAttributes(const xercesc::Attributes& attrs)
	{
		for(unsigned i=0;i<attrs.getLength();++i)
		{
			const XMLCh* t = attrs.getLocalName(i);
			const XMLCh* v = attrs.getValue(i);

			auto p = attributeMap_.find(t);
			if (p == attributeMap_.end())		// not found
			{
				assert(defaultAttributeHandler_);
				defaultAttributeHandler_->process(t,v);
			}
			else
				p->second->process(t,v);
		}
	}

	virtual SAX2ElementHandler* getHandlerForElement(const XMLCh* const uri,const XMLCh* const localname,const XMLCh* const qname)
	{
		auto p = elementMap_.find(localname);
		std::string s = XMLChString(localname);
		return p == elementMap_.end() ? defaultElementHandler_ : p->second;
	}


	virtual void visitElementHandlers(Visitor* v)
	{
		v->discover(this);
		for (SAX2ElementHandler* h : elementMap_ | boost::adaptors::map_values)
			h->visitElementHandlers(v);
		v->finish(this);
	}

private:
	typedef boost::container::flat_map<
			const XMLCh*,
			SAX2ElementHandler*,
			std::function<bool(const XMLCh*,const XMLCh*)> > ElementMap;

	typedef boost::container::flat_map<
			const XMLCh*,
			SAX2AttributeHandler*,
			std::function<bool(const XMLCh*,const XMLCh*)> > AttributeMap;

	virtual void startElement_(const XMLCh* const,const XMLCh* const,const XMLCh* const,const xercesc::Attributes& attrs)
	{
		processAttributes(attrs);
	}

	virtual void endElement_(const XMLCh* const,const XMLCh* const,const XMLCh* const){}

	static bool xmlChLess(const XMLCh* lhs,const XMLCh* rhs){ return xercesc::XMLString::compareString(lhs,rhs)<0; }

	ElementMap elementMap_ = ElementMap(xmlChLess);
	AttributeMap attributeMap_ = AttributeMap(xmlChLess);

	SAX2ElementHandler *defaultElementHandler_=nullptr;
	SAX2AttributeHandler *defaultAttributeHandler_=nullptr;
};

class OSMEntityHandlerBase : public FlatMapSAX2ElementHandler {
public:
	OSMEntityHandlerBase(const OSMEntityHandlerBase&) = default;
	OSMEntityHandlerBase(const std::string name,OSMDatabaseBuilder* dbb) : FlatMapSAX2ElementHandler(name),dbb_(dbb){}

protected:
	OSMDatabaseBuilder *dbb_=nullptr;
};

template<class EntityType>struct OSMEntityInfo;
template<>struct OSMEntityInfo<OSMNode>{ static constexpr const char* xmlTag = "<node>"; };
template<>struct OSMEntityInfo<OSMWay>{ static constexpr const char* xmlTag = "<way>"; };
template<>struct OSMEntityInfo<OSMRelation>{ static constexpr const char* xmlTag = "<relation>"; };

template<class EntityType>class OSMEntityHandler : public OSMEntityHandlerBase {
public:
	OSMEntityHandler(const OSMEntityHandlerBase& b) : OSMEntityHandlerBase(b){ name(OSMEntityInfo<EntityType>::xmlTag); };
	OSMEntityHandler(const std::string name,OSMDatabaseBuilder* dbb) : OSMEntityHandlerBase(name,dbb){}

//	void databaseBuilder(OSMDatabaseBuilder* dbb){ dbb_=dbb; }
//	OSMDatabaseBuilder* databaseBuilder() const { return dbb_; }

	virtual void startElement_(const XMLCh* const,const XMLCh* const,const XMLCh* const,const xercesc::Attributes& attrs)
	{
		dbb_->createNew<EntityType>();
		processAttributes(attrs);
	}

	virtual void endElement_(const XMLCh* const,const XMLCh* const,const XMLCh* const)
	{
		dbb_->finishEntity<EntityType>();
	}
};



class SAX2ContentHandler : public xercesc::DefaultHandler
{

public:

	SAX2ContentHandler(SAX2ElementHandler* rootElH)
		{
			// set up base element handler; top of stack will be first to be called when an element is found
			elements_.push_back(
					XMLElementInfo { nullptr, nullptr, nullptr, *(xercesc::Attributes*)nullptr, rootElH });
		}

	~SAX2ContentHandler(){};

	// print to std::cout a trace of the XML elements in the parse stack
	void trace() const;

	// start a new element; increments the
    void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc::Attributes& attrs);

    // TODO: Delegate events other than start/end to sub-handler
    void characters(const XMLCh* const chars, const XMLSize_t length)
    {
    };

    void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length){};

    void startDocument(){
    	std::cout << "Starting document parse" << std::endl;
    };

    void endElement(const XMLCh* const uri,const XMLCh* const localname,const XMLCh *const qname);

    void endDocument();

    void visitElementHandlers(SAX2ElementHandler::Visitor* v)
    {
    	elements_.front().elH->visitElementHandlers(v);
    }

    // -----------------------------------------------------------------------
    //  Handlers for the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
	void warning(const xercesc::SAXParseException& exc){};
    void error(const xercesc::SAXParseException& exc){};
    void fatalError(const xercesc::SAXParseException& exc){};
    void resetErrors(){};

private:

    struct XMLElementInfo {
    	const XMLCh* uri;
    	const XMLCh* localname;
    	const XMLCh* qname;
    	const xercesc::Attributes& attrs;
    	SAX2ElementHandler* elH;
    };

    std::vector<XMLElementInfo> elements_;
};

