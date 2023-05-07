add_library(TestModule SHARED TestModule.cpp)
target_include_directories(TestModule PRIVATE ${HOOKS_INCLUDE_DIR})
target_link_libraries(TestModule PRIVATE CK2 VxMath)
