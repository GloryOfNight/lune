project(lune
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

file(GLOB_RECURSE LUNE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/lune/*.hxx)
file(GLOB_RECURSE LUNE_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/lune/*.cxx)

add_library(${PROJECT_NAME}-static STATIC ${LUNE_HEADER_FILES} ${LUNE_SOURCE_FILES})

target_include_directories(${PROJECT_NAME}-static PUBLIC        "include")

set_target_properties(${PROJECT_NAME}-static PROPERTIES         C_STANDARD 17
                                                                CXX_STANDARD 20
                                                                CXX_STANDARD_REQUIRED ON
                                                                EXPORT_COMPILE_COMMANDS ON)

# vulkan #
find_package(Vulkan)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}-static PUBLIC spirv-reflect-static Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
# vulkan end #

# other packages #
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(glm REQUIRED)
target_link_libraries(${PROJECT_NAME}-static PUBLIC glm::glm SDL3::SDL3-static SDL3_image::SDL3_image-static)
# other packages end #

# build-in thirdparty #
add_subdirectory(thirdparty)
target_link_libraries(${PROJECT_NAME}-static PUBLIC ImGui::ImGui-static)
# build-in thirdparty end #

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
        target_compile_definitions(${PROJECT_NAME}-static PRIVATE SDL_DISABLE_ANALYZE_MACROS)
        set_target_properties(${PROJECT_NAME}-static PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}")
endif()