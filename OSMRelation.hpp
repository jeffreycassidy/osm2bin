#pragma once
#include "OSMEntity.hpp"



#include <vector>

class OSMRelation : public OSMEntity {
public:

    typedef enum {
        InvalidType, Node, Way, Relation
    } MemberType;

    struct Member {
        unsigned long long id;
        MemberType type;
        unsigned role; // string table index

    private:

        template<class Archive> void serialize(Archive& ar, const unsigned) {
            ar & id & type & role;
        }
        friend class boost::serialization::access;
    };

    OSMRelation(unsigned long long id = 0) : OSMEntity(id) {
    }

    void addMember(unsigned long long id, MemberType type, unsigned role) {
        addMember(Member{id, type, role});
    }

    void addMember(const Member m) {
        members_.push_back(m);
    }

    const std::vector<Member>& members() const {
        return members_;
    }

private:
    std::vector<Member> members_;

    template<class Archive>void serialize(Archive& ar, const unsigned) {
        ar & boost::serialization::base_object<OSMEntity>(*this) & members_;
    }
    friend boost::serialization::access;

};

//istream& operator>>(istream& is,OSMRelation::MemberRole& r);

//ostream& operator<<(ostream& os,const Relation& r);
