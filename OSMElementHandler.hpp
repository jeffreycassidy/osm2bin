/*
 * AttributeHandler.hpp
 *
 *  Created on: May 8, 2015
 *      Author: jcassidy
 */

#ifndef OSMELEMENTHANDLER_HPP_
#define OSMELEMENTHANDLER_HPP_

#include <string>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <iostream>
#include <vector>

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <boost/container/flat_map.hpp>

#include <functional>
#include <utility>

#include "OSMDatabaseBuilder.hpp"
#include "OSMTagHandler.hpp"
#include "XercesUtils.hpp"

#include "OSMNode.hpp"
#include "OSMRelation.hpp"
#include "OSMWay.hpp"
#include "SAX2AttributeHandler.hpp"
#include "SAX2ElementHandler.hpp"

class OSMEntity;
class OSMNode;



/** Handles <tag> elements by extracting the key and value, and looking up the key in a hash table to find the corresponding tag
 * handler.
 *
 * If the key isn't found, it runs through a set of rules to determine an appropriate handler. Failing that, the default is called.
 * The tag handler is notified that this is the first occurrence of the key.
 *
 * If the key is found, the key and value are sent to the tag handler.
 *
 */

class OSMTagElementHandler : public SAX2ElementHandler {
private:
	struct TagKeyInfo {
		OSMTagHandler* 	tagH=nullptr;
		unsigned 		idx=-1U;			// unsigned int which has specific meaning for the tag handler
		unsigned		Ninst=0;
	};

public:

	// required overrides from SAX2ElementHandler
	virtual SAX2ElementHandler* getHandlerForElement(const XMLCh*const,const XMLCh*const,const XMLCh*const){ return nullptr; }
	virtual void visitElementHandlers(Visitor*){}

	OSMTagElementHandler(const std::string name,OSMTagHandler* defaultTagHandler) :
		SAX2ElementHandler(name),
		defaultTagHandler_(defaultTagHandler)
	{
	}

	void addKey(const std::string k,OSMTagHandler* h)
	{
		auto p = keyMap_.lookupOrInsert(k);
		p.first.attr.tagH=h;
		p.first.attr.idx = h->newTagKey(p.first.idx,k);
	}

	// adds a rule to ignore a tag with a given key
	void ignoreTagWithKey(const std::string k)
	{
		auto p = keyMap_.lookupOrInsert(k);
		p.first.attr.tagH=&ignoreTag;
		p.first.attr.idx=-1U;
	}

	// adds a rule to the stack
	void addRule(OSMTagKeyRule* r)
	{
		keyRuleSet_.push_back(r);
	}

	std::vector<std::pair<std::string,unsigned>> tagKeys()
	{
		std::vector<std::pair<std::string,unsigned>> v;
		v.reserve(keyMap_.size());

		for(const auto p : keyMap_)
			v.push_back(std::make_pair(p.str,p.attr.Ninst));

		boost::sort(v,[](const std::pair<std::string,unsigned>& lhs,const std::pair<std::string,unsigned>& rhs){ return lhs.first < rhs.first; });

		return v;
	}

private:

	// XML strings used to locate key & value
	AutoXMLChPtr xmlstr_k = AutoXMLChPtr("k");
	AutoXMLChPtr xmlstr_v = AutoXMLChPtr("v");


	// Finds the key & value from the attributes, looks up the key, and sends to the appropriate handler
	virtual void startElement_(const XMLCh* const,const XMLCh*const,const XMLCh*const,const xercesc::Attributes& attrs) override
	{
		// check for correct number of attributes (2) and locate key/value
		assert(attrs.getLength() == 2);

		const XMLCh* s[2]{
			attrs.getLocalName((XMLSize_t)0),
			attrs.getLocalName((XMLSize_t)1U) };

		const XMLCh* v[2]{
			attrs.getValue((XMLSize_t)0),
			attrs.getValue((XMLSize_t)1)
		};

		unsigned idxk=2;

		if (xercesc::XMLString::compareString(xmlstr_k.get(),s[0]) == 0)
		{ idxk = 0; }
		else if (xercesc::XMLString::compareString(xmlstr_k.get(),s[1]) == 0)
		{ idxk = 1; }
		else {
			std::cerr << "ERROR: tag element missing attribute 'k' (key)" << std::endl;
		}

		unsigned idxv = (idxk+1)%2;

		if (xercesc::XMLString::compareString(xmlstr_v.get(),s[idxv]) != 0)
			std::cerr << "ERROR: tag element missing attribute 'v' (value)" << std::endl;

		if (idxk >= 2)		// Error condition
			return;

		// look up the key
		auto p = keyMap_.lookupOrInsert(v[idxk]);

		if (p.second)		// this is the first time we've seen this key
		{
			p.first.attr.tagH=nullptr;

			for(unsigned i=0;i<keyRuleSet_.size() && !p.first.attr.tagH; ++i)
				p.first.attr.tagH = keyRuleSet_[i]->getHandlerForKey(p.first.str);

			if (!p.first.attr.tagH)
				p.first.attr.tagH = defaultTagHandler_;

			// notify the tag handler of a new key coming in, get its unique integer ID
			p.first.attr.idx = p.first.attr.tagH->newTagKey(p.first.idx,p.first.str);
		}

		p.first.attr.Ninst++;
		assert(p.first.attr.tagH);
		p.first.attr.tagH->processTag(p.first.attr.idx,v[idxv]);
	}

