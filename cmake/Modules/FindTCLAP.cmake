# Find GLOG
# Merder Kim <hoxnox@gmail.com>
# 
# input:
#  TCLAP_ROOT
#
# output:
#  TCLAP_FOUND
#  TCLAP_INCLUDE_DIRS
#

if(TCLAP_INCLUDE_DIR)
	set(TCALP_FIND_QUITELY TRUE) # cached
endif(TCLAP_INCLUDE_DIR)

if(NOT DEFINED TCLAP_ROOT)
	set(TCLAP_ROOT /usr /usr/local $ENV{TCLAP_ROOT})
endif(NOT DEFINED TCLAP_ROOT)

find_path(TCLAP_INCLUDE_DIR tclap/CmdLine.h
	PATHS ${TCLAP_ROOT}
	PATH_SUFFIXES tclap/include include
)

mark_as_advanced(TCLAP_INCLUDE_DIR)

include("${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCLAP DEFAULT_MSG TCLAP_INCLUDE_DIR)


