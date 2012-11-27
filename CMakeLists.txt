# @author $username$ <$usermail$>
# @date $date$
# @copyright $username$

cmake_minimum_required(VERSION 2.6)

project(cursord2)

###############################################################################
# OPTIONS

option(WITHOUT_GLOG "Don't use google logging even it is in the system" OFF)

###############################################################################

include("${PROJECT_SOURCE_DIR}/cmake/ext/nx_utils.cmake")
nx_configure()

set(CURSORD_VERSION_MAJOR 2)
set(CURSORD_VERSION_MINOR 1)
# configuration header
configure_file(
	"${PROJECT_SOURCE_DIR}/cursord.cfg"
	"${PROJECT_SOURCE_DIR}/src/cursordconf.h"
	ESCAPE_QUOTES
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/facet")

###########################################################################
# tclap

find_package(TCLAP REQUIRED)
include_directories(${TCLAP_INCLUDE_DIR})

##########################################################################
# glog

if( NOT WITHOUT_GLOG )
	find_package( Glog )
	if(NOT GLOG_FOUND)
		message(STATUS "Glog not found, WITHOUT_GLOG had been set.")
		set(WITHOUT_GLOG ON)
	endif(NOT GLOG_FOUND)
endif()

if( WITHOUT_GLOG )
	add_definitions("-DCFG_WITHOUT_GLOG")
else()
	include_directories(${GLOG_INCLUDE_DIRS})
	list(APPEND LIBRARIES ${GLOG_LIBRARIES})
endif()

##########################################################################
# nx_socket

set(NX_SOCK_SRC ./src/nx_socket.h ./src/nx_socket.c)
add_library(nx_socket STATIC ${NX_SOCK_SRC})
list(APPEND LIBRARIES nx_socket)

##########################################################################
# cursord

set(SRC ./src/cursord.cpp
	./src/string.hpp
	./src/string.cpp
	./src/cursor.hpp
	./src/cursor.cpp
	./src/cursor_generator.hpp
	./src/cursor_generator.cpp
	./src/cursor_file.hpp
	./src/cursor_file.cpp
	./src/cursor_odbc.hpp
	./src/cursor_odbc.cpp)
add_executable(cursord ${SRC})
target_link_libraries(cursord ${LIBRARIES})
set_target_properties(cursord PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

##########################################################################
# client

set(SRC ./src/cursor-cli.cpp
	./src/string.hpp
	./src/string.cpp)
add_executable(cursor-cli ${SRC})
target_link_libraries(cursor-cli ${LIBRARIES})
set_target_properties(cursor-cli PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

