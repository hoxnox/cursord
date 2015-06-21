# @author Merder Kim <hoxnox@gmail.com>
# @date 20131225 15:48:29
# @copyright Merder Kim

cmake_minimum_required(VERSION 2.6)

project(cursord)

###############################################################################
# OPTIONS

option(WITHOUT_GLOG "Don't use google logging even it is in the system" OFF)
option(WITHOUT_IPV4GEN "Don't build ipv4gen tool" ON)
option(WITHOUT_ODBC "Don't build odbc dependent code" OFF)
option(WITH_SYSTEM_NANOMSG "Use system nanomsg" OFF)
option(WITH_expand_ipv4rg "Build expand_ipv4rg utility" OFF)
option(GENERATE_DOC "Don't generate docs" ON)
option(WITH_TESTS "Build tests" OFF)


###############################################################################

include("${PROJECT_SOURCE_DIR}/cmake/ext/nx_utils.cmake")
nx_configure()

set(CURSORD_VERSION_MAJOR 2)
set(CURSORD_VERSION_MINOR 2)
set(CURSORD_VERSION_PATCH 0)
# configuration header
configure_file(
	"${PROJECT_SOURCE_DIR}/src/cursord.cfg"
	"${PROJECT_SOURCE_DIR}/src/cursordconf.h"
	ESCAPE_QUOTES
)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules")
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src/facet")
find_package(Threads REQUIRED)
list(APPEND LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

###########################################################################
# tclap

find_package(TCLAP REQUIRED)
include_directories(${TCLAP_INCLUDE_DIR})

########################################################################
# nanomsg

if (WITH_SYSTEM_NANOMSG)
	# TODO: FindNANOMGS.cmake
	set(NANOMSG_USE_STATIC_LIBS true)
	find_package(NANOMSG REQUIRED)
else()
	include(ExternalProject)
	set(NANOMSG_ROOT "${CMAKE_CURRENT_BINARY_DIR}/nanomsg")
	ExternalProject_Add(nanomsg
		URL "https://github.com/nanomsg/nanomsg/archive/0.5-beta.tar.gz"
		URL_MD5 "918c76cbf2498d5b2947e45207f36fab"
		PATCH_COMMAND <SOURCE_DIR>/autogen.sh
		CONFIGURE_COMMAND ./configure --enable-static --disable-shared --prefix <INSTALL_DIR>
		INSTALL_DIR "${NANOMSG_ROOT}"
		BUILD_IN_SOURCE 1
	)
	set(NANOMSG_INCLUDE_DIR ${NANOMSG_ROOT}/include)
	set(NANOMSG_LIBRARIES ${NANOMSG_ROOT}/lib/libnanomsg.a)
	list(APPEND EXTERNAL_DEPS nanomsg)
endif()
list(APPEND LIBRARIES -lanl ${NANOMSG_LIBRARIES})

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
	./src/ipv4.hpp
	./src/ipv4.cpp
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
add_dependencies(cursord ${EXTERNAL_DEPS})
target_link_libraries(cursord ${LIBRARIES})
set_target_properties(cursord PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

##########################################################################
# client

set(SRC_CLI ./src/cursor-cli.cpp
            ./src/string.hpp
            ./src/string.cpp)
add_executable(cursor-cli ${SRC_CLI})
add_dependencies(cursor-cli ${EXTERNAL_DEPS})
target_link_libraries(cursor-cli ${LIBRARIES})
set_target_properties(cursor-cli PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -std=c++0x -D_GLIBCXX_USE_NANOSLEEP")

###########################################################################
# expand_ipv4rg

if(WITH_expand_ipv4rg)
	set(EXPAND_IPv4RG_SRC 
		./src/string.hpp
		./src/string.cpp
		./src/cursor.hpp
		./src/cursor.cpp
		./src/cursor_file.hpp
		./src/cursor_file.cpp
		./src/ipv4.hpp
		./src/ipv4.cpp
		./src/ipv4_generator.hpp
		./src/ipv4_generator.cpp
		./src/shuffle.hpp
		./src/shuffle.cpp
		./src/speedometer.hpp
		./src/speedometer.cpp
		./misc/expand_ipv4rg.cpp)
	add_executable(expand_ipv4rg ${EXPAND_IPv4RG_SRC})
	target_link_libraries(expand_ipv4rg ${LIBRARIES})
endif()

###########################################################################
# ipv4gen

if(NOT WITHOUT_IPV4GEN)
	set(IPV4GEN_SRC ./misc/ipv4gen.c)
	add_executable(ipv4gen ${IPV4GEN_SRC})
	target_link_libraries(ipv4gen)
endif()

###########################################################################
# ipv4gen_m

if(NOT WITHOUT_IPV4GEN)
	set(IPV4GEN_M_SRC ./misc/ipv4gen_m.cpp 
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

if( WITH_TESTS )
	set(SOURCES_TEST
		./test/tshuffle.hpp
		./src/shuffle.hpp
		./src/shuffle.cpp
		./src/ipv4.cpp
		./src/ipv4.hpp
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
else( WITH_TESTS )
	message(STATUS "Tests disabled. Use -DBUILD_TESTS=TRUE to enable it.")
endif( WITH_TESTS )

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

###########################################################################
# cpack

INSTALL(TARGETS cursord cursor-cli DESTINATION bin)
SET(CPACK_PACKAGE_NAME "cursord")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cursord - is a common data source")
SET(CPACK_PACKAGE_VENDOR "Merder Kim <hoxnox@gmail.com>")
SET(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.markdown")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_RPM_PACKAGE_GROUP "Development/Tools")
SET(CPACK_RPM_PACKAGE_LICENSE "MIT")
SET(CPACK_STRIP_FILES TRUE)
INCLUDE(CPack)

