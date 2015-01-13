PROJECT(OpenDaed)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# common depends and definitions
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")

OPTION(BUG2BUG "Enable bug-to-bug compatibility (useless really)" OFF)

# depends
IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/SDL2pp/CMakeLists.txt)
	MESSAGE(FATAL_ERROR "The source directory\n  ${PROJECT_SOURCE_DIR}/extlibs/SDL2pp\ndoes not contain a CMakeLists.txt file.\nIt is likely that you forgot to run\n  git submodule init && git submodule update")
ENDIF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/SDL2pp/CMakeLists.txt)

SET(SDL2PP_WITH_IMAGE TRUE)
ADD_SUBDIRECTORY(extlibs/SDL2pp)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
FIND_PACKAGE(QuickTime REQUIRED)

# sources
SET(OPENDAED_SOURCES
	artemispuzzle.cc
	datamanager.cc
	gameinterface.cc
	hexagonspuzzle.cc
	hotfile.cc
	interpreter.cc
	main.cc
	movplayer.cc
	nodfile.cc
	quicktime.cc
	screen.cc
	sunpuzzle.cc
)

SET(OPENDAED_HEADERS
	artemispuzzle.hh
	datamanager.hh
	gameeventlistener.hh
	gameinterface.hh
	hexagonspuzzle.hh
	hotfile.hh
	interpreter.hh
	logger.hh
	movplayer.hh
	nodfile.hh
	quicktime.hh
	screen.hh
	sunpuzzle.hh
)

# binary
IF(BUG2BUG)
	ADD_DEFINITIONS(-DBUG2BUG)
ENDIF(BUG2BUG)

INCLUDE_DIRECTORIES(${SDL2PP_INCLUDE_DIRS} ${QUICKTIME_INCLUDE_DIR})
ADD_EXECUTABLE(opendaed ${OPENDAED_SOURCES} ${OPENDAED_HEADERS})
TARGET_LINK_LIBRARIES(opendaed ${SDL2PP_LIBRARIES} ${QUICKTIME_LIBRARY})
