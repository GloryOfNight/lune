
project(entt)

set(EnTT_DIR "3.16.0")

add_library(entt INTERFACE)

target_include_directories(entt INTERFACE "${CMAKE_CURRENT_LIST_DIR}/${EnTT_DIR}")

add_library(EnTT::EnTT ALIAS entt)