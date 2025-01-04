cmake_minimum_required(VERSION 3.20.0)
project(lune-project)

# Setup global project options and variables
include(cmake/ProjectOptions.cmake)
# Do not allow to build in root directory
include(cmake/PreventInSourceBuilds.cmake)
# Optional sanitizers
include(cmake/Sanitizers.cmake)

add_subdirectory(engine)
add_subdirectory(game)

