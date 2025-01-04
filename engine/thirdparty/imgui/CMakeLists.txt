project(lune-imgui)
set(IMGUI_DIR "1.91.6")

add_library(imgui-static STATIC 
${IMGUI_DIR}/imgui.cpp 
${IMGUI_DIR}/imgui_demo.cpp 
${IMGUI_DIR}/imgui_draw.cpp 
${IMGUI_DIR}/imgui_tables.cpp 
${IMGUI_DIR}/imgui_widgets.cpp 
${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp 
${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui-static PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui-static PRIVATE SDL3::SDL3-static Vulkan::Vulkan)

add_library(ImGui::ImGui-static ALIAS imgui-static)

unset(IMGUI_DIR)