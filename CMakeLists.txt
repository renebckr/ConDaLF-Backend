cmake_minimum_required(VERSION 3.7.0)
project(ConDaLF-Backend LANGUAGES C CXX VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/;${CMAKE_MODULE_PATH}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/apps/ConDaLF-Backend)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Get CPM
set(CPM_DOWNLOAD_VERSION 0.27.2)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif()
include(${CPM_DOWNLOAD_LOCATION})

# Include Dependencies

# libcoap -> CoAP
CPMAddPackage(
    NAME LIBCOAP
    GITHUB_REPOSITORY obgm/libcoap
    GIT_TAG v4.3.0-rc3
    GIT_SUBMODULES_RECURSE TRUE
    OPTIONS 
    "MBEDTLS_INCLUDE_DIRS " # Because the CMake File is bugged
    "ENABLE_DOCS OFF"

)
include_directories(${LIBCOAP_SOURCE_DIR}/include)
include_directories(${LIBCOAP_SOURCE_DIR}/../libcoap-build/include)
# ----


# TODO: User should be able to decide on not using scripts
# python 3.7
find_package(Python3 3.7 REQUIRED COMPONENTS Development)
include_directories(${Python3_INCLUDE_DIRS})
# ----

include(CTest)
enable_testing()
add_subdirectory(src)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Documentation
find_package(Doxygen OPTIONAL_COMPONENTS dot)
if(DOXYGEN_FOUND)
    add_custom_target(documentation ALL COMMAND ${DOXYGEN_EXECUTABLE} "./Doxyfile"
        WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
        COMMENT "Building user's documentation into doc build dir..."
    )
endif()