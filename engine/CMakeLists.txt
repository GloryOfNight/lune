project(lune
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

add_library(${PROJECT_NAME}-static STATIC 
                                        "include/lune.hxx" 
                                        "src/lune.cxx"
                                        )

target_include_directories(${PROJECT_NAME}-static PUBLIC include)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
        set_target_properties(${PROJECT_NAME}-static PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}")
endif()