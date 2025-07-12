find_path(EMBREE_INCLUDE_DIR
    NAMES embree4/rtcore.h
    PATHS ${EMBREE_ROOT_DIR}/include
    NO_DEFAULT_PATH
)

find_library(EMBREE_LIBRARY
    NAMES embree4
    PATHS ${EMBREE_ROOT_DIR}/lib
    NO_DEFAULT_PATH
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Embree
    REQUIRED_VARS EMBREE_LIBRARY EMBREE_INCLUDE_DIR
)

if(EMBREE_FOUND)
    set(EMBREE_INCLUDE_DIRS ${EMBREE_INCLUDE_DIR})
    set(EMBREE_LIBRARIES ${EMBREE_LIBRARY})
endif()

mark_as_advanced(EMBREE_INCLUDE_DIR EMBREE_LIBRARY)