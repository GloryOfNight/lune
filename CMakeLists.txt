cmake_minimum_required(VERSION 3.28.0)
project(lune-project)

include(cmake/ProjectDefaults.cmake)
include(cmake/ProjectOptions.cmake)
include(cmake/PreventInSourceBuilds.cmake)
include(cmake/Sanitizers.cmake)

add_subdirectory(engine)
add_subdirectory(game)