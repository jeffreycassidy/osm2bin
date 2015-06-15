/*
 * ValueTable.hpp
 *
 *  Created on: Jun 14, 2015
 *      Author: jcassidy
 */

#ifndef VALUETABLE_HPP_
#define VALUETABLE_HPP_

class ValueTable {
public:
	// adds a value, returns the index
	unsigned addValue(const std::string v)
	{
		values_.push_back(v);
		return values_.size()-1;
	}

	const std::string& getValue(unsigned vi) const
	{
		return values_.at(vi);
	}

	bool valueValid(unsigned vi) const { return vi < values_.size(); }

	std::function<const std::string&(unsigned)> valueLookup() const { return [this](unsigned i){ return cref(values_.at(i)); }; }

	const std::vector<std::string>& values() const 	{ return values_; }

private:
	std::vector<std::string> values_;
};




#endif /* VALUETABLE_HPP_ */
