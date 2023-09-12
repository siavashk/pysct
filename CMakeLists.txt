project(isct_propseg)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "DEBUG" FORCE)

option(BUILD_SHARED_LIBS ON)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)

add_subdirectory(util)
add_subdirectory(alglib)
add_subdirectory(propseg)

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(isct_propseg propseg)