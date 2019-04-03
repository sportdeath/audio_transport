# Find audiorw
#
#  AUDIORW_INCLUDE    - Where to find audiorw.hpp
#  AUDIORW_LIBRARY    - The audiorw library
#  AUDIORW_FOUND      - True iff audiorw found.

if(AUDIORW_INCLUDE)
  # Already in cache, be silent!
  set(AUDIORW_FIND_QUIETLY TRUE)
endif(AUDIORW_INCLUDE)

find_path(AUDIORW_INCLUDE audiorw.hpp)

find_library(AUDIORW_LIBRARY NAMES audiorw)

# Handle the QUIETLY and REQUIRED arguments
# and set AUDIORW_FOUND to TRUE if all listed
# variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AUDIORW DEFAULT_MSG AUDIORW_LIBRARY AUDIORW_INCLUDE)

mark_as_advanced(AUDIORW_LIBRARY AUDIORW_INCLUDE)
