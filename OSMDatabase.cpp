/*
 * OSMDatabase.cpp
 *
 *  Created on: Jun 15, 2015
 *      Author: jcassidy
 */

#include "OSMDatabase.hpp"
#include <boost/range/adaptor/transformed.hpp>


void OSMDatabase::print() const
{
	std::cout << "OSMDatabaseBuilder summary: " << std::endl;
	std::cout << "  Bounds: " << bounds_.first << "-" << bounds_.second << std::endl;
	std::cout << "  " << relations_.size() << " relations" << std::endl;
	std::cout << "  " << nodes_.size() << " nodes" << std::endl;
	std::cout << "  " << ways_.size() << " ways" << std::endl;

	std::cout << "  Relation member roles: ";
	for(const auto r : relationMemberRoles_.values())
		std::cout << r << "  ";
	std::cout << std::endl;
}

void OSMDatabase::showNode(unsigned i) const
{
	std::cout << "Full details for node " << i << " (OSM ID " << nodes_[i].id() << ")" << std::endl;
	std::cout << "  Coords (" << nodes_[i].coords().lat << "," << nodes_[i].coords().lon << ")" << std::endl;

	for(auto p : nodes_[i].tags() | boost::adaptors::transformed(nodeTags_.pairLookup()))
		std::cout << "  " << p.first << " = " << p.second << std::endl;
}

void OSMDatabase::showRelation(unsigned i) const
{
	std::cout << "Full details for relation " << i << "(OSM ID " << relations_[i].id() << ")" << std::endl;
	std::cout << " Tags" << std::endl;
	for(auto p : relations_[i].tags() | boost::adaptors::transformed(relationTags_.pairLookup()))
		std::cout << "  " << p.first << " = " << p.second << std::endl;

	std::cout << " Members" << std::endl;

	boost::container::flat_map<OSMRelation::MemberType,std::string> typeMap;

	typeMap.insert(std::make_pair(OSMRelation::Node,std::string("node")));
	typeMap.insert(std::make_pair(OSMRelation::Way,std::string("way")));
	typeMap.insert(std::make_pair(OSMRelation::Relation,std::string("relation")));

	for(auto p : relations_[i].members())
	{
		std::cout << "  type='" << typeMap.at(p.type) << " ID " << p.id << "  role='" << relationMemberRoles_.getValue(p.role) << "' " << std::endl;
	}
}

void OSMDatabase::showWay(unsigned i) const
{
	std::cout << "Full details for way " << i << " (OSM ID " << ways_[i].id() << ")" << std::endl;
	std::cout << " Tags" << std::endl;
	for(auto p : ways_[i].tags() | boost::adaptors::transformed(wayTags_.pairLookup()))
		std::cout << "  " << p.first << " = " << p.second << std::endl;
	std::cout << " Node refs" << std::endl;

	for(auto p: ways_[i].ndrefs())
		std::cout << "  " << p << std::endl;
}
