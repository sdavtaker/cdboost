cmake_minimum_required(VERSION 3.25)
project(cdboost VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_library(cdboost INTERFACE)
target_include_directories(cdboost INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(cdboost INTERFACE ${Boost_INCLUDE_DIRS})
target_compile_options(cdboost INTERFACE -pedantic)

option(CDBOOST_BUILD_TESTS "Build tests" ON)
option(CDBOOST_BUILD_EXAMPLES "Build examples" ON)

if(CDBOOST_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(CDBOOST_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
