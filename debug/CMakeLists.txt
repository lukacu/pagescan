cmake_minimum_required(VERSION 3.4.1)

PROJECT(pagescan)

set (CMAKE_CXX_STANDARD 11)

SET(NATIVE_DIR ../app/src/main/cpp)

add_executable(
             pagescan
             ${NATIVE_DIR}/detector.cpp
             debug.cpp
             )

FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(AFTER ${NATIVE_DIR} ${OpenCV_INCLUDE_DIR})
LINK_LIBRARIES(${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(pagescan ${OpenCV_LIBS})
