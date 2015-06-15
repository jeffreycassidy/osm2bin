#pragma once
#include "OSMEntity.hpp"
#include <boost/serialization/base_object.hpp>

#include <boost/container/flat_map.hpp>



#include <vector>

using namespace std;

class OSMRelation : public OSMEntity {
public:
	typedef enum { InvalidType, Node, Way, Relation } MemberType;

	struct Member {
		unsigned long long 	id;
		MemberType			type;
		unsigned 			role;		// string table index
	};

	OSMRelation(unsigned long long id=0) : OSMEntity(id){}

	void addMember(unsigned long long id, MemberType type,unsigned role){ addMember(Member{id,type,role}); }
	void addMember(const Member m){ members_.push_back(m); }

	const std::vector<Member>& members() const { return members_; }

private:
	vector<Member> members_;

//	friend boost::serialization::access;
//	template<class Archive>void serialize(Archive& ar,unsigned int version){
//		ar & boost::serialization::base_object<OSMEntity>(*this) & members_;
//	}
};

//istream& operator>>(istream& is,OSMRelation::MemberRole& r);

//ostream& operator<<(ostream& os,const Relation& r);
