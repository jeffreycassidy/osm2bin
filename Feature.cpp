/*
 * Feature.cpp
 *
 *  Created on: Dec 31, 2015
 *      Author: jcassidy
 */

#include "Feature.h"
#include <string>
#include <vector>

using namespace std;

// needs to be kept in sync with enum defs for FeatureTypes
const vector<string> featureNames{
    "<unknown-feature-type>",
    "park",
    "beach",
    "lake",
    "river",
    "island",
    "shoreline",
    "building",
    "greenspace",
    "golf course"
};

const string& asString(FeatureType t) {
    return featureNames.at((unsigned) t);
}


