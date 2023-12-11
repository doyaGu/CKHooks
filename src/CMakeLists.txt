configure_file(Hooks.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Hooks.h)
configure_file(Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Version.h)

set(HOOKS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HOOKS_HEADERS
        HookManager.h
        HookLoader.h
        HookModule.h
        WindowHook.h
        FileHook.h
        Logger.h
        CallbackList.h
        Utils.h
        Macros.h
        Hooks.h
)

set(HOOKS_SOURCES
        Plugin.cpp
        HookBlock.cpp
        HookManager.cpp
        HookLoader.cpp
        HookModule.cpp
        WindowHook.cpp
        FileHook.cpp
        HookApi.cpp
        Logger.cpp
        Utils.cpp
        Plugin.rc
)

add_library(Hooks SHARED ${HOOKS_SOURCES} ${HOOKS_HEADERS})
target_include_directories(Hooks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Hooks PRIVATE CK2 VxMath minhook yyjson)

set_target_properties(Hooks PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)
