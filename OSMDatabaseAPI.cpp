/*
 * OSMDatabaseAPI.cpp
 *
 *  Created on: Dec 23, 2015
 *      Author: jcassidy
 */

#include <fstream>
#include "OSMDatabaseAPI.h"
#include "OSMDatabase.hpp"
#include <string>
#include <utility>

#include <boost/archive/binary_iarchive.hpp>

using namespace std;

OSMDatabase osmdb;

// load the optional layer-1 OSM database

bool loadOSMDatabaseBIN(const std::string& fn) {
    ifstream is(fn.c_str(), ios_base::in | ios_base::binary);

    boost::archive::binary_iarchive ia(is);

    ia & osmdb;

    return true;
}

void closeOSMDatabase() {
    osmdb = OSMDatabase();
}

// Query the number of entities in the database

unsigned long long getNumberOfNodes() {
    return osmdb.nodes().size();
}

unsigned long long getNumberOfWays() {
    return osmdb.ways().size();
}

unsigned long long getNumberOfRelations() {
    return osmdb.relations().size();
}

// Query all nodes in the database, by node index

const OSMNode* getNodeByIndex(unsigned idx) {
    return &osmdb.nodes().at(idx);
}

const OSMWay* getWayByIndex(unsigned idx) {
    return &osmdb.ways().at(idx);
}

const OSMRelation* getRelationByIndex(unsigned idx) {
    return &osmdb.relations().at(idx);
}

unsigned getTagCount(const OSMEntity* e) {
    return e->tags().size();
}

std::pair<std::string, std::string> getTagPair(const OSMEntity* e, unsigned tagIdx) {
    const OSMNode* n = nullptr;
    const OSMWay* w = nullptr;
    const OSMRelation* r = nullptr;

    std::pair<unsigned, unsigned> p = e->tags().at(tagIdx);

    if ((n = dynamic_cast<const OSMNode*> (e)))
        return osmdb.nodeTags().getKeyValue(p);
    else if ((w = dynamic_cast<const OSMWay*> (e)))
        return osmdb.wayTags().getKeyValue(p);
    else if ((r = dynamic_cast<const OSMRelation*> (e)))
        return osmdb.relationTags().getKeyValue(p);

    assert(false);
}


