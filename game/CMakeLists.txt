project(so8
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX C)

add_executable(${PROJECT_NAME} "src/main.cxx")

target_link_libraries(${PROJECT_NAME} PUBLIC lune-static)