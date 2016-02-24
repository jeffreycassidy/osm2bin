#include "OSMEntity.hpp"

#include "OSMNode.hpp"
#include "OSMWay.hpp"
#include "OSMRelation.hpp"

// load the optional layer-1 OSM database
bool loadOSMDatabaseBIN(const std::string&);
void closeOSMDatabase();

// Query the number of entities in the database
unsigned long long getNumberOfNodes();
unsigned long long getNumberOfWays();
unsigned long long getNumberOfRelations();

// Query all nodes in the database, by node index
const OSMNode* getNodeByIndex(unsigned idx);
const OSMWay* getWayByIndex(unsigned idx);
const OSMRelation* getRelationByIndex(unsigned idx);

// Count number of tags for a given OSMEntity (OSMWay/OSMNode/OSMRelation)
unsigned getTagCount(const OSMEntity* e);

// Return n'th key-value pair
std::pair<std::string, std::string> getTagPair(const OSMEntity* e, unsigned idx);
