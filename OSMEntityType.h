/*
 * OSMEntityType.h
 *
 *  Created on: Dec 30, 2015
 *      Author: jcassidy
 */

#ifndef OSMENTITYTYPE_H_
#define OSMENTITYTYPE_H_

enum OSMEntityType {
    Invalid = 0,
    Node,
    Way,
    Relation
};

class OSMEntity;
class OSMNode;
class OSMWay;
class OSMRelation;

typedef unsigned long long OSMID;

#endif /* OSMENTITYTYPE_H_ */
