# Find GLOG
# Merder Kim <hoxnox@gmail.com>
# 
# input:
#  GLOG_ROOT
#  GLOG_USE_STATIC_LIBS
#
# output:
#  GLOG_FOUND
#  GLOG_INCLUDE_DIR
#  GLOG_LIBRARIES
#

if(GLOG_INCLUDE_DIR AND GLOG_LIBRARIES)
	set(GLOG_FIND_QUITELY TRUE) # cached
endif(GLOG_INCLUDE_DIR AND GLOG_LIBRARIES)

if(NOT DEFINED GLOG_ROOT)
	set(GLOG_ROOT /usr /usr/local $ENV{GLOG_ROOT})
endif(NOT DEFINED GLOG_ROOT)

find_path(GLOG_INCLUDE_DIR glog/logging.h
	PATHS ${GLOG_ROOT}
	PATH_SUFFIXES glog/include include
)

if( GLOG_USE_STATIC_LIBS )
	set( _glog_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
	if(WIN32)
		set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	else()
		set(CMAKE_FIND_LIBRARY_SUFFIXES .a )
	endif()
endif()

find_library(GLOG_LIBRARIES
	NAMES glog
	PATHS ${GLOG_ROOT}
	PATH_SUFFIXES lib
)
mark_as_advanced(GLOG_INCLUDE_DIR GLOG_LIBRARIES)

# Restore the original find library ordering
if( GLOG_USE_STATIC_LIBS )
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${_glog_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

include("${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Glog DEFAULT_MSG GLOG_INCLUDE_DIR GLOG_LIBRARIES)

