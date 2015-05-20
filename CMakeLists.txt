cmake_minimum_required (VERSION 2.6)
project(EvGen)

# check for in-source build, forbid it!
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR AND NOT MSVC_IDE)
  message(FATAL_ERROR "\nIn-source build attempt detected!\n"
    "Please create a new directory (e.g. build) and run `cmake ..`.\n"
    "Also don't forget to delete the created CMakeCache.txt and CMakeFiles dir"
    )
endif()
    
include (cmake/settings.cmake)

message(STATUS "*** Build Type:      " ${CMAKE_BUILD_TYPE})
message(STATUS "*** Compiler Flags:  " ${DEFAULT_COMPILE_FLAGS})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/" ${CMAKE_MODULE_PATH})
# require a fairly recent ROOT version >5.30
find_package(ROOT "5.30" REQUIRED)
include_directories(${ROOT_INCLUDES})
link_directories(${ROOT_LIBRARY_DIR})

# create executables

include_directories(inc)

add_executable(EvGen src/EvGen.cxx)
target_link_libraries(EvGen ${ROOT_LIBRARIES})

add_executable(EvGenBasic src/EvGenBasic.cxx)
target_link_libraries(EvGenBasic ${ROOT_LIBRARIES})

add_executable(EvGenRes src/EvGenRes.cxx)
target_link_libraries(EvGenRes ${ROOT_LIBRARIES})
