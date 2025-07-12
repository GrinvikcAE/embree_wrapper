cmake_minimum_required(VERSION 3.12)
project(embree_wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fvisibility=default)
add_compile_options(-fPIC)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/embree_wrapper)
set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/output)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/embree_wrapper)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})


find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

set(EMBREE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/embree_binary/linux/embree-4.4.0.x86_64.linux)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/embree_wrapper/cmake")
find_package(Embree REQUIRED)

pybind11_add_module(embree_wrapper
    ${SOURCE_DIR}/core.cpp
)

target_compile_features(embree_wrapper PRIVATE cxx_std_17)
target_compile_options(embree_wrapper PRIVATE
    -fvisibility=default
    -fPIC
)

set_target_properties(embree_wrapper PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    OUTPUT_NAME "embree_wrapper"
    PREFIX ""
    SUFFIX ".so"
    CXX_VISIBILITY_PRESET "default"
    VISIBILITY_INLINES_HIDDEN OFF
)

target_include_directories(embree_wrapper PRIVATE
    ${EMBREE_INCLUDE_DIRS}
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(embree_wrapper PRIVATE
    ${EMBREE_LIBRARIES}
)