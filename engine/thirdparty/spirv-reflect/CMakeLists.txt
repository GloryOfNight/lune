project(spirv-reflect)

# 1.3.296.0
add_library(spirv-reflect-static STATIC "${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.h" "${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.c")

target_include_directories(spirv-reflect-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})