SET(CAIRO_FOUND 1 BOOL)

SET(CAIROMM_INCLUDE_DIR /usr/include/cairomm-1.0 CACHE PATH "")

SET(SigC++_FOUND 1)
SET(SigC++_INCLUDE_DIR /usr/lib/x86_64-linux-gnu/sigc++-2.0/include /usr/include/sigc++-2.0)
SET(SigC++_LIB_DIR /usr/lib/x86_64-linux-gnu)
SET(SigC++_INCLUDE_DIR /sw/include/sigc++-2.0 /sw/lib/sigc++-2.0/include)
SET(SigC++_LIB_DIR /sw/lib/sigc++-2.0)



if (${SigC++_FOUND})
	INCLUDE_DIRECTORIES(${SigC++_INCLUDE_DIR})
	LINK_DIRECTORIES(${SigC++_LIB_DIR})
endif()


SET(CAIRO_LIB_DIRS /sw/lib CACHE PATH "Location of libcairo")
SET(CAIRO_INCLUDE_DIR /sw/include)
SET(CAIRO_LIBRARIES "cairo" CACHE STRING "-l libs for cairo")

if (NOT X11_FOUND)
	FIND_PACKAGE(X11)
endif()


SET(CAIROMM_FOUND 1 BOOL)

if (CAIROMM_FOUND)
	MESSAGE("  Including cairomm")
	SET(CAIROMM_LIB_DIR /sw/lib CACHE PATH "Location of libcairomm")
	SET(CAIROMM_INCLUDE_DIR /sw/include/cairomm-1.0 CACHE PATH "Location of cairomm/ folder")
	SET(CAIROMM_LIBRARIES cairomm-1.0 CACHE STRING "-l<lib> for cairomm")
	
	SET(CAIRO_LIBRARIES ${CAIRO_LIBRARIES} ${CAIROMM_LIBRARIES})
	SET(CAIRO_INCLUDE_DIR ${CAIRO_INCLUDE_DIR} ${CAIROMM_INCLUDE_DIR} ${CAIRO_INCLUDE_DIR}/cairo)
endif()

if (X11_FOUND)
	SET(CAIRO_INCLUDE_DIR ${CAIRO_INCLUDE_DIR} ${X11_INCLUDE_DIR})
endif()