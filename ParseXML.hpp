/*
 * ParseXML.hpp
 *
 *  Created on: Aug 30, 2015
 *      Author: jcassidy
 */

#ifndef PARSEXML_HPP_
#define PARSEXML_HPP_

#include <xercesc/sax/InputSource.hpp>
#include "SAX2ElementHandler.hpp"

#include <xercesc/sax2/XMLReaderFactory.hpp>

void parseXML(xercesc::InputSource* src,SAX2ContentHandler* handler);

#endif /* PARSEXML_HPP_ */
