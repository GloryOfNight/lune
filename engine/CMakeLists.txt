project(lune
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

file(GLOB_RECURSE LUNE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/lune/*.hxx)
file(GLOB_RECURSE LUNE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/lune/*.cxx)

add_library(${PROJECT_NAME}-static STATIC ${LUNE_HEADER_FILES} ${LUNE_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}-static PUBLIC        "include")

# vulkan #
find_package(Vulkan)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}-static PUBLIC spirv-reflect-static Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
# vulkan end #

# modules #
add_subdirectory(modules)
target_link_libraries(${PROJECT_NAME}-static PUBLIC Lune::ShaderCompilerTool)
# modules end#

# other packages #
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME}-static PUBLIC TinyGltf::TinyGltf-static glm::glm SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)
# other packages end #

# build-in thirdparty #
add_subdirectory(thirdparty)
target_link_libraries(${PROJECT_NAME}-static PUBLIC ImGui::ImGui-static)
# build-in thirdparty end #