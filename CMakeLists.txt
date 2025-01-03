cmake_minimum_required(VERSION 3.20.0)

project(cpp-cmake-template
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

# Setup global project options and variables
include(cmake/ProjectOptions.cmake)

# Do not allow to build in root directory
include(cmake/PreventInSourceBuilds.cmake)

add_executable(${PROJECT_NAME} "src/main.cxx")
# add_library(${PROJECT_NAME} STATIC "src/main.cxx")

# Optional clang-tidy support
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}")
endif()

# Optional sanitizers
include(cmake/Sanitizers.cmake)
enable_sanitizers(${PROJECT_NAME}
                    ${ENABLE_SANITIZER_ADDRESS}
                    ${ENABLE_SANITIZER_LEAK}
                    ${ENABLE_SANITIZER_UNDEFINED}
                    ${ENABLE_SANITIZER_THREAD}
                    ${ENABLE_SANITIZER_MEMORY})