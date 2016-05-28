# looks for libpng
find_path(PNG_INCLUDE_DIR NAMES pngconf.h HINTS ${THIRDPARTY_LIBS_HINTS} PATH_SUFFIXES libpng/1.6.13/include libpng-1.6.21/ NO_DEFAULT_PATH)
find_library(PNG_LIBRARY NAMES libpng.a HINTS ${THIRDPARTY_LIBS_HINTS} PATH_SUFFIXES libpng/1.6.13/lib libpng-1.6.21/lib/ NO_DEFAULT_PATH)

message("***** libpng Header path:" ${PNG_INCLUDE_DIR})
message("***** libpng Libarary path:" ${PNG_LIBRARY})

set(PNG_NAMES ${PNG_NAMES} PNG)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PNG DEFAULT_MSG PNG_LIBRARY PNG_INCLUDE_DIR)

if(PNG_FOUND)
    set(PNG_LIBRARIES ${PNG_LIBRARY})
endif()

mark_as_advanced(PNG_LIBRARY PNG_INCLUDE_DIR)