	// process for dealing with keys: lookup in string table, then apply rules, then apply default
	XMLStringTable<TagKeyInfo> keyMap_;
	std::vector<OSMTagKeyRule*> keyRuleSet_;
	OSMTagHandler *defaultTagHandler_=nullptr;
};

class OSMNdElementHandler : public SAX2ElementHandler {
public:
	OSMNdElementHandler(OSMDatabaseBuilder* dbb) : SAX2ElementHandler("nd"),dbb_(dbb){}

private:

	OSMDatabaseBuilder *dbb_=nullptr;

	AutoXMLChPtr xmlref_ = AutoXMLChPtr("ref");

	virtual void startElement_(const XMLCh* const,const XMLCh* const,const XMLCh* const,const xercesc::Attributes& attrs)
	{
		unsigned N;
		if ((N = attrs.getLength()) != 1)
			std::cerr << "ERROR: Unexpected number of attributes for element type 'nd' (" << N << ", expecting 1)" << std::endl;
		else if (!xercesc::XMLString::equals(attrs.getLocalName(0),xmlref_.get()))
			std::cerr << "ERROR: Element 'nd' attribute is not 'ref' as expected" << std::endl;
		else
		{
			unsigned long long id;
			std::string s = XMLChString(attrs.getValue((XMLSize_t)0));
			std::stringstream ss(s);
			ss >> id;
			if (ss.fail())
				std::cerr << "ERROR: Failed to parse '" << s << "' as integer node reference" << std::endl;
			dbb_->currentWay()->addNode(id);			// add this node to the current way
		}
	}
};

class OSMMemberElementHandler : public FlatMapSAX2ElementHandler {
public:
	OSMMemberElementHandler(OSMDatabaseBuilder* dbb) : FlatMapSAX2ElementHandler("<member>"),dbb_(dbb)
	{
		addAttributeHandler("ref",makeTypedAttributeHandler<unsigned long long>([this](unsigned long long id){ id_=id; }));

		ValueTable* roles = &dbb->relationMemberRoles();

		auto roleH = makeFlatMapAttributeHandler(
				[roles](const std::string role,unsigned idx){ return roles->addValue(role); },
				[this](unsigned idx){ role_=idx; });
		addAttributeHandler("role",roleH);

		auto typeH = makeEnumAttributeHandler<OSMRelation::MemberType>([this](OSMRelation::MemberType t){ type_=t; });
			typeH->addItem("node",OSMRelation::Node);
			typeH->addItem("way",OSMRelation::Way);
			typeH->addItem("relation",OSMRelation::Relation);
		addAttributeHandler("type",typeH);
	}

private:

	OSMDatabaseBuilder *dbb_=nullptr;

	unsigned role_=-1U;
	unsigned long long id_=-1U;
	OSMRelation::MemberType type_ = OSMRelation::InvalidType;

	virtual void startElement_(const XMLCh* const,const XMLCh* const,const XMLCh* const,const xercesc::Attributes& attrs)
	{
		unsigned N;
		if ((N = attrs.getLength()) != 3)
			std::cerr << "ERROR: Unexpected number of attributes for element type 'member' (" << N << ", expecting 3)" << std::endl;
		processAttributes(attrs);
	}

	virtual void endElement_(const XMLCh* const,const XMLCh* const,const XMLCh* const)
	{
		if (type_ == OSMRelation::InvalidType || id_ == -1U || role_ == -1U)
			std::cerr << "ERROR: Invalid relation member type" << std::endl;
		else
			dbb_->currentRelation()->addMember(id_,type_,role_);

		type_ = OSMRelation::MemberType::InvalidType;
		id_ = -1U;
		role_ = -1U;
	}
};




#endif /* OSMELEMENTHANDLER_HPP_ */
