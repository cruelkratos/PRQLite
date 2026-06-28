cmake_minimum_required(VERSION 3.15)
project(DB LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Make build multi-config friendly
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(src)

add_subdirectory(bloom_filter)
target_link_libraries(db PRIVATE BloomFilter)

target_include_directories(db PRIVATE 
    bloom_filter/include
)


