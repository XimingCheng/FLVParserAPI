cmake_minimum_required(VERSION 2.8)

project (FLVParserAPI)

add_definitions(
    -W
    -O2
    -g
)

OPTION(DEBUG "Build project using debug mode" ON)

if (DEBUG)
        SET(CMAKE_BUILD_TYPE Debug)
else (DEBUG)
        SET(CMAKE_BUILD_TYPE Release)
endif (DEBUG)

message("Build type is " ${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -g")
set(CMAKE_CXX_FLAGS_DEBUG   "-D_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

subdirs(
    api
    test
)