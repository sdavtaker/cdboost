cmake_minimum_required(VERSION 3.28)

# Toolchain must be set before project() so CMake loads vcpkg during compiler detection.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

project(cdboost VERSION 0.3.2 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS rational)
find_package(spdlog CONFIG REQUIRED)

add_library(cdboost INTERFACE)
add_library(cdboost::cdboost ALIAS cdboost)
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

# ============================================================================
# Installation
# ============================================================================

include(GNUInstallDirs)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/cdboost/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cdboost
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    TARGETS cdboost
    EXPORT cdboostTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT cdboostTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cdboost
    NAMESPACE cdboost::
    FILE cdboostTargets.cmake
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cdboostConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cdboostConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cdboost
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cdboostConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cdboostConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cdboostConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cdboost
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
