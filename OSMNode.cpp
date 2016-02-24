/*
 * OSMNode.cpp
 *
 *  Created on: May 10, 2015
 *      Author: jcassidy
 */

#include "OSMNode.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

ostream& operator<<(ostream& os, const OSMNode& n) {
    return os << "Node id " << setw(12) << n.id() << " at coords " << setprecision(6) << n.coords();
}

