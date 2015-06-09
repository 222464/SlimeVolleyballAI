cmake_minimum_required(VERSION 2.8)

project(SlimeVolleyballAI)

include_directories("${PROJECT_SOURCE_DIR}/source")

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
 
find_package(SFML 2.3 REQUIRED system window graphics audio network)
 
include_directories(${SFML_INCLUDE_DIR})
 
add_executable(SlimeVolleyballAI "${PROJECT_SOURCE_DIR}/source/Main.cpp")

target_link_libraries(SlimeVolleyballAI ${SFML_LIBRARIES})