cmake_minimum_required (VERSION 3.8)
project ("Raytracer")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths
set(PROJ_INCL_DIR "${CMAKE_SOURCE_DIR}/include")
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/dependencies")
set(GLAD_DIR "${DEPS_DIR}/glad")
set(GLFW_DIR "${DEPS_DIR}/glfw")
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

# Sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(Raytracer ${SOURCES})

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

# Link executable
target_link_libraries(Raytracer PRIVATE glad glfw OpenGL::GL)

# Include directories
target_include_directories(Raytracer PRIVATE "${CMAKE_SOURCE_DIR}/src" "${GLAD_DIR}/include" "${PROJ_INCL_DIR}" "${EXTERNAL_DIR}")

# Copy assets to output folder
add_custom_target(sync_assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets
		${CMAKE_BINARY_DIR}/assets)
add_dependencies(Raytracer sync_assets)

