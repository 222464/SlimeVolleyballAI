cmake_minimum_required(VERSION 2.8)

project(SlimeVolleyballAI)
project(MyBot)

# Compiler-specific flags and definitions
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
endif()

include_directories("${PROJECT_SOURCE_DIR}/source")

# This is only required for the script to work in the version control
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
 
find_package(SFML 2.3 REQUIRED system window graphics network audio)
 
include_directories(${SFML_INCLUDE_DIR})
 
#set(SOURCE_PATH "${PROJECT_SOURCE_DIR}/source")
#set(SOURCES
#    "${SOURCE_PATH}/SlimeVolleyballAI.cpp"
#)

#add_library(SlimeVolleyballAI STATIC ${SOURCES})

add_executable(SlimeVolleyballAI "${PROJECT_SOURCE_DIR}/source/SlimeVolleyballAI.cpp")
add_executable(MyBot "${PROJECT_SOURCE_DIR}/source/MyBot.cpp")

target_link_libraries(SlimeVolleyballAI ${SFML_LIBRARIES})
target_link_libraries(MyBot ${SFML_LIBRARIES})