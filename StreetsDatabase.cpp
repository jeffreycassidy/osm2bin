/*
 * StreetsDatabase.cpp
 *
 *  Created on: Dec 18, 2015
 *      Author: jcassidy
 */

#include "StreetsDatabase.h"

void StreetsDatabase::buildStreetSegmentVector() {
    m_streetSegmentVector.clear();
    m_streetSegmentVector.reserve(num_edges(m_roadNetwork));

    for (const auto e : edges(m_roadNetwork)) {
        m_roadNetwork[e].streetSegmentVectorIndex = m_streetSegmentVector.size();
        m_streetSegmentVector.push_back(e);
    }
}
