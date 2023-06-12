cmake_minimum_required(VERSION 3.20) # FetchContent is available in 3.11+

project(rtplib VERSION 0.0.1)

#build compiler tool
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/out)

# config IDE ext 
include(CTest)
enable_testing()

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

set(BUILD_MinimalCppSocket_SAMPLES OFF)
include(FetchContent)
FetchContent_Declare(
min_sock
GIT_REPOSITORY https://github.com/andreacasalino/Minimal-Socket
GIT_TAG        master

)
FetchContent_MakeAvailable(min_sock)

add_subdirectory(${EXTERNAL_DIR}/opus-1.4)

file(GLOB_RECURSE CPP_HEADER_FILES ${MAIN_SRC_DIR}/*.h)
file(GLOB_RECURSE CPP_SOURCE_FILES ${MAIN_SRC_DIR}/*.cc)

add_executable(${PROJECT_NAME}
  ${ENTRYPOINT} 
  ${CPP_SOURCE_FILES}
  ${CPP_HEADER_FILES}
)

add_dependencies(${PROJECT_NAME} opus)

target_include_directories(${PROJECT_NAME} 
  SYSTEM PRIVATE 
  ${CPP_INCLUDE_DIRS}
  ${EXTERNAL_DIR}/FFmpeg/include
  ${EXTERNAL_DIR}/opus-1.4/include
)

#================================================
# LIBRARIES
#================================================
#target_link_libraries(${PROJECT_NAME} SDL2main SDL2-static)
target_link_libraries(${PROJECT_NAME} 
  PRIVATE 
  ${CPP_LIBS}
  MinimalSocket
  libavformat
  libavutil
  libavcodec
  libavdevice
  libswresample
  libswscale
  opus
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD  COMMAND ${CMAKE_COMMAND} -E copy_directory ${FFMPEG_DLL_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/)
