project(png_add_inserts)
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
aux_source_directory(. SRC_LIST)
find_package(PNG REQUIRED)
message(STATUS "LibPNG package was found with include dir: '${PNG_INCLUDE_DIRS}'")
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})

