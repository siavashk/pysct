file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB HXX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.hxx)
list(APPEND HEADER_FILES ${HXX_FILES})

add_library(propseg STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(propseg PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(propseg ${VTK_LIBRARIES} ${ITK_LIBRARIES} util alglib)
vtk_module_autoinit(TARGETS propseg MODULES ${VTK_LIBRARIES})