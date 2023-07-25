cmake_minimum_required(VERSION 3.12)
project(Hooks VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED NO)
set(CMAKE_CXX_EXTENSIONS YES)

# Add path for custom modules
list(INSERT CMAKE_MODULE_PATH 0
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT WIN32)
    message(FATAL_ERROR "Only support Windows.")
endif ()

# Use relative paths
if (WIN32)
    set(CMAKE_USE_RELATIVE_PATHS TRUE)
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
endif ()

# Use folders to organize targets in an IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as no build type was specified")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Debug/Release)" FORCE)
endif ()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE PATH "The root directory of the installation" FORCE)
    message(STATUS "Setting default install directory to ${CMAKE_INSTALL_PREFIX} as no install directory was specified")
endif ()

# Disable msvc unsafe warnings
if (MSVC)
    add_compile_definitions(
            $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
            $<$<C_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_WARNINGS>
    )
endif ()

find_package(VirtoolsSDK REQUIRED)

add_subdirectory(deps)

configure_file(Hooks.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Hooks.h)
configure_file(Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.h)

set(HOOKS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(HOOKS_SOURCES
        Plugin.cpp
        HookBlock.cpp
        HookManager.cpp
        HookLoader.cpp
        HookModule.cpp
        HookApi.cpp
        Utils.cpp

        HookManager.h
        HookLoader.h
        HookModule.h
        Utils.h
        Hooks.h

        Plugin.rc
        )

add_library(Hooks SHARED ${HOOKS_SOURCES})
target_include_directories(Hooks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Hooks PRIVATE CK2 VxMath minhook cjson)

add_subdirectory(test)