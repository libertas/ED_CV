
cmake_minimum_required(VERSION 3.5)

project(ED_CV)
add_subdirectory(simcom)
include_directories(${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/simcom)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cpp)

find_package(OpenCV REQUIRED)

add_executable(ed ${SOURCE_FILES})
target_link_libraries(ed ${OpenCV_LIBS} simcom)
