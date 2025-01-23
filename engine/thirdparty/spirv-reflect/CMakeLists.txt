project(spirv-reflect)

# 1.3.296.0
add_library(spirv-reflect-static STATIC "${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.h" "${CMAKE_CURRENT_SOURCE_DIR}/spirv_reflect.c")
set_target_properties(spirv-reflect-static PROPERTIES   C_STANDARD 17
                                                        CXX_STANDARD 20
                                                        CXX_STANDARD_REQUIRED ON
                                                        EXPORT_COMPILE_COMMANDS ON
                                                        BUILD_TYPE RelWithDebInfo
                                                        POSITION_INDEPENDENT_CODE ON)

target_include_directories(spirv-reflect-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})