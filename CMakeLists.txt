cmake_minimum_required(VERSION 3.15)
project(byps VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Find dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Optional: Boost (try to find it, but don't require it)
find_package(Boost 1.70 COMPONENTS system)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cpp/include)
include_directories(${OPENSSL_INCLUDE_DIR})

# Collect all source files
file(GLOB_RECURSE CORE_SOURCES
    cpp/src/common/*.cpp
    cpp/src/core/*.cpp
    cpp/src/techniques/*.cpp
    cpp/src/waf/*.cpp
    cpp/src/network/*.cpp
    cpp/src/scanner/*.cpp
)

# Create core library
add_library(byps_core SHARED ${CORE_SOURCES} cpp/lib/byps_core.cpp)

# Link libraries
target_link_libraries(byps_core 
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(Boost_FOUND)
    target_include_directories(byps_core PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(byps_core ${Boost_LIBRARIES})
    target_compile_definitions(byps_core PRIVATE HAVE_BOOST)
endif()

# Set library properties
set_target_properties(byps_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER cpp/include/byps.h
)

# Install targets
install(TARGETS byps_core
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

# Testing
enable_testing()
add_subdirectory(tests/cpp)

# Export compile commands for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
