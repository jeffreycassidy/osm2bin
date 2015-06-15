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

class OSMDatabase {

public:

	OSMDatabase(){}

	OSMDatabase(const std::pair<LatLon,LatLon>& bounds,
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
				relationMemberRoles_(std::move(relationMemberRoles))
	{}


	void print() const
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

	void showNode(unsigned i) const
	{
		std::cout << "Full details for node " << i << " (OSM ID " << nodes_[i].id() << ")" << std::endl;
		std::cout << "  Coords (" << nodes_[i].coords().lat << "," << nodes_[i].coords().lon << ")" << std::endl;

		for(auto p : nodes_[i].tags() | boost::adaptors::transformed(nodeTags_.pairLookup()))
			std::cout << "  " << p.first << " = " << p.second << std::endl;
	}

	void showRelation(unsigned i) const
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

	void showWay(unsigned i) const
	{
		std::cout << "Full details for way" << i << " (OSM ID " << ways_[i].id() << ")" << std::endl;
		std::cout << " Tags" << std::endl;
		for(auto p : ways_[i].tags() | boost::adaptors::transformed(wayTags_.pairLookup()))
			std::cout << "  " << p.first << " = " << p.second << std::endl;
		std::cout << " Node refs" << std::endl;

		for(auto p: ways_[i].ndrefs())
			std::cout << "  " << p << std::endl;
	}


private:

	std::pair<LatLon,LatLon> bounds_ = std::make_pair( LatLon { NAN, NAN }, LatLon { NAN, NAN} );

	std::vector<OSMNode> 		nodes_;
	KeyValueTable 				nodeTags_;

	std::vector<OSMWay>			ways_;
	KeyValueTable 				wayTags_;

	std::vector<OSMRelation> 	relations_;
	KeyValueTable 				relationTags_;

	ValueTable 					relationMemberRoles_;

	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ ar & bounds_ & nodes_ & nodeTags_ & ways_ & wayTags_ & relations_ & relationTags_ & relationMemberRoles_; }

	friend class boost::serialization::access;
};


#endif /* OSMDATABASE_HPP_ */
