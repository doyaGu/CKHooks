# MinHook
add_subdirectory(minhook EXCLUDE_FROM_ALL)
set_target_properties(minhook PROPERTIES FOLDER "Dependencies")

# cJSON
set(CJSON_BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ENABLE_CJSON_TEST OFF CACHE BOOL "")
add_subdirectory(cJSON EXCLUDE_FROM_ALL)
target_include_directories(cjson INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cJSON>)
target_compile_definitions(cjson PRIVATE "CJSON_HIDE_SYMBOLS")
set_target_properties(cjson PROPERTIES FOLDER "Dependencies")