# Find Libevent
# Merder Kim <hoxnox@gmail.com>
#
#  input:
#  Event_ROOT
#
#  output:
#  Event_FOUND
#  Event_INCLUDE_DIR
#  Event_LIBRARIES
#

if(Event_INCLUDE_DIR AND Event_LIBRARIES)
	set(Event_FIND_QUIETLY TRUE) # cached
endif(Event_INCLUDE_DIR AND Event_LIBRARIES)

if(NOT DEFINED Event_ROOT)
	set (Event_ROOT /usr /usr/local $ENV{Event_ROOT} )
endif(NOT DEFINED Event_ROOT)

find_path(Event_INCLUDE_DIR event.h
	PATHS ${Event_ROOT}
	PATH_SUFFIXES event/include include
)

find_library(EventCore_LIBRARY
	NAMES event_core
	PATHS ${Event_ROOT}
	PATH_SUFFIXES lib
)

set(Event_LIBRARIES ${EventCore_LIBRARY})

add_definitions(-DLIBNET_LIL_ENDIAN)

mark_as_advanced(Event_INCLUDE_DIR Event_LIBRARIES)

include("${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Event DEFAULT_MSG Event_INCLUDE_DIR Event_LIBRARIES)

set(Event_FOUND ${EVENT_FOUND})

