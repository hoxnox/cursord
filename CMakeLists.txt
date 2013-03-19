# @author $username$ <$usermail$>
# @date $date$
# @copyright $username$

cmake_minimum_required(VERSION 2.6)

project(cursord2)

###############################################################################
# OPTIONS

option(WITHOUT_GLOG "Don't use google logging even it is in the system" OFF)
option(WITHOUT_IPV4GEN "Don't build ipv4gen tool" ON)
option(WITHOUT_ODBC "Don't build odbc dependent code" OFF)
option(GENERATE_DOC "Don't generate docs" ON)


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
	./src/ipv4_generator.hpp
	./src/ipv4_generator.cpp
	./src/cursor_file.hpp
	./src/cursor_file.cpp
	./src/shuffle.hpp
	./src/shuffle.cpp
	./src/speedometer.hpp
	./src/speedometer.cpp)
if(NOT WITHOUT_ODBC)
	list(APPEND SRC
		./src/cursor_odbc.hpp
		./src/cursor_odbc.cpp)
else()
	add_definitions("-DCFG_WITHOUT_ODBC")
endif()
add_executable(cursord ${SRC})
target_link_libraries(cursord ${LIBRARIES})
set_target_properties(cursord PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

##########################################################################
# client

set(SRC_CLI ./src/cursor-cli.cpp
	./src/string.hpp
	./src/string.cpp)
add_executable(cursor-cli ${SRC_CLI})
target_link_libraries(cursor-cli ${LIBRARIES})
set_target_properties(cursor-cli PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

###########################################################################
# ipv4gen

if(NOT WITHOUT_IPV4GEN)
	set(IPV4GEN_SRC ./misc/ipv4gen.c ./src/nx_socket.h)
	add_executable(ipv4gen ${IPV4GEN_SRC})
	target_link_libraries(ipv4gen nx_socket)
endif()

###########################################################################
# ipv4gen_m

if(NOT WITHOUT_IPV4GEN)
	set(IPV4GEN_M_SRC ./misc/ipv4gen_m.cpp 
	                  ./src/nx_socket.h
	                  ./src/ipv4_generator.cpp 
	                  ./src/ipv4_generator.hpp
	                  ./src/shuffle.hpp
	                  ./src/shuffle.cpp)
	add_executable(ipv4gen_m ${IPV4GEN_M_SRC})
	target_link_libraries(ipv4gen_m ${LIBRARIES})
endif()

###########################################################################
# range_cvt

if(NOT WITHOUT_RANGECVT)
	add_executable(range_cvt ./misc/range_cvt.cpp)
endif()

##########################################################################
# tests

if( BUILD_TESTS )
	set(SOURCES_TEST
		./test/tshuffle.hpp
		./src/shuffle.hpp
		./src/shuffle.cpp
		./test/test.cpp
	)
	if(BUILD_GTEST)
		#TODO: Find and check GTEST_SOURCES
		set(GTEST_SOURCES "${PROJECT_SOURCE_DIR}/../gtest")
		enable_testing()
		set(TEST test_${PROJECT_NAME})
				include_directories("${GTEST_SOURCES}/src")
		add_executable(${TEST} ${SOURCES_TEST})
		target_link_libraries("${TEST}" gtest ${LIBRARIES})
		nx_GTEST_ADD_TESTS("${TEST}" ${SOURCES_TEST})
	else(BUILD_GTEST)
		enable_testing()
		find_package(GTest REQUIRED)
		include_directories(${GTEST_INCLUDE_DIRS})
		set(TEST test_${PROJECT_NAME})
		add_executable(${TEST} ${SOURCES_TEST})
		if(NOT WIN32)
			SET_TARGET_PROPERTIES(${TEST} PROPERTIES LINK_FLAGS -pthread)
		endif(NOT WIN32)
		target_link_libraries("${TEST}" ${GTEST_LIBRARIES} ${LIBRARIES} )
		nx_GTEST_ADD_TESTS("${TEST}" ${SOURCES_TEST})
	endif(BUILD_GTEST)
	set_target_properties("${TEST}" PROPERTIES
		COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x")
else( BUILD_TESTS )
	message(STATUS "Tests disabled. Use -DBUILD_TESTS=TRUE to enable it.")
endif( BUILD_TESTS )

###########################################################################
# docs

if(GENERATE_DOC)
	FIND_PACKAGE(Doxygen)
	if(DOXYGEN_FOUND)
		set(ENV{CURSORD_ROOT} ${PROJECT_SOURCE_DIR})
		ADD_CUSTOM_TARGET(doc COMMAND ${CMAKE_COMMAND} 
			-D PDIR="${PROJECT_SOURCE_DIR}"
			-P "${PROJECT_SOURCE_DIR}/doc/doxygen.cmake")
	else()
		message(WARNING "Doxygen not found. Documentation wouldn't be ganarated")
	endif()
endif()
