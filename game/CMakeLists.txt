project(so8
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

add_executable(${PROJECT_NAME} "src/main.cxx")

target_link_libraries(${PROJECT_NAME} PUBLIC lune-static)

set_target_properties(${PROJECT_NAME} PROPERTIES        C_STANDARD 17
                                                        CXX_STANDARD 20
                                                        CXX_STANDARD_REQUIRED ON
                                                        EXPORT_COMPILE_COMMANDS ON)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}")
endif()