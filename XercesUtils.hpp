/*
 * XercesUtils.hpp
 *
 *  Created on: May 8, 2015
 *      Author: jcassidy
 */

#ifndef XERCESUTILS_HPP_
#define XERCESUTILS_HPP_

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <boost/functional/hash.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/map.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <memory>

#include <boost/optional.hpp>

#include <functional>

#include <iostream>

#include <unordered_map>


/** RAII approach to starting and terminating xercesc platform */

class XMLPlatform
{
public:
	XMLPlatform(){ xercesc::XMLPlatformUtils::Initialize(); }
	~XMLPlatform(){ xercesc::XMLPlatformUtils::Terminate(); }
};

class AutoXMLChPtr
{
public:
	// require explicit acquisition of ownership to prevent premature deletion
	explicit AutoXMLChPtr(const XMLCh* p) : p_(p){}

	// allow implicit conversion from string/const char* with automatic duration
	AutoXMLChPtr(const std::string s) 	: p_(xercesc::XMLString::transcode(s.c_str())){}
	AutoXMLChPtr(const char* p) 		: p_(xercesc::XMLString::transcode(p)){}

	// forbid copying, permit moving to keep ownership unique
	AutoXMLChPtr(const AutoXMLChPtr&) = delete;
	AutoXMLChPtr(AutoXMLChPtr&& rhs) : p_(rhs.p_){ rhs.p_=nullptr; }
	AutoXMLChPtr& operator=(AutoXMLChPtr&& rhs){
		if (p_)
			xercesc::XMLString::release((XMLCh**)&p_);
		p_=rhs.p_; rhs.p_=nullptr; return *this;
	}

	AutoXMLChPtr clone() const
	{
		return AutoXMLChPtr(xercesc::XMLString::replicate(p_));
	}

	~AutoXMLChPtr(){ if (p_) xercesc::XMLString::release((XMLCh**)&p_); }

	const XMLCh* release(){
		const XMLCh* p=p_;
		p_=nullptr;
		return p;
	}

	const XMLCh* get() const { return p_; }

	operator std::string() const
	{
		const char* c = xercesc::XMLString::transcode(p_);
		std::string s(c);
		xercesc::XMLString::release((char**)&c);
		return s;
	}

	static bool less(const AutoXMLChPtr& lhs,const AutoXMLChPtr& rhs)
		{ return xercesc::XMLString::compareString(lhs.p_,rhs.p_) < 0; }

	static bool lessI(const AutoXMLChPtr& lhs,const AutoXMLChPtr& rhs)
			{ return xercesc::XMLString::compareIString(lhs.p_,rhs.p_) < 0; }

	static bool equals(const AutoXMLChPtr& lhs,const AutoXMLChPtr& rhs)
		{ return xercesc::XMLString::equals(lhs.p_,rhs.p_); }

	static int compare(const AutoXMLChPtr& lhs,const AutoXMLChPtr& rhs)
		{ return xercesc::XMLString::compareString(lhs.p_,rhs.p_); }

	static int compareI(const AutoXMLChPtr& lhs,const AutoXMLChPtr& rhs)
		{ return xercesc::XMLString::compareIString(lhs.p_,rhs.p_); }

protected:
	const XMLCh* p_;
};

class XMLChString : public AutoXMLChPtr {
public:
	XMLChString(const XMLCh* p) : AutoXMLChPtr(p){}

	// should only ever be created by conversion
	XMLChString(const XMLChString&) = delete;
	XMLChString(XMLChString&&)		= delete;

	~XMLChString(){ p_=nullptr; }			// prevent parent from deleting
};



/** Maps an XMLCh* string to a unique unsigned integer index, a standard string, and an unsigned index.
 * Currently supports insertion but not deletion.
 */

class XMLStringTableBase
{
private:
	typedef std::unordered_map<
			const XMLCh*,
			std::pair<std::string,unsigned>,
			std::size_t(*)(const XMLCh*),
			bool(*)(const XMLCh*,const XMLCh*)> IndexMap;

public:

	typedef IndexMap::iterator 			iterator;
	typedef IndexMap::const_iterator 	const_iterator;

	XMLStringTableBase(){}

	~XMLStringTableBase()
	{
		for(const XMLCh* p : imap_ | boost::adaptors::map_keys)
			delete (p);
			//xercesc::XMLString::release((XMLCh**)&p);
	}

