# Local options (should move out)
SET(BOOST_LIB_SUFFIX -1_58 CACHE STRING "Optional suffix for boost libraries in the -l argument")
# (end local options)

MESSAGE("CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}")

# GCC and clang both complain about missing braces despite the syntax being OK (-Wno-missing-braces)
# some versions of clang also complain about unused local typedefs in BOOST_CONCEPT_CHECK
# GCC and other clang versions don't have the option and so warn about the flag being invalid
ADD_DEFINITIONS(-Wall -Wno-missing-braces -Wno-unknown-warning-option -Wno-unused-local-typedef)

FIND_PACKAGE(Boost REQUIRED COMPONENTS serialization system iostreams)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})




CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(CMAKE_CXX_STANDARD 11)

# -Wno-unused-local-typedef			clang complains about a whole load of (harmless) unused typedefs in BOOST_STATIC_ASSERT
#SET (CMAKE_CXX_FLAGS "-Wall -std=c++11 -g" CACHE STRING "")

# XercesC find module
FIND_PACKAGE(XercesC REQUIRED)

LINK_DIRECTORIES(${XercesC_LIB_DIRS})
INCLUDE_DIRECTORIES(${XercesC_INCLUDE_DIRS})

## OSM parser (used when dealing with plain .osm files)
ADD_LIBRARY(OSMParser SHARED ParseOSM.cpp LoadOSM.cpp ParseXML.cpp OSMElementHandler.cpp SAX2AttributeHandler.cpp SAX2ElementHandler.cpp OSMTagHandler.cpp)
TARGET_LINK_LIBRARIES(OSMParser ${XercesC_LIBRARIES} boost_iostreams${BOOST_LIB_SUFFIX} boost_serialization${BOOST_LIB_SUFFIX} boost_system${BOOST_LIB_SUFFIX})

# Add BZip2 if present, otherwise disable
FIND_PACKAGE(BZip2)
if (BZIP2_FOUND)
	TARGET_LINK_LIBRARIES(OSMParser bz2)
else()
	ADD_DEFINITIONS(-DNO_BZIP2)
endif()



## Layer-1 OSM database (can be generated from .osm files or loaded from .osm.bin files)
ADD_LIBRARY(OSMDatabase SHARED OSMDatabase.cpp OSMNode.cpp PathNetwork.cpp OSMDatabaseAPI.cpp BasicWayFeatureFactory.cpp BasicRelationFeatureFactory.cpp  FeatureFactory.cpp Feature.cpp Compass.cpp MultipolyCloser.cpp)
TARGET_LINK_LIBRARIES(OSMDatabase boost_iostreams${BOOST_LIB_SUFFIX} boost_serialization${BOOST_LIB_SUFFIX} boost_system${BOOST_LIB_SUFFIX})

## Layer-2 Streets database
ADD_LIBRARY(StreetsDatabase SHARED StreetsDatabaseAPI.cpp StreetsDatabase.cpp Feature.cpp)
TARGET_LINK_LIBRARIES(StreetsDatabase boost_serialization${BOOST_LIB_SUFFIX})

## osm (and .osm.gz / .osm.bz2) -> .osm.bin converter
ADD_EXECUTABLE(osm2bin osm2bin.cpp)
TARGET_LINK_LIBRARIES(osm2bin OSMDatabase StreetsDatabase boost_timer${BOOST_LIB_SUFFIX} OSMParser)

## street/intersection explorer (lists in text output)
ADD_EXECUTABLE(explorer explorer.cpp)
TARGET_LINK_LIBRARIES(explorer StreetsDatabase OSMDatabase)

SET(GRAPHICS ON CACHE BOOL "Enable Cairo graphics")

if (GRAPHICS)
    FIND_PACKAGE(GTK2 COMPONENTS gtk gtkmm)

    MESSAGE("Found GTK2 Cairo (${GTK2_CAIRO_INCLUDE_DIR} and Fontconfig ${GTK2_FONTCONFIG_INCLUDE_DIR}")
	SET (CAIRO_LIBRARIES ${GTK2_CAIRO} ${GTK2_CAIROMM})
	INCLUDE_DIRECTORIES(${GTK2_CAIRO_INCLUDE_DIR} ${GTK2_CAIROMM_INCLUDE_DIR} ${GTK2_FONTCONFIG_INCLUDE_DIR} ${GTK2_SIGC++CONFIG_INCLUDE_DIR} ${GTK2_SIGC++_INCLUDE_DIR})
	LINK_DIRECTORIES(${GTK2_CAIRO_LINK_DIR} ${GTK2_CAIROMM_LINK_DIR})
	SET (CAIRO_FOUND BOOL ON)


	FIND_PACKAGE(Freetype REQUIRED)
	INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIR_ft2build})
	INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIR_freetype2})
	MESSAGE("-- Found Freetype at ${FREETYPE_INCLUDE_DIR_ft2build}")
	
	INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})
	
	## draw lake outline	
	ADD_EXECUTABLE(lake lake.cpp)
	TARGET_LINK_LIBRARIES(lake OSMDatabase OSMParser ${CAIRO_LIBRARIES} ${CAIROMM_LIBRARIES} cairo cairomm-1.0)
	
endif()

SET (MAPS_PATH /home/jcassidy/src/ECE297/maps CACHE PATH "")

function (MAKE_BIN_TARGET fnroot)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${fnroot}.osm.bin ${fnroot}.streets.bin
		COMMAND ./osm2bin ${MAPS_PATH}/${fnroot}.osm.gz ${fnroot}
		DEPENDS ./osm2bin ${MAPS_PATH}/${fnroot}.osm.gz
		WORKING_DIRECTORY .)
	ADD_CUSTOM_TARGET(
		${fnroot}_bin DEPENDS ${fnroot}.osm.bin ${fnroot}.streets.bin)
endfunction()

function (MAKE_WATER_TARGET fnroot)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${fnroot}.water.pdf
		COMMAND ./lake ${fnroot}.osm.bin ${fnroot}.water.pdf
		DEPENDS ./lake ${fnroot}.osm.bin
		WORKING_DIRECTORY .)
	ADD_CUSTOM_TARGET(
    	${fnroot}_water DEPENDS ${fnroot}.water.pdf)
endfunction()

SET (MAPS barcelona dresden_germany newyork hamilton_canada moscow saint_helena toronto cairo_egypt bejaia basel_switzerland)

ADD_CUSTOM_TARGET(waters)
ADD_CUSTOM_TARGET(bins)

ADD_CUSTOM_TARGET(mapsdir COMMAND ln -sF ${MAPS_PATH} maps)

FOREACH(map ${MAPS})
    MAKE_BIN_TARGET(${map})
	MAKE_WATER_TARGET(${map})
	ADD_DEPENDENCIES(waters ${map}_water)
    ADD_DEPENDENCIES(bins ${map}_bin)
ENDFOREACH()
