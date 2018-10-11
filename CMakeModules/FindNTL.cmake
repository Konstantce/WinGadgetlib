# Locate NTL
# This module defines
# NTL_LIBRARY
# NTL_FOUND, if false, do not try to link to NTL
# NTL_INCLUDE_DIR, where to find the headers

#as hint it may use NTL_ROOT
set(NTL_ROOT "" CACHE STRING "Root location of NTL library")

# Include dir
find_path(NTL_INCLUDE_DIR
  NAMES "NTL"
  PATHS ${NTL_ROOT}
)

# Finally the library itself
find_library(NTL_LIBRARY
  NAMES NTL
  PATHS ${NTL_ROOT}
)

if(NOT ${NTL_INCLUDE_DIR} STREQUAL "NTL_INCLUDE_DIR-NOTFOUND" AND
   NOT ${NTL_LIBRARY} STREQUAL "NTL_LIBRARY-NOTFOUND")
    set(NTL_FOUND TRUE)
endif()

MARK_AS_ADVANCED(NTL_ROOT NTL_LIBRARY NTL_INCLUDE_DIR)