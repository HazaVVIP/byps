cmake_minimum_required(VERSION 3.15)

# Find GoogleTest
find_package(GTest)

if(NOT GTest_FOUND)
    message(STATUS "GoogleTest not found, skipping C++ tests")
    return()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../cpp/include)

# Test sources
add_executable(byps_tests
    test_main.cpp
    test_path_bypass.cpp
    test_utils.cpp
)

target_link_libraries(byps_tests
    byps_core
    GTest::GTest
    GTest::Main
)

# Add tests
add_test(NAME byps_unit_tests COMMAND byps_tests)
