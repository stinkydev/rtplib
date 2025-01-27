cmake_minimum_required(VERSION 3.20) # FetchContent is available in 3.11+

project(rtplib VERSION 0.0.1)

#build compiler tool
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(MAIN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

# config IDE ext 
include(CTest)
enable_testing()

option(BUILD_MinimalCppSocket_SAMPLES "" OFF)
add_subdirectory(${EXTERNAL_DIR}/minimal-socket)

message (STATUS "FFMPEG_DIR: " ${MAIN_SRC_DIR})

set(LIB_SOURCES 
  src/h264.h
  src/rtp-session.cc
  src/rtp-stream.cc
  src/rtp-socket.cc
  src/rtcp-packets.cc
  src/rtcp.cc
  src/clock.cc
)

file(GLOB_RECURSE INCLUDE_FILES ${MAIN_INCLUDE_DIR}*.h)

add_library(${PROJECT_NAME} STATIC
  ${INCLUDE_FILES}
  ${LIB_SOURCES}
)

add_dependencies(${PROJECT_NAME} MinimalSocket)

target_include_directories(${PROJECT_NAME}
  PUBLIC 
  ${MAIN_INCLUDE_DIR}
  ${EXTERNAL_DIR}/minimal-socket/src/header
)

#================================================
# LIBRARIES
#================================================
#target_link_libraries(${PROJECT_NAME} SDL2main SDL2-static)
target_link_libraries(${PROJECT_NAME} 
  PRIVATE 
  ${CPP_LIBS}
  MinimalSocket
)
