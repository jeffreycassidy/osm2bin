/*
 * OSMDatabase.hpp
 *
 *  Created on: Jun 14, 2015
 *      Author: jcassidy
 */

#ifndef OSMDATABASE_HPP_
#define OSMDATABASE_HPP_

#include "OSMNode.hpp"
#include "OSMWay.hpp"
#include "OSMRelation.hpp"
#include "ValueTable.hpp"
#include "KeyValueTable.hpp"

#include <boost/container/flat_map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

#include <unordered_map>

class OSMDatabase {
public:

    OSMDatabase() {
    }

    OSMDatabase(const std::pair<LatLon, LatLon>& bounds,
            std::vector<OSMNode>&& nodes,
            KeyValueTable&& nodeTags,
            std::vector<OSMWay>&& ways,
            KeyValueTable&& wayTags,
            std::vector<OSMRelation>&& relations,
            KeyValueTable&& relationTags,
            ValueTable&& relationMemberRoles) :

    bounds_(bounds),
    nodes_(std::move(nodes)),
    nodeTags_(std::move(nodeTags)),
    ways_(std::move(ways)),
    wayTags_(std::move(wayTags)),
    relations_(std::move(relations)),
    relationTags_(std::move(relationTags)),
    relationMemberRoles_(std::move(relationMemberRoles)) {
        buildIDMap_();
    }


    // print summary information on the database
    void print() const;

    // print full information for node/relation/way
    void showNode(unsigned i) const;
    void showRelation(unsigned i) const;
    void showWay(unsigned i) const;

    const std::vector<OSMNode>& nodes() const {
        return nodes_;
    }

    const std::vector<OSMWay>& ways() const {
        return ways_;
    }

    const std::vector<OSMRelation>& relations() const {
        return relations_;
    }

    const KeyValueTable& nodeTags() const {
        return nodeTags_;
    }

    const KeyValueTable& wayTags() const {
        return wayTags_;
    }

    const KeyValueTable& relationTags() const {
        return relationTags_;
    }

    const ValueTable& relationRoles() const {
        return relationMemberRoles_;
    }

    std::vector<LatLon> extractPoly(const OSMWay& way) const;

    const OSMRelation& relationFromID(unsigned long long id) const {
        return *idToRelationMap_.at(id);
    }

    const OSMWay& wayFromID(unsigned long long id) const {
        return *idToWayMap_.at(id);
    }

    const OSMWay* wayPtrFromID(unsigned long long id) const {
        const auto it = idToWayMap_.find(id);
        if (it != idToWayMap_.end())
            return it->second;
        else
            return nullptr;
    }

    const OSMNode& nodeFromID(unsigned long long id) const {
        return *idToNodeMap_.at(id);
    }

    const OSMNode* nodePtrFromID(unsigned long long id) const {
        const auto it = idToNodeMap_.find(id);
        return it == idToNodeMap_.end() ? nullptr : it->second;
    }

    std::vector<LatLon> corners() const {
        return std::vector<LatLon>{
            bounds_.first,
            LatLon(bounds_.second.lat, bounds_.first.lon),
            bounds_.second,
            LatLon(bounds_.first.lat, bounds_.second.lon)
        };
    }

    // returns all key string values for a given entity type

    std::vector<std::pair<std::string, unsigned>> nodeTagKeys() const {
        return tagKeys(nodes_, nodeTags_);
    }

    std::vector<std::pair<std::string, unsigned>> wayTagKeys() const {
        return tagKeys(ways_, wayTags_);
    }

    std::vector<std::pair<std::string, unsigned>> relationTagKeys() const {
        return tagKeys(relations_, relationTags_);
    }

    // returns all string values for a given key over all of an entity type (nodes/ways/relations)

    std::vector<std::pair<std::string, unsigned>> nodeTagValuesForKey(const std::string k) const {
        return tagValuesForKey(k, nodes_, nodeTags_);
    }

    std::vector<std::pair<std::string, unsigned>> wayTagValuesForKey(const std::string k) const {
        return tagValuesForKey(k, ways_, wayTags_);
    }

