cmake_minimum_required(VERSION 3.19)
project(CourseTorrent)

set(CMAKE_CXX_STANDARD 20)

add_executable(CourseTorrent main.cpp)

target_link_libraries(CourseTorrent  -lssl)
target_link_libraries(CourseTorrent  -lcrypto)
target_link_libraries(CourseTorrent -lcurl)
target_link_libraries(CourseTorrent -lpthread)
find_package(Boost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
target_link_libraries(CourseTorrent ${Boost_LIBRARIES})