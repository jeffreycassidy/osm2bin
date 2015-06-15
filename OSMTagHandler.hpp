/*
 * OSMTagHandler.hpp
 *
 *  Created on: May 8, 2015
 *      Author: jcassidy
 */

#ifndef OSMTAGHANDLER_HPP_
#define OSMTAGHANDLER_HPP_


#include <string>
#include <vector>
#include "XercesUtils.hpp"
#include <sstream>
#include <iostream>

#include "OSMDatabaseBuilder.hpp"

#include <regex>

class OSMTagHandler;

/** Encapsulates a rule that returns the handler for a particular tag.
 * This should be run only once for each new tag key, to specify what handler will be used.
 *
 */


class OSMTagKeyRule {
public:
	virtual OSMTagHandler* getHandlerForKey(std::string)=0;
};

class OSMTagConstantKeyRule : public OSMTagKeyRule {
public:
	OSMTagConstantKeyRule(OSMTagHandler* tagH) : tagH_(tagH){}

	virtual OSMTagHandler* getHandlerForKey(const std::string)
	{
//		std::cout << "  Rule defined by constant return" << std::endl;
		return tagH_;
	}

private:
	OSMTagHandler* tagH_=nullptr;
};


class OSMTagRegexKeyRule : public OSMTagKeyRule {
public:
	OSMTagRegexKeyRule(const std::string expr,OSMTagHandler* tagH) : expr_(expr),exprStr_(expr),tagH_(tagH){}

	virtual OSMTagHandler* getHandlerForKey(const std::string s) override
	{
		if (std::regex_match(s,expr_))
		{
			//std::cout << "  Rule defined by regex match to '" << exprStr_ << "'" << std::endl;
			return tagH_;
		}
		else
			{} // std::cout << "    Did not match regex '" << exprStr_ << "'" << std::endl;
		return nullptr;
	}

private:
	std::regex expr_;
	std::string exprStr_;
	OSMTagHandler* tagH_=nullptr;
};





/** Abstract class to handle tag (key,value) pairs.
 *
 * newTagKey will be called at most once on the first occurrence of a given tag key. It returns a subclass-specific integer value,
 * which is passed to subsequent invocations of processTag.
 *
 * processTag is called on every occurrence of a given tag. The first argument is the integer value described above.
 */

class OSMTagHandler {
public:
	virtual unsigned newTagKey(unsigned ki,const std::string k)=0;
	virtual void processTag(unsigned,const XMLCh* v)=0;
};



/** Silently ignores any tag passed to it
 *
 */

class IgnoreTag : public OSMTagHandler {
public:
	virtual unsigned newTagKey(unsigned ki,const std::string k) override { return -1U; }
	virtual void processTag(unsigned, const XMLCh* v) override { }
};

extern IgnoreTag ignoreTag;



class OSMStringTableTagHandler : public OSMTagHandler {
public:
	OSMStringTableTagHandler(BoundKeyValueTable* tbl)
		: tbl_(tbl){}

	virtual unsigned newTagKey(unsigned ki,const std::string k) override
	{
		assert(ki != -1U);
		assert(tbl_);

		unsigned idx = tbl_->addKey(k);
		//std::cout << " Adding tag key '" << k << "' to string table with index " << idx << std::endl;
		return idx;
	}


//	void addKey(const std::string k,OSMTagHandler* h)
//	{
//		auto p = keyMap_.lookupOrInsert(k);
//
//		p.first.attr.tagH=h;
//
//		// TODO: Fix - this will go bust if a key is reassigned; is OK if only assigned once, hence the assertion
//		// rationale: index may not be appropriately updated (eg. if moved from ignore to string table handler)
//		assert(p.second);
//		assert(p.first.attr.tagH);
//
//		p.first.attr.idx = p.first.attr.tagH->newTagKey(p.first.idx,p.first.str);
//		cout << "  output index " << p.first.attr.idx << endl;
//		assert(p.first.attr.idx != -1U);
//	}

	virtual void processTag(unsigned ki,const XMLCh* v) override
	{
		unsigned vi;
		auto p = valueMap_.lookupOrInsert(v);

		vi = p.first->second.second;

		if (p.second)		// new value
		{
			unsigned t = tbl_->addValue(p.first->second.first);
			assert(t == p.first->second.second);
		}

		tbl_->addTag(ki,vi);
	}

private:
	BoundKeyValueTable* tbl_=nullptr;
	XMLStringTableBase valueMap_;
};

#endif /* OSMTAGHANDLER_HPP_ */
