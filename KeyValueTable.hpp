/*
 * KeyValueTable.hpp
 *
 *  Created on: Jun 14, 2015
 *      Author: jcassidy
 */

#ifndef KEYVALUETABLE_HPP_
#define KEYVALUETABLE_HPP_

#include <string>
#include <vector>
#include <boost/serialization/serialization.hpp>
#include <cassert>

#include "OSMEntity.hpp"
#include "ValueTable.hpp"

class KeyValueTable {
public:

    unsigned addKey(const std::string k) {
        return keys_.addValue(k);
    }

    unsigned addValue(const std::string v) {
        return values_.addValue(v);
    }

    const std::string& getKey(unsigned ki) const {
        return keys_.getValue(ki);
    }

    const std::string& getValue(unsigned vi) const {
        return values_.getValue(vi);
    }

    std::pair<std::string, std::string> getKeyValue(std::pair<unsigned, unsigned> p) const {
        return std::make_pair(keys_.getValue(p.first), values_.getValue(p.second));
    }

    bool keyValid(unsigned ki) const {
        return keys_.valueValid(ki);
    }

    bool valueValid(unsigned vi) const {
        return values_.valueValid(vi);
    }

    unsigned getIndexForKeyString(const std::string s) const {
        return keys_.getIndexOfValue(s);
    }

    unsigned getIndexForValueString(const std::string s) const {
        return values_.getIndexOfValue(s);
    }

    std::function<const std::string&(unsigned) > keyLookup() const {
        return [this](unsigned i) {
            return cref(keys_.getValue(i));
        };
    }

    std::function<const std::string&(unsigned) > valueLookup() const {
        return [this](unsigned i) {
            return cref(values_.getValue(i));
        };
    }

    std::function<std::pair<const std::string&, const std::string&>(std::pair<unsigned, unsigned>) > pairLookup() const {
        return [this](const std::pair<unsigned, unsigned> p) {
            return std::make_pair(cref(keys_.getValue(p.first)), cref(values_.getValue(p.second)));
        };
    }

    const std::vector<std::string>& keys() const {
        return keys_.values();
    }

    const std::vector<std::string>& values() const {
        return values_.values();
    }

private:
    ValueTable keys_;
    ValueTable values_;

    template<class Archive>void serialize(Archive& ar, const unsigned ver) {
        ar & keys_ & values_;
    }
    friend class boost::serialization::access;
};

/** A key-value table which is bound to a specific set of tags.
 *
 */

class BoundKeyValueTable : public KeyValueTable {
public:

    BoundKeyValueTable(OSMEntity* e = nullptr) : entity_(e) {
    }

    void addTag(unsigned ki, unsigned vi) {
        assert(entity_);
        assert(keyValid(ki));
        assert(valueValid(vi));

        entity_->addTag(ki, vi);
    }

    void activeEntity(OSMEntity* e) {
        entity_ = e;
    }

    OSMEntity* activeEntity() const {
        return entity_;
    }

private:
    OSMEntity* entity_ = nullptr;
};




#endif /* KEYVALUETABLE_HPP_ */