	std::pair<iterator,bool> lookupOrInsert(const XMLCh* k)
	{
		iterator newIt;
		bool inserted;
		std::tie(newIt,inserted) = imap_.insert(std::make_pair(k,std::make_pair(std::string(),imap_.size())));

		// defer creation of the std::string representation until we know it doesn't already exist
		if (inserted)
		{
			newIt->second.first = XMLChString(k);

			// Cast away constness of the key and make it a copy of the argument.
			// Does not break the hash because although the pointers are different, the values pointed to compare the same.
			XMLSize_t len = xercesc::XMLString::stringLen(k);
			(XMLCh*&)(newIt->first) = new XMLCh[len+1];					// <== cast way constness of key
			xercesc::XMLString::copyString((XMLCh*&)(newIt->first),k);
		}

		return std::make_pair(newIt,inserted);
	}

	std::pair<iterator,bool> lookupOrInsert(const std::string k)
	{
		// transcode the string into XML string (required for hash key)
		AutoXMLChPtr xmlp(k);

		// attempt insertion
		auto p = imap_.insert(std::make_pair(xmlp.get(),std::make_pair(k,imap_.size())));

		// if inserted, grant ownership to the hashtable for deletion in destructor
		if (p.second)
			xmlp.release();

		return p;
	}

	const_iterator begin() const { return imap_.begin(); }
	const_iterator   end() const { return imap_.end();   }

	iterator begin() { return imap_.begin(); }
	iterator end()   { return imap_.end();   }

	std::size_t size() const { return imap_.size(); }



	boost::optional<const_iterator> lookup(const XMLCh* k) const
	{
		const_iterator it = imap_.find(k);
		return it == imap_.end() ? boost::optional<const_iterator>() : boost::optional<const_iterator>(it);
	}

	boost::optional<iterator> lookup(const XMLCh* k)
	{
		iterator it = imap_.find(k);
		return it == imap_.end() ? boost::optional<iterator>() : boost::optional<iterator>(it);
	}

	boost::optional<const_iterator> lookup(const std::string k) const
	{
		AutoXMLChPtr xmlp(k);
		return lookup(xmlp.get());
	}

	boost::optional<iterator> lookup(const std::string k)
	{
		AutoXMLChPtr xmlp(k);
		return lookup(xmlp.get());
	}


private:


	static std::size_t hash_(const XMLCh* p)
	{
		return std::size_t(xercesc::XMLString::hash(p,(XMLSize_t)-1));
	}

	IndexMap imap_ = IndexMap(0,&hash_,&xercesc::XMLString::equals);
};


template<class Attributes>class XMLStringTable {

public:

	struct Entry {
		const XMLCh*		xmlstr;		// XML string rep
		const std::string&	str;		// localized string rep
		unsigned 			idx;		// index in string table
		Attributes& 		attr;		// user-defined attributes
	};

	class iterator : public boost::iterator_facade<
		iterator,
		Entry,
		std::forward_iterator_tag,
		Entry>
	{

	public:
		iterator(std::vector<Attributes>& st,XMLStringTableBase::const_iterator it) : st_(st),it_(it){}

	private:
		std::vector<Attributes>& st_;
		XMLStringTableBase::const_iterator it_;

		friend class boost::iterator_core_access;
		void increment(){ it_++; }
		bool equal(iterator const& rhs) const { return it_==rhs.it_; }
		Entry dereference() const { return Entry { it_->first, it_->second.first, it_->second.second, st_[it_->second.second] }; }
	};

	void insertNew(const std::string k,const Attributes& a)
	{
		auto p = st_.lookupOrInsert(k);

		// must be a new insertion
		assert(p.second);

		// should be inserting at end of attributes vector
		assert(attrs_.size() == p.first->second.second);

		attrs_.push_back(a);
	}

	template<typename T>std::pair<Entry,bool> lookupOrInsert(const T k)
	{
		auto p = st_.lookupOrInsert(k);
		if (p.second)
		{
			assert(attrs_.size() == p.first->second.second);
			attrs_.push_back(Attributes());
		}
		return std::make_pair( Entry { p.first->first, p.first->second.first, p.first->second.second, attrs_[p.first->second.second] } , p.second);
	}

	iterator begin(){ return iterator(attrs_,st_.begin()); }
	iterator   end(){ return iterator(attrs_,st_.end()); }
//
//	const_iterator begin() const { return const_iterator(attrs_,st_.begin()); }
//	const_iterator   end() const { return const_iterator(attrs_,st_.end()); }

	std::size_t size() const { return st_.size(); }

private:

	XMLStringTableBase st_;
	std::vector<Attributes> attrs_;
};


#endif /* XERCESUTILS_HPP_ */
