cmake_minimum_required(VERSION 3.12)
project(Hooks VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)

if (NOT WIN32)
    message(FATAL_ERROR "Only support Windows.")
endif ()

# Use folders to organize targets in an IDE
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

# Use relative paths
if (WIN32)
    set(CMAKE_USE_RELATIVE_PATHS TRUE)
    set(CMAKE_SUPPRESS_REGENERATION TRUE)
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as no build type was specified")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the build type (Debug/Release)" FORCE)
endif ()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install" CACHE PATH "The root directory of the installation" FORCE)
    message(STATUS "Setting default install directory to ${CMAKE_INSTALL_PREFIX} as no install directory was specified")
endif ()

set(VIRTOOLS_SDK_PATH "${VIRTOOLS_SDK_PATH}" CACHE PATH "Path to the Virtools SDK")
set(VIRTOOLS_SDK_FETCH_FROM_GIT "${VIRTOOLS_SDK_FETCH_FROM_GIT}" CACHE BOOL "Set to ON to fetch copy of SDK from git if not otherwise locatable")
set(VIRTOOLS_SDK_FETCH_FROM_GIT_PATH "${VIRTOOLS_SDK_FETCH_FROM_GIT_PATH}" CACHE FILEPATH "location to download SDK")

if (NOT VIRTOOLS_SDK_PATH)
    if (NOT VIRTOOLS_SDK_FETCH_FROM_GIT)
        message(FATAL_ERROR "Virtools SDK location was not specified. Please set VIRTOOLS_SDK_PATH or set VIRTOOLS_SDK_FETCH_FROM_GIT to on to fetch from git.")
    else ()
        include(FetchContent)
        set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
        if (VIRTOOLS_SDK_FETCH_FROM_GIT_PATH)
            get_filename_component(FETCHCONTENT_BASE_DIR "${VIRTOOLS_SDK_FETCH_FROM_GIT_PATH}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
        endif ()
        FetchContent_Declare(
                Virtools_SDK
                GIT_REPOSITORY https://github.com/doyaGu/Virtools-SDK-2.1.git
                GIT_TAG main
        )
        if (NOT Virtools_SDK)
            message("Downloading Virtools SDK")
            FetchContent_Populate(Virtools_SDK)
            set(VIRTOOLS_SDK_PATH "${virtools_sdk_SOURCE_DIR}" CACHE PATH "Path to the Virtools SDK" FORCE)
        endif ()
        set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})
    endif ()
endif ()

find_package(VirtoolsSDK REQUIRED HINTS ${VIRTOOLS_SDK_PATH})

add_subdirectory(deps)

# Disable msvc unsafe warnings
add_compile_definitions(
        $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        $<$<C_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_WARNINGS>
        $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_DEPRECATE>
        $<$<C_COMPILER_ID:MSVC>:_CRT_NONSTDC_NO_DEPRECATE>
)

configure_file(Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.h)

set(HOOKS_SOURCES
        Plugin.cpp
        HookBlock.cpp
        HookManager.cpp
        HookLoader.cpp
        HookModule.cpp
        MessageHook.cpp
        Utils.cpp

        HookManager.h
        HookLoader.h
        HookModule.h
        MessageHook.h
        Utils.h
        Hooks.h

        Plugin.rc
        )

add_library(Hooks SHARED ${HOOKS_SOURCES})
target_include_directories(Hooks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Hooks PRIVATE CK2 VxMath minhook cjson)

add_library(TestModule SHARED TestModule.cpp)
target_include_directories(TestModule PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(TestModule PRIVATE CK2 VxMath)