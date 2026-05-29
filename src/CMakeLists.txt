cmake_minimum_required(VERSION 3.15)

# Collect sources automatically from this directory
file(GLOB_RECURSE DB_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(db ${DB_SOURCES})

# Include both project root and include/ so existing includes resolve as-is
target_include_directories(db PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(db PRIVATE cxx_std_17)

# Useful warnings
if(MSVC)
    target_compile_options(db PRIVATE /W4 /permissive-)
else()
    target_compile_options(db PRIVATE -Wall -Wextra -Wpedantic)
endif()
