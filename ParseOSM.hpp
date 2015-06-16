/*
 * ParseOSM.hpp
 *
 *  Created on: Aug 30, 2015
 *      Author: jcassidy
 */

#ifndef PARSEOSM_HPP_
#define PARSEOSM_HPP_

#include "OSMDatabase.hpp"
#include <xercesc/sax/InputSource.hpp>

OSMDatabase parseOSM(xercesc::InputSource *src);


#endif /* PARSEOSM_HPP_ */
