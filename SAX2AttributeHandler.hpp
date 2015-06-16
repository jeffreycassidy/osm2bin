/*
 * SAX2AttributeHandler.hpp
 *
 *  Created on: May 10, 2015
 *      Author: jcassidy
 */

#ifndef SAX2AttributeHandler_HPP_
#define SAX2AttributeHandler_HPP_

#include <string>
#include <vector>
#include "XercesUtils.hpp"
#include <sstream>
#include <iostream>

#include <boost/container/flat_map.hpp>

#include <unordered_map>

/** Abstract class to handle element attributes
 *
 */

class SAX2AttributeHandler {
public:
	virtual void process(const XMLCh* t,const XMLCh* v){ process(std::string(XMLChString(t)),v); }
	virtual void process(const std::string k,const XMLCh* v)=0;
};



/** Silently ignores any attribute passed to it
 *
 */

class IgnoreAttribute : public SAX2AttributeHandler {
public:
	void process(const std::string k,const XMLCh* v){}
};

extern IgnoreAttribute ignoreAttribute;



/** Prints a warning the first time it encounters an unknown attribute
 *
 */

class WarnAttribute : public SAX2AttributeHandler {
public:
	WarnAttribute(std::string elementType) : m_eltype(elementType){}
	void process(const std::string k,const XMLCh* v)
	{
		auto p = m_attributes.insert(make_pair(k,1));
		if (p.second)
			std::cout << "WARNING: Unhandled attribute '" << k << "' on element of type '" << m_eltype << "' (notifying on first occurrence only)" << std::endl;
		else
			p.first->second++;
	}

private:
	std::string m_eltype="?unspecified?";
	std::unordered_map<std::string,unsigned> m_attributes;
};


/** Displays an info message for an unknown/unexpected attribute
 *
 */

class UnknownAttributeHandler : public SAX2AttributeHandler {
public:
	void process(const std::string k,const XMLCh* v)
	{
		std::cout << "  Unknown attribute '" << k << '\'' << std::endl;
	}
};

extern UnknownAttributeHandler unknownAttribute;


/** Adds the subject value to a string table
 *
 */

template<class NewValueNotifier,class ValueNotifier>class FlatMapAttributeHandler : public SAX2AttributeHandler {
public:
	FlatMapAttributeHandler(NewValueNotifier newN, ValueNotifier vN) : newN_(newN),vN_(vN){}

	void process(const std::string k,const XMLCh* v)
	{
		auto p = valueMap_.find(v);

		if (p == valueMap_.end())
		{
			bool inserted;
			std::tie(p,inserted) = valueMap_.insert(std::make_pair((const XMLCh*)xercesc::XMLString::replicate(v),(unsigned)strs_.size()));
			assert(inserted);
			strs_.push_back(std::string(XMLChString(v)));
			newN_(strs_.back(),p->second);
		}

		vN_(p->second);
	}

private:
	boost::container::flat_map<
		const XMLCh*,
		unsigned,
		std::function<bool(const XMLChString&,const XMLChString&)>> valueMap_ = boost::container::flat_map<
			const XMLCh*,
			unsigned,
			std::function<bool(const XMLChString&,const XMLChString&)>>(XMLChString::less);

	NewValueNotifier newN_;
	ValueNotifier	vN_;

	std::vector<std::string> strs_;
};


template<class NewValueNotifier,class ValueNotifier>FlatMapAttributeHandler<NewValueNotifier,ValueNotifier>* makeFlatMapAttributeHandler
	(NewValueNotifier newN,ValueNotifier vN)
	{
		return new FlatMapAttributeHandler<NewValueNotifier,ValueNotifier>(newN,vN);
	}


/** Uses the standard extraction operator to convert the string to a value of type T, then calls a unary notifier function
 * with that value
 *
 * @tparam Notifier 	Unary function object
 * @tparam T 			Value type
 *
 */

template<typename T,class Notifier>class TypedAttributeHandler : public SAX2AttributeHandler {
public:
	TypedAttributeHandler(Notifier n) : n_(n){ }

	void process(const std::string,const XMLCh* v)
	{
		std::string s=XMLChString(v);
		T i;
		std::stringstream ss(s);
		ss >> i;

		if (!ss.fail())
			n_(i);
		else
			std::cerr << "Failed to parse '" << s << "' as type " << std::endl;
	}

private:
	Notifier n_;
};

template<typename T,typename Notifier>SAX2AttributeHandler* makeTypedAttributeHandler(Notifier n)
{
	return new TypedAttributeHandler<T,Notifier>(n);
}



/** Extracts an enum value using a Boost flat_map to map from XMLCh* to enum values.
 *
 * @tparam	T			Enum class
 * @tparam	Notifier	Unary function type to be called when value is received
 *
 */

template<typename T,class Notifier>class EnumAttributeHandler : public SAX2AttributeHandler {
private:
	typedef boost::container::flat_map<
			AutoXMLChPtr,
			T,
			std::function<bool(const AutoXMLChPtr&,const AutoXMLChPtr&)> > EnumMap;

public:

	EnumAttributeHandler(Notifier n) : types_(AutoXMLChPtr::less),n_(n){}

	void process(const std::string,const XMLCh* v)
	{
		XMLChString xmlp(v);

		typename EnumMap::const_iterator it = types_.find(xmlp);

		if (it != types_.end())
			n_(it->second);
		else
			std::cout << "Unknown enum member " << std::string(XMLChString(v)) << std::endl;

	}

	void addItem(const std::string k,T v){ types_.insert(std::make_pair(AutoXMLChPtr(k),v)); }

private:
	EnumMap types_;
	Notifier n_;
};

template<typename T,class Notifier>EnumAttributeHandler<T,Notifier>* makeEnumAttributeHandler(Notifier n)
{
	return new EnumAttributeHandler<T,Notifier>(n);
}



#endif /* SAX2AttributeHandler_HPP_ */
