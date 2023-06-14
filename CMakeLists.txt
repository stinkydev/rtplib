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

set(BUILD_MinimalCppSocket_SAMPLES OFF)
add_subdirectory(${EXTERNAL_DIR}/minimal-socket)
add_subdirectory(${EXTERNAL_DIR}/opus-1.4)

message (STATUS "FFMPEG_DIR: " ${MAIN_SRC_DIR})

set(LIB_SOURCES 
  src/h264.h
  src/rtp-header.h
  src/rtp-header.cc
  src/rtp-session.cc
  src/rtp-stream.cc
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


set (BUILD_TEST_PROJECT ON)
if (BUILD_TEST_PROJECT)

add_library(libavformat STATIC IMPORTED)
add_library(libavutil STATIC IMPORTED)
add_library(libavcodec STATIC IMPORTED)
add_library(libavdevice STATIC IMPORTED)
add_library(libswresample STATIC IMPORTED)
add_library(libswscale STATIC IMPORTED)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  #target_compile_options(myApp PRIVATE /W4)
  set_property(TARGET libavformat PROPERTY IMPORTED_LOCATION ${EXTERNAL_DIR}/ffmpeg/lib/x64/avformat.lib)
  set_property(TARGET libavutil PROPERTY IMPORTED_LOCATION ${EXTERNAL_DIR}/ffmpeg/lib/x64/avutil.lib)
  set_property(TARGET libavcodec PROPERTY IMPORTED_LOCATION ${EXTERNAL_DIR}/ffmpeg/lib/x64/avcodec.lib)
  set_property(TARGET libavdevice PROPERTY IMPORTED_LOCATION ${EXTERNAL_DIR}/ffmpeg/lib/x64/avdevice.lib)
  set_property(TARGET libswresample PROPERTY IMPORTED_LOCATION ${EXTERNAL_DIR}/ffmpeg/lib/x64/swresample.lib)
  set_property(TARGET libswscale PROPERTY IMPORTED_LOCATION ${EXTERNAL_DIR}/ffmpeg/lib/x64/swscale.lib)
  Set(FFMPEG_DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External/FFmpeg/lib/x64/)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  #target_compile_options(myApp PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # other macOS-specific flags for Clang
endif()

set(TESTPROJECT_NAME ${PROJECT_NAME}_test)
add_executable(${TESTPROJECT_NAME}
  src/rtptest/main.cc
  src/rtptest/utils.h
  src/rtptest/testing/ffmpeg-encoder.h
  src/rtptest/testing/ffmpeg-reader.h
  src/rtptest/testing/opus-encoder.h
)
target_link_libraries(${TESTPROJECT_NAME}
  PRIVATE 
  ${PROJECT_NAME}
  libavformat
  libavutil
  libavcodec
  libavdevice
  libswresample
  libswscale
  opus
)

target_include_directories(${TESTPROJECT_NAME}
  SYSTEM PRIVATE
  
  ${EXTERNAL_DIR}/FFmpeg/include
  ${EXTERNAL_DIR}/opus-1.4/include
)
endif()

