PROJECT(OpenDaed)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# common depends and definitions
ADD_DEFINITIONS("-std=c++11 -Wall -Wextra -pedantic")

# depends
IF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/SDL2pp/CMakeLists.txt)
	MESSAGE(FATAL_ERROR "The source directory\n  ${PROJECT_SOURCE_DIR}/extlibs/SDL2pp\ndoes not contain a CMakeLists.txt file.\nIt is likely that you forgot to run\n  git submodule init && git submodule update")
ENDIF(NOT EXISTS ${PROJECT_SOURCE_DIR}/extlibs/SDL2pp/CMakeLists.txt)
ADD_SUBDIRECTORY(extlibs/SDL2pp)

# sources
SET(OPENDAED_SOURCES
	main.cc
)

# binary
INCLUDE_DIRECTORIES(${SDL2PP_INCLUDE_DIRS})
ADD_EXECUTABLE(opendaed ${OPENDAED_SOURCES})
TARGET_LINK_LIBRARIES(opendaed ${SDL2PP_LIBRARIES})
