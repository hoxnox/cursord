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

set(CURSORD2_VERSION_MAJOR 2)
set(CURSORD2_VERSION_MINOR 1)
# configuration header
configure_file(
	"${PROJECT_SOURCE_DIR}/cursord.cfg"
	"${PROJECT_SOURCE_DIR}/src/cursordconf.h"
	ESCAPE_QUOTES
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include_directories("${PROJECT_SOURCE_DIR}/src")

###########################################################################
# event

find_package(Event)
if(NOT Event_FOUND)
	message(FATAL_ERROR "Can't find libevent")
endif(NOT Event_FOUND)
include_directories(Event_INCLUDE_DIR)

###########################################################################
# tclap

find_package(TCLAP)
if(NOT Event_FOUND)
	message(FATAL_ERROR "Can't find tclap")
endif(NOT Event_FOUND)
include_directories(TCLAP_INCLUDE_DIR)

##########################################################################
# glog

if( NOT WITHOUT_GLOG )
	find_package( Glog )
	if( NOT GLOG_FOUND )
		message(STATUS "Glog not found, WITHOUT_GLOG had been set.")
		set(WITHOUT_GLOG ON)
	endif()
endif()

if( WITHOUT_GLOG )
	add_definitions("-DCFG_WITHOUT_GLOG")
else()
	include_directories(${GLOG_INCLUDE_DIRS})
	list(APPEND LIBRARIES ${GLOG_LIBRARIES})
endif()

##########################################################################

set(SRC cursord.cpp)
add_executable(cursord ${SRC})
target_link_libraries(cursord ${Event_LIBRARIES} ${GLOG_LIBRARIES})
