# Local options (should move out)
SET(BOOST_ROOT /sw/opt/boost-1_58)
SET(Boost_LIB_DIRS /sw/opt/boost-1_58/lib)
SET(BOOST_LIB_SUFFIX -1_58)
SET(XercesC_LIB_DIR /sw/lib)


# (end local options)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(CMAKE_CXX_STANDARD 11)

# -Wno-unused-local-typedef			clang complains about a whole load of (harmless) unused typedefs in BOOST_STATIC_ASSERT
# 
SET (CXX_FLAGS -Wall -Wno-unused-local-typedef)

FIND_PACKAGE(Boost REQUIRED)

if (Boost_FOUND)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
	LINK_DIRECTORIES(${Boost_LIB_DIRS})
endif()

FIND_PACKAGE(xercesc REQUIRED)

if (XercesC_FOUND)
	INCLUDE_DIRECTORIES(${XercesC_INCLUDE_DIR})
	LINK_DIRECTORIES(${XercesC_LIB_DIR})
endif()

ADD_EXECUTABLE(osm2bin osm2bin.cpp OSMElementHandler.cpp SAX2AttributeHandler.cpp SAX2ElementHandler.cpp OSMTagHandler.cpp OSMDatabase.cpp OSMNode.cpp)
TARGET_LINK_LIBRARIES(osm2bin bz2 boost_iostreams${BOOST_LIB_SUFFIX} boost_serialization${BOOST_LIB_SUFFIX} boost_system${BOOST_LIB_SUFFIX} boost_timer${BOOST_LIB_SUFFIX} xerces-c)