    std::vector<std::pair<std::string, unsigned>> relationTagValuesForKey(const std::string k) const {
        return tagValuesForKey(k, relations_, relationTags_);
    }

    // map bounds reported by OSM file

    std::pair<LatLon, LatLon> bounds() const {
        return bounds_;
    }

private:

    template<typename OSMEntityRange>std::vector<std::pair<std::string, unsigned>> tagKeys(OSMEntityRange R, const KeyValueTable& tbl) const;
    template<typename OSMEntityRange>std::vector<std::pair<std::string, unsigned>> tagValuesForKey(const std::string, OSMEntityRange R, const KeyValueTable& tbl) const;

    std::unordered_map<unsigned long long, const OSMNode*> idToNodeMap_;
    std::unordered_map<unsigned long long, const OSMWay*> idToWayMap_;
    std::unordered_map<unsigned long long, const OSMRelation*> idToRelationMap_;

    std::pair<LatLon, LatLon> bounds_ = std::make_pair(LatLon{NAN, NAN}, LatLon{NAN, NAN});

    std::vector<OSMNode> nodes_;
    KeyValueTable nodeTags_;

    std::vector<OSMWay> ways_;
    KeyValueTable wayTags_;

    std::vector<OSMRelation> relations_;
    KeyValueTable relationTags_;

    ValueTable relationMemberRoles_;

    template<class Archive>void serialize(Archive& ar, const unsigned ver) {
        ar & bounds_ & nodes_ & nodeTags_ & ways_ & wayTags_ & relations_ & relationTags_ & relationMemberRoles_;
        buildIDMap_();
    }

    void buildIDMap_() {
        idToWayMap_.clear();
        idToNodeMap_.clear();
        idToRelationMap_.clear();

        idToNodeMap_.reserve(nodes_.size());
        idToRelationMap_.reserve(relations_.size());
        idToWayMap_.reserve(ways_.size());

        // build ID to entity (node/way/relation) map
        bool inserted;
        for (const auto& w : ways_) {
            tie(std::ignore, inserted) = idToWayMap_.insert(std::make_pair(w.id(), &w));
            assert(inserted);
        }
        for (const auto& r : relations_) {
            tie(std::ignore, inserted) = idToRelationMap_.insert(std::make_pair(r.id(), &r));
            assert(inserted);
        }
        for (const auto& n : nodes_) {
            tie(std::ignore, inserted) = idToNodeMap_.insert(std::make_pair(n.id(), &n));
            assert(inserted);
        }
    }

    friend class boost::serialization::access;
};

template<typename OSMEntityRange>std::vector<std::pair<std::string, unsigned>> OSMDatabase::tagKeys(OSMEntityRange r, const KeyValueTable& tbl) const {
    std::vector<std::pair < std::string, unsigned>> v(tbl.keys().size());

    for (unsigned ki = 0; ki < v.size(); ++ki)
        v[ki] = std::make_pair(tbl.keys()[ki], 0);

    for (const auto& entity : r)
        for (const auto ki : entity.tags() | boost::adaptors::map_keys)
            v[ki].second++;

    return v;
}

template<typename OSMEntityRange>std::vector<std::pair<std::string, unsigned>> OSMDatabase::tagValuesForKey(const std::string k, OSMEntityRange r, const KeyValueTable& tbl) const {
    // lookup integer ID corresponding to key name k
    auto p = std::find(tbl.keys().begin(), tbl.keys().end(), k);

    std::unordered_map<unsigned, unsigned> m;

    if (p != tbl.keys().end()) {
        unsigned ki = p - tbl.keys().begin();

        for (const auto& entity : r) {
            for (const auto p : entity.tags() | boost::adaptors::filtered([ki](std::pair<const unsigned, unsigned> p) {
                    return p.first == ki; }))
            m[p.second]++;
        }

    }


    std::vector<std::pair < std::string, unsigned>> v;

    for (const auto p : m)
        v.emplace_back(tbl.getValue(p.first), p.second);

    return v;
}


#endif /* OSMDATABASE_HPP_ */
