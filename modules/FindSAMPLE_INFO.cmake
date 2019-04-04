# Find sample_info 
#
#  SAMPLE_INFO_INCLUDE    - Where to find sample_info headers
#  SAMPLE_INFO_LIBRARY    - The sample_info library
#  SAMPLE_INFO_FOUND      - True iff sample_info found.

if(SAMPLE_INFO_INCLUDE)
  # Already in cache, be silent!
  set(SAMPLE_INFO_FIND_QUIETLY TRUE)
endif(SAMPLE_INFO_INCLUDE)

find_path(SAMPLE_INFO_INCLUDE sample_info/spectral.hpp)

find_library(SAMPLE_INFO_LIBRARY NAMES sample_info)

# Handle the QUIETLY and REQUIRED arguments
# and set SAMPLE_INFO_FOUND to TRUE if all listed
# variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SAMPLE_INFO DEFAULT_MSG SAMPLE_INFO_LIBRARY SAMPLE_INFO_INCLUDE)

mark_as_advanced(SAMPLE_INFO_LIBRARY SAMPLE_INFO_INCLUDE)
