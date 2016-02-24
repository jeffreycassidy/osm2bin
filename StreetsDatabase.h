/*
 * StreetsDatabase.h
 *
 *  Created on: Dec 18, 2015
 *      Author: jcassidy
 */

#ifndef STREETSDATABASE_H_
#define STREETSDATABASE_H_

#include "PathNetwork.hpp"

#include <vector>
#include <string>

#include "POI.hpp"
#include "Feature.h"

#include <utility>

class StreetsDatabase {
public:

    StreetsDatabase() : m_roadNetwork() {
    }

    ~StreetsDatabase() {
    }

    StreetsDatabase(const PathNetwork& roads_, std::vector<std::string> streets_)
    : m_roadNetwork(roads_), m_streets(streets_) {
        buildStreetSegmentVector();
    }

    const PathNetwork& roads() const {
        return m_roadNetwork;
    }

    const std::vector<std::string>& streets() const {
        return m_streets;
    }

    const PathNetwork::edge_descriptor streetSegment(unsigned idx) {
        return m_streetSegmentVector.at(idx);
    }

    const PathNetwork::vertex_descriptor intersection(unsigned idx) {
        if (idx >= num_vertices(m_roadNetwork))
            std::out_of_range("StreetsDatabase: intersectionID");
        return PathNetwork::vertex_descriptor(idx);
    }

    const POI& poi(unsigned poiID) const {
        return m_pois.at(poiID);
    }

    unsigned getNumberOfPOIs() const {
        return m_pois.size();
    }

    const Feature& feature(unsigned featureID) const {
        return m_features.at(featureID);
    }

    unsigned getNumberOfFeatures() const {
        return m_features.size();
    }

    void pois(std::vector<POI>&& p) {
        m_pois = std::move(p);
    }

    void features(std::vector<Feature>&& f) {
        m_features = std::move(f);
    }

private:

    // the road network
    PathNetwork m_roadNetwork; // connectivity graph for roads
    std::vector<std::string> m_streets; // street names (not necessarily unique)
    std::vector<PathNetwork::edge_descriptor> m_streetSegmentVector; // map streetSeg index -> graph edge

    void buildStreetSegmentVector();

    // points of interest
    std::vector<POI> m_pois;

    // features
    std::vector<Feature> m_features;

    // serialization support

    template<class Archive>void serialize(Archive& ar, const unsigned) {
        ar & m_roadNetwork & m_streets & m_pois & m_features;
        buildStreetSegmentVector();
    }
    friend boost::serialization::access;
};


#endif /* STREETSDATABASE_H_ */
