#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/map.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>

#include "OSMEntityType.h"

class OSMEntity {
public:

    OSMEntity(OSMID id_) : id_(id_) {
    }

    OSMEntity(OSMEntity&&) = default;
    OSMEntity(const OSMEntity&) = default;
    OSMEntity& operator=(const OSMEntity&) = default;
    OSMEntity& operator=(OSMEntity&&) = default;

    virtual ~OSMEntity() {
    }

    void addTag(unsigned k, unsigned v) {
        tags_.push_back(std::make_pair(k, v));
    }

    unsigned long long id() const {
        return id_;
    }

    void id(OSMID newID) {
        id_ = newID;
    }

    const std::vector<std::pair<unsigned, unsigned>>&tags() const {
        return tags_;
    }


    /// Must call this to sort the tags by key index before calling any of the functions listed below

    void sortTags() {
        boost::sort(tags_, tagKeyLess);
    }

    /// Uses a binary search to find the tag index; requires that sortTags has been called first

    unsigned getValueForKey(unsigned ki) const {
        auto it = boost::lower_bound(tags_, ki, tagKeyLess);
        return (it == tags_.end() || it->first != ki) ? -1U : it->second;
    }

    bool hasTagWithValue(unsigned k, unsigned v) const {
        const auto it = boost::lower_bound(tags_, k, tagKeyLess);
        return it != tags_.end() && *it == std::make_pair(k, v);
    }

    bool hasTag(unsigned k) const {
        const auto it = boost::lower_bound(tags_, k, tagKeyLess);
        return it != tags_.end() && it->first == k;
    }

    static bool osmIDLess(const OSMEntity& lhs, const OSMEntity& rhs) {
        return lhs.id_ < rhs.id_;
    }

private:

    static struct {

        bool operator()(const std::pair<unsigned, unsigned> lhs, const std::pair<unsigned, unsigned> rhs) const {
            return lhs.first < rhs.first;
        }

        bool operator()(const std::pair<unsigned, unsigned> lhs, unsigned rhs) const {
            return lhs.first < rhs;
        }

        bool operator()(unsigned lhs, const std::pair<unsigned, unsigned> rhs) const {
            return lhs < rhs.first;
        }
    } tagKeyLess;

    unsigned long long id_;
    std::vector<std::pair<unsigned, unsigned>> tags_;

    template<class Archive>void serialize(Archive& ar, const unsigned ver) {
        ar & id_ & tags_;
    }
    friend class boost::serialization::access;
};
