/*
 * ValueTable.hpp
 *
 *  Created on: Jun 14, 2015
 *      Author: jcassidy
 */

#ifndef VALUETABLE_HPP_
#define VALUETABLE_HPP_

#include <string>
#include <functional>
#include <vector>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

/** Wrapper on a vector of strings with a restricted interface to permit insertion only.
 * The container does not guarantee uniqueness, and values are in insertion order (not sorted) so inverse-lookup is linear time.
 */

class ValueTable {
public:
    /// adds a value, returns the index

    unsigned addValue(const std::string v) {
        values_.push_back(v);
        return values_.size() - 1;
    }

    /// gets a value

    const std::string& getValue(unsigned vi) const {
        return values_.at(vi);
    }

    /// Checks if an index is within the valid range

    bool valueValid(unsigned vi) const {
        return vi < values_.size();
    }

    /// Linear-time search for the index of a string value value within the table, returning -1U if not found

    unsigned getIndexOfValue(const std::string v) const {
        auto it = find(values_.begin(), values_.end(), v);
        if (it == values_.end())
            return -1U;
        else
            return it - values_.begin();
    }

    /// Function object which takes an unsigned index and returns the corresponding string

    std::function<const std::string&(unsigned) > valueLookup() const {
        return [this](unsigned i) {
            return cref(values_.at(i));
        };
    }

    /// Returns all the values (unsorted)

    const std::vector<std::string>& values() const {
        return values_;
    }

private:
    /// Vector of all strings in insertion order (unsorted)
    std::vector<std::string> values_;

    template<class Archive>void serialize(Archive& ar, const unsigned ver) {
        ar & values_;
    }
    friend class boost::serialization::access;
};




#endif /* VALUETABLE_HPP_ */
