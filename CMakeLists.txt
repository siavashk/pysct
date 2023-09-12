project(isct_propseg)
cmake_minimum_required(VERSION 3.16)

find_package( PkgConfig )

set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "DEBUG" FORCE)

option(BUILD_SHARED_LIBS ON)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)

add_subdirectory(extern/pybind11)
add_subdirectory(util)
add_subdirectory(alglib)
add_subdirectory(propseg)

pybind11_add_module(pysct pysct.cpp)
target_link_libraries(pysct PRIVATE propseg)

# add_executable(${PROJECT_NAME} Main.cpp ${HEADER_FILES})
# target_link_libraries(isct_propseg propseg)