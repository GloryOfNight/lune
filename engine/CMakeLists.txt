project(lune
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

add_subdirectory(thirdparty)

add_library(${PROJECT_NAME}-static STATIC 
                                        "include/lune/lune.hxx" 
                                        "include/lune/engine.hxx"
                                        "src/lune/lune.cxx"
                                        "src/lune/engine.cxx"
                                        )

target_include_directories(${PROJECT_NAME}-static PUBLIC include)
target_include_directories(${PROJECT_NAME}-static PRIVATE include/lune)

target_link_libraries(${PROJECT_NAME}-static PUBLIC Vulkan::Vulkan SDL3::SDL3-static SDL3_image::SDL3_image-static ImGui::ImGui-static)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
        target_compile_definitions(${PROJECT_NAME}-static PRIVATE SDL_DISABLE_ANALYZE_MACROS)
        set_target_properties(${PROJECT_NAME}-static PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}")
endif()