cmake_minimum_required (VERSION 3.11)
project ("Raytracer")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Paths
set(PROJ_INCL_DIR "${CMAKE_SOURCE_DIR}/include")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/dependencies")
set(GLAD_DIR "${DEPS_DIR}/glad")
set(GLFW_DIR "${DEPS_DIR}/glfw")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

# Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(Raytracer ${SOURCES})

target_compile_definitions(Raytracer PRIVATE ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

# Miniz
add_library(miniz STATIC
    "${CMAKE_SOURCE_DIR}/external/miniz/miniz.c"
    "${CMAKE_SOURCE_DIR}/external/miniz/miniz_tdef.c"
    "${CMAKE_SOURCE_DIR}/external/miniz/miniz_tinfl.c"
    "${CMAKE_SOURCE_DIR}/external/miniz/miniz_zip.c"
)

target_include_directories(miniz PUBLIC
    "${CMAKE_SOURCE_DIR}/external/miniz"
)

# Nlohmann_json
add_subdirectory(external/nlohmann_json)
target_link_libraries(Raytracer PRIVATE nlohmann_json::nlohmann_json)


# GLM
add_subdirectory(${EXTERNAL_DIR}/glm)

# OpenGL
find_package(OpenGL REQUIRED)

# GLAD
add_library(glad ${GLAD_DIR}/glad.c)
target_include_directories(glad PUBLIC "${GLAD_DIR}/include")

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS	OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS		OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL		OFF CACHE BOOL "" FORCE)
add_subdirectory(${GLFW_DIR})

# ImGui
set(IMGUI_DIR "${EXTERNAL_DIR}/imgui")

add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)

# we use GLAD as OpenGL-Loader
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# ImGui needs GLFW + OpenGL
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# Link executable
target_link_libraries(Raytracer PRIVATE glad glfw OpenGL::GL imgui miniz)

# Include directories
target_include_directories(Raytracer PRIVATE "${CMAKE_SOURCE_DIR}/src" "${GLAD_DIR}/include" "${PROJ_INCL_DIR}" "${EXTERNAL_DIR}" "${CMAKE_SOURCE_DIR}/external/miniz")

# Copy assets to output folder
add_custom_target(sync_assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets
		${CMAKE_BINARY_DIR}/assets)
add_dependencies(Raytracer sync_assets)

# Copy shaders to output folder
add_custom_target(sync_shaders
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/shaders
		${CMAKE_BINARY_DIR}/shaders)
add_dependencies(Raytracer sync_shaders)

