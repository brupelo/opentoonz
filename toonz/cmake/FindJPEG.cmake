# looks for libjpeg
find_path(JPEG_INCLUDE_DIR NAMES jpeglib.h HINTS ${THIRDPARTY_LIBS_HINTS} PATH_SUFFIXES jpeg/8d/include LibJPEG/jpeg-9/ NO_DEFAULT_PATH)
find_library(JPEG_LIBRARY NAMES libjpeg.a LibJPEG-9_64.lib HINTS ${THIRDPARTY_LIBS_HINTS} PATH_SUFFIXES jpeg/8d/lib/ LibJPEG/jpeg-9/lib NO_DEFAULT_PATH)

message("***** libjpeg Header path:" ${JPEG_INCLUDE_DIR})
message("***** libjpeg Libarary path:" ${JPEG_LIBRARY})

set(JPEG_NAMES ${JPEG_NAMES} JPEG)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(JPEG DEFAULT_MSG JPEG_LIBRARY JPEG_INCLUDE_DIR)

if(JPEG_FOUND)
    set(JPEG_LIBRARIES ${JPEG_LIBRARY})
endif()

mark_as_advanced(JPEG_LIBRARY JPEG_INCLUDE_DIR)
