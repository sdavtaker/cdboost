cmake_minimum_required(VERSION 3.28)

# Toolchain must be set before project() so CMake loads vcpkg during compiler detection.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

project(cdboost VERSION 0.3.1 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS rational)
find_package(spdlog CONFIG REQUIRED)

add_library(cdboost INTERFACE)
target_include_directories(cdboost INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(cdboost INTERFACE cxx_std_23)
target_link_libraries(cdboost INTERFACE Boost::boost spdlog::spdlog)
# Warnings apply only when building from source tree, not when installed via vcpkg.
target_compile_options(cdboost INTERFACE
    $<BUILD_INTERFACE:-Wall -Wextra -pedantic>
)

# Default to ON only when this is the top-level project; OFF when consumed via add_subdirectory.
option(CDBOOST_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})
option(CDBOOST_BUILD_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})

if(CDBOOST_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(CDBOOST_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_SOURCE_FILES
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/example/*.cpp"
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCE_FILES}
        COMMENT "Applying clang-format"
    )
    add_custom_target(check-format
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCE_FILES}
        COMMENT "Checking clang-format"
    )
endif()
