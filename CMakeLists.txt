cmake_minimum_required(VERSION 3.20) # FetchContent is available in 3.11+

project(rtplib VERSION 0.0.1)

#build compiler tool
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(MAIN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)

# config IDE ext 
include(CTest)
enable_testing()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  #target_compile_options(myApp PRIVATE /W4)
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


file(GLOB_RECURSE CPP_HEADER_FILES ${MAIN_SRC_DIR}/*.h)
file(GLOB_RECURSE CPP_SOURCE_FILES ${MAIN_SRC_DIR}/*.cc)

#================================================
# INCLUDE DIR FOLDERS
#================================================

#================================================
# EXECUTE APP
#================================================
#add_executable(${PROJECT_NAME} ${ENTRYPOINT})
add_executable(${PROJECT_NAME}
  ${ENTRYPOINT} 
  ${CPP_SOURCE_FILES}
  ${CPP_HEADER_FILES}
)

#================================================
# INCLUDE DIRS
#================================================
target_include_directories(${PROJECT_NAME} 
  SYSTEM PRIVATE 
  ${CPP_INCLUDE_DIRS}
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
