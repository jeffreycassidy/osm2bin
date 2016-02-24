/*
 * OSMEntityFilter.hpp
 *
 *  Created on: Dec 30, 2015
 *      Author: jcassidy
 */

#ifndef OSMENTITYFILTER_HPP_
#define OSMENTITYFILTER_HPP_

class OSMDatabase;

/** Base class for filters that pick out certain types of entities (way/node/rel) from an OSMDatabse */

template<class OSMEntityType>class OSMEntityFilter {
public:

    OSMEntityFilter(const OSMDatabase& db) : m_db(db) {
    }

    virtual bool operator()(const OSMEntityType& e) const = 0;

protected:
    const OSMDatabase& m_db;
};



#endif /* OSMENTITYFILTER_HPP_ */
