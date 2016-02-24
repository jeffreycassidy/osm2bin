/*
 * OSMDatabaseBuilder.hpp
 *
 *  Created on: Jun 8, 2015
 *      Author: jcassidy
 */

#ifndef OSMDATABASEBUILDER_HPP_
#define OSMDATABASEBUILDER_HPP_

#include "OSMEntity.hpp"
#include "OSMWay.hpp"
#include "OSMNode.hpp"
#include "OSMRelation.hpp"
#include "OSMDatabase.hpp"

#include <boost/container/flat_map.hpp>

#include <functional>

class OSMDatabase;

class OSMDatabaseBuilder {
public:
    std::pair<LatLon, LatLon> bounds = std::make_pair(LatLon{NAN, NAN}, LatLon{NAN, NAN});

    void addNode() {
        nodes_.emplace_back();
        currentEntity_ = currentNode_ = &nodes_.back();
        nodeTags_.activeEntity(currentNode_);
    }

    OSMNode* currentNode() const {
        return currentNode_;
    }

    void addWay() {
        ways_.emplace_back();
        currentEntity_ = currentWay_ = &ways_.back();
        wayTags_.activeEntity(currentWay_);
    }

    OSMWay* currentWay() const {
        return currentWay_;
    }

    void addRelation() {
        relations_.emplace_back();
        currentEntity_ = currentRelation_ = &relations_.back();
        relationTags_.activeEntity(currentRelation_);
    }

    OSMRelation* currentRelation() const {
        return currentRelation_;
    }

    template<class EntityType>void createNew();
    template<class EntityType>void finishEntity(); // finish the current entity (set currentXXX_ = nullptr)

    OSMEntity* currentEntity() const {
        return currentEntity_;
    }


    // Interaction with the key-value tables

    BoundKeyValueTable& nodeTags() {
        return nodeTags_;
    }

    BoundKeyValueTable& wayTags() {
        return wayTags_;
    }

    BoundKeyValueTable& relationTags() {
        return relationTags_;
    }

    ValueTable& relationMemberRoles() {
        return relationMemberRoles_;
    }

    /** Note this is destructive because it moves the vectors.
     *
     */

    OSMDatabase getDatabase() {
        for (OSMNode& n : nodes_)
            n.sortTags();

        for (OSMWay& w : ways_)
            w.sortTags();

        for (OSMRelation& r : relations_)
            r.sortTags();

        return OSMDatabase(
                bounds,
                std::move(nodes_),
                std::move(nodeTags_),
                std::move(ways_),
                std::move(wayTags_),
                std::move(relations_),
                std::move(relationTags_),
                std::move(relationMemberRoles_));
    }

private:

    OSMNode* currentNode_ = nullptr;
    OSMRelation* currentRelation_ = nullptr;
    OSMWay* currentWay_ = nullptr;
    OSMEntity* currentEntity_ = nullptr;

    // for each of (nodes, relations, ways) keep a vector of keys and values
    BoundKeyValueTable nodeTags_;
    BoundKeyValueTable relationTags_;
    BoundKeyValueTable wayTags_;

    ValueTable relationMemberRoles_;

    std::vector<OSMRelation> relations_;
    std::vector<OSMNode> nodes_;
    std::vector<OSMWay> ways_;

    //OSMBounds 					bounds_;
    //OSMMap 						map_;

    friend class OSMDatabase;
};

template<>inline void OSMDatabaseBuilder::createNew<OSMNode>() {
    addNode();
}

template<>inline void OSMDatabaseBuilder::createNew<OSMRelation>() {
    addRelation();
}

template<>inline void OSMDatabaseBuilder::createNew<OSMWay>() {
    addWay();
}

template<>inline void OSMDatabaseBuilder::finishEntity<OSMNode>() {
    currentEntity_ = currentNode_ = nullptr;
}

template<>inline void OSMDatabaseBuilder::finishEntity<OSMRelation>() {
    currentEntity_ = currentRelation_ = nullptr;
}

template<>inline void OSMDatabaseBuilder::finishEntity<OSMWay>() {
    currentEntity_ = currentWay_ = nullptr;
}

#endif /* OSMDATABASEBUILDER_HPP_ */
