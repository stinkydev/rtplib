# add_executable(README README.cpp)
# target_link_libraries(README PUBLIC MinimalSocket)	

find_package (Python3 COMPONENTS Interpreter)

include(cmake/MakeSample.cmake)

add_subdirectory(utils)

add_subdirectory(tcp)
add_subdirectory(udp)
