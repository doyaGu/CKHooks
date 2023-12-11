add_library(TestModule SHARED TestModule.cpp)
target_include_directories(TestModule PRIVATE ${HOOKS_SOURCE_DIR})
target_link_libraries(TestModule PRIVATE CK2 VxMath)

add_library(ListBuildingBlocks SHARED ListBuildingBlocks.cpp)
target_include_directories(ListBuildingBlocks PRIVATE ${HOOKS_SOURCE_DIR})
target_link_libraries(ListBuildingBlocks PRIVATE CK2 VxMath)