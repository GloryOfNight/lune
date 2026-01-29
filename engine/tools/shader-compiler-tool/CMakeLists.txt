# Dreco (and Lune) helper to compile shaders within runtime
# works as stupid as possible, by invoking glslc on outdated shader files

project(shader-compiler-tool)

add_library(${PROJECT_NAME} STATIC src/shader_compiler.cxx)
set_target_properties(${PROJECT_NAME} PROPERTIES    CXX_STANDARD 20
                                                    CXX_STANDARD_REQUIRED ON
                                                    EXPORT_COMPILE_COMMANDS ON
                                                    POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_SHADER_COMPILER_TOOL=1)

add_library(Lune::ShaderCompilerTool ALIAS shader-compiler-tool)