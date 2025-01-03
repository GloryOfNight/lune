project(space-outpost-8
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

add_executable(${PROJECT_NAME} "src/main.cxx")

target_link_libraries(${PROJECT_NAME} PRIVATE lune-static)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}")
endif()