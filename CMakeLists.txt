
cmake_minimum_required(VERSION 3.5)

project(ED_CV)
add_subdirectory(simcom)
include_directories(${PROJECT_SOURCE_DIR}/ ${PROJECT_SOURCE_DIR}/simcom)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cpp imageProcessing.cpp)

find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

add_executable(ed ${SOURCE_FILES})
target_link_libraries(ed ${OpenCV_LIBS} simcom)
