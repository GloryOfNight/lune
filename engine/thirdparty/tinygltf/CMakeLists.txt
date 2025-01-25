
project(spirv-reflect)

set(TINYGLTF_DIR "2.9.5")

add_library(tinygltf-static STATIC "${CMAKE_CURRENT_LIST_DIR}/${TINYGLTF_DIR}/src/tiny_gltf.cc")

set_target_properties(tinygltf-static PROPERTIES    C_STANDARD 17
                                                    CXX_STANDARD 20
                                                    CXX_STANDARD_REQUIRED ON
                                                    EXPORT_COMPILE_COMMANDS ON
                                                    BUILD_TYPE RelWithDebInfo
                                                    POSITION_INDEPENDENT_CODE ON)

target_include_directories(tinygltf-static PUBLIC "${CMAKE_CURRENT_LIST_DIR}/${TINYGLTF_DIR}/include")
target_include_directories(tinygltf-static PRIVATE "${CMAKE_CURRENT_LIST_DIR}/${TINYGLTF_DIR}/src" "${CMAKE_CURRENT_LIST_DIR}/${TINYGLTF_DIR}/include/tinygltf")

target_compile_definitions(tinygltf-static PUBLIC TINYGLTF_NO_STB_IMAGE TINYGLTF_NO_STB_IMAGE_WRITE)

add_library(TinyGltf::TinyGltf-static ALIAS tinygltf-static)