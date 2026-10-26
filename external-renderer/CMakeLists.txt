cmake_minimum_required(VERSION 3.15)
project(renderbridge-cpp-renderer LANGUAGES C CXX)

# Project version
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Project configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bgfx/include
    ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/bgfx/include
)

# Options
option(BUILD_TESTS "Build tests" OFF)
option(BGFX_BUILD_EXAMPLES "Build bgfx examples" OFF)
option(BGFX_BUILD_TOOLS "Build bgfx tools" OFF)
option(USE_SHARED_MEMORY "Use shared memory transport" ON)
option(USE_LOCAL_SOCKET "Use local socket transport" ON)

# ============================================================================
# Dependency: bgfx
# ============================================================================

# Add bgfx as subdirectory
add_subdirectory(3rdparty/bgfx)

# Set bgfx options
set(BGFX_BUILD_EXAMPLES ${BGFX_BUILD_EXAMPLES} CACHE BOOL "Build bgfx examples")
set(BGFX_BUILD_TOOLS ${BGFX_BUILD_TOOLS} CACHE BOOL "Build bgfx tools")

# ============================================================================
# Dependency: GLFW
# ============================================================================

# Find GLFW package
find_package(glfw3 3.3 REQUIRED)

if(NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW 3.3 or newer is required. Please install it or use vcpkg/conan.")
endif()

# ============================================================================
# Shader compilation setup
# ============================================================================

# Find Python (required for shaderc)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Shader compiler configuration
set(SHADERC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bgfx/tools/shaderc/shaderc")

# Platform-specific shader compiler
if(WIN32)
    set(SHADERC_EXECUTABLE "${SHADERC_PATH}.exe")
else()
    set(SHADERC_EXECUTABLE "${SHADERC_PATH}")
endif()

# Check if shaderc exists, if not we'll compile it
if(NOT EXISTS "${SHADERC_EXECUTABLE}")
    # Build shaderc from bgfx
    add_subdirectory(3rdparty/bgfx/tools/shaderc)
    set(SHADERC_EXECUTABLE "${CMAKE_BINARY_DIR}/shaderc/shaderc${CMAKE_EXECUTABLE_SUFFIX}")
endif()

# Custom command to compile shaders
function(compile_shader SHADER_NAME)
    set(VS_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER_NAME}.sc")
    set(VS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
    
    # Create output directory
    file(MAKE_DIRECTORY ${VS_OUTPUT_DIR})
    
    # Define output files for different backends
    set(OPENGL_OUTPUT "${VS_OUTPUT_DIR}/${SHADER_NAME}_opengl.bin")
    set(VULKAN_OUTPUT "${VS_OUTPUT_DIR}/${SHADER_NAME}_vulkan.bin")
    set(D3D11_OUTPUT "${VS_OUTPUT_DIR}/${SHADER_NAME}_d3d11.bin")
    set(D3D12_OUTPUT "${VS_OUTPUT_DIR}/${SHADER_NAME}_d3d12.bin")
    set(METAL_OUTPUT "${VS_OUTPUT_DIR}/${SHADER_NAME}_metal.bin")
    
    # Custom commands for each backend
    add_custom_command(
        OUTPUT ${OPENGL_OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${SHADERC_EXECUTABLE} -f ${VS_INPUT} -o ${OPENGL_OUTPUT} --platform windows --type vertex --profile 120
        COMMENT "Compiling ${SHADER_NAME}.sc for OpenGL"
        DEPENDS ${VS_INPUT}
        VERBATIM
    )
    
    add_custom_command(
        OUTPUT ${VULKAN_OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${SHADERC_EXECUTABLE} -f ${VS_INPUT} -o ${VULKAN_OUTPUT} --platform windows --type vertex --profile spirv
        COMMENT "Compiling ${SHADER_NAME}.sc for Vulkan"
        DEPENDS ${VS_INPUT}
        VERBATIM
    )
    
    add_custom_command(
        OUTPUT ${D3D11_OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${SHADERC_EXECUTABLE} -f ${VS_INPUT} -o ${D3D11_OUTPUT} --platform windows --type vertex --profile 50
        COMMENT "Compiling ${SHADER_NAME}.sc for D3D11"
        DEPENDS ${VS_INPUT}
        VERBATIM
    )
    
    add_custom_command(
        OUTPUT ${D3D12_OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${SHADERC_EXECUTABLE} -f ${VS_INPUT} -o ${D3D12_OUTPUT} --platform windows --type vertex --profile 60
        COMMENT "Compiling ${SHADER_NAME}.sc for D3D12"
        DEPENDS ${VS_INPUT}
        VERBATIM
    )
    
    add_custom_command(
        OUTPUT ${METAL_OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${SHADERC_EXECUTABLE} -f ${VS_INPUT} -o ${METAL_OUTPUT} --platform osx --type vertex --profile metal
        COMMENT "Compiling ${SHADER_NAME}.sc for Metal"
        DEPENDS ${VS_INPUT}
        VERBATIM
    )
    
    # Create custom target for shader compilation
    add_custom_target(
        ${SHADER_NAME}_shaders ALL
        DEPENDS ${OPENGL_OUTPUT} ${VULKAN_OUTPUT} ${D3D11_OUTPUT} ${D3D12_OUTPUT} ${METAL_OUTPUT}
    )
    
    # Add output directory to include path for runtime loading
    target_include_directories(${SHADER_NAME}_shaders PRIVATE ${VS_OUTPUT_DIR})
endfunction()

# ============================================================================
# Source files
# ============================================================================

# Main application
set(APP_SOURCES
    src/main.cpp
    src/main.hpp
)

# Window management
set(WINDOW_SOURCES
    src/window.cpp
    src/window.hpp
)

# Input handling
set(INPUT_SOURCES
    src/input_handler.cpp
    src/input_handler.hpp
)

# Bridge client
set(BRIDGE_SOURCES
    src/bridge/renderer_bridge_client.cpp
    src/bridge/renderer_bridge_client.hpp
    src/bridge/bridge_connection.cpp
    src/bridge/bridge_connection.hpp
    src/bridge/message_handler.cpp
    src/bridge/message_handler.hpp
)

# Scene classes
set(SCENE_SOURCES
    src/scene/scene.cpp
    src/scene/scene.hpp
    src/scene/camera.cpp
    src/scene/camera.hpp
    src/scene/chunk.cpp
    src/scene/chunk.hpp
    src/scene/entity.cpp
    src/scene/entity.hpp
    src/scene/particle.cpp
    src/scene/particle.hpp
    src/scene/weather.cpp
    src/scene/weather.hpp
)

# Renderer
set(RENDERER_SOURCES
    src/renderer/renderer.cpp
    src/renderer/renderer.hpp
    src/renderer/voxel_renderer.cpp
    src/renderer/voxel_renderer.hpp
    src/renderer/entity_renderer.cpp
    src/renderer/entity_renderer.hpp
    src/renderer/particle_renderer.cpp
    src/renderer/particle_renderer.hpp
)

# Adapter layer
set(ADAPTER_SOURCES
    src/adapter/protocol_adapter.cpp
    src/adapter/protocol_adapter.hpp
    src/adapter/mesh_builder.cpp
    src/adapter/mesh_builder.hpp
    src/adapter/texture_manager.cpp
    src/adapter/texture_manager.hpp
    src/adapter/uniform_manager.cpp
    src/adapter/uniform_manager.hpp
)

# Utilities
set(UTILS_SOURCES
    src/utils/logger.cpp
    src/utils/logger.hpp
    src/utils/byte_buffer.cpp
    src/utils/byte_buffer.hpp
    src/utils/math_utils.cpp
    src/utils/math_utils.hpp
)

# All source files
set(ALL_SOURCES
    ${APP_SOURCES}
    ${WINDOW_SOURCES}
    ${INPUT_SOURCES}
    ${BRIDGE_SOURCES}
    ${SCENE_SOURCES}
    ${RENDERER_SOURCES}
    ${ADAPTER_SOURCES}
    ${UTILS_SOURCES}
)

# ============================================================================
# Executable target
# ============================================================================

# Create the renderer executable
add_executable(renderer ${ALL_SOURCES})

# Target properties
target_compile_features(renderer PRIVATE cxx_std_17)
set_target_properties(renderer PROPERTIES
    OUTPUT_NAME "renderbridge-renderer"
    VERSION ${PROJECT_VERSION}
    DEBUG_POSTFIX "d"
)

# ============================================================================
# Include directories
# ============================================================================

target_include_directories(renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
    ${glfw3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bgfx/include
    ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/bgfx/include
)

# ============================================================================
# Link libraries
# ============================================================================

# Platform-specific linking
target_link_libraries(renderer PRIVATE
    glfw
    bgfx
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(renderer PRIVATE
        opengl32
        gdi32
        user32
    )
    target_compile_definitions(renderer PRIVATE
        GLFW_EXPOSE_NATIVE_WIN32
        BGFX_CONFIG_RENDERER_OPENGL=1
        BGFX_CONFIG_RENDERER_DIRECT3D11=1
        BGFX_CONFIG_RENDERER_DIRECT3D12=1
    )
elseif(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    find_library(METAL_FRAMEWORK Metal)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)
    
    target_link_libraries(renderer PRIVATE
        ${COCOA_FRAMEWORK}
        ${IOKIT_FRAMEWORK}
        ${COREVIDEO_FRAMEWORK}
        ${METAL_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
    )
    target_compile_definitions(renderer PRIVATE
        GLFW_EXPOSE_NATIVE_COCOA
        BGFX_CONFIG_RENDERER_METAL=1
        BGFX_CONFIG_RENDERER_OPENGL=1
    )
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    find_package(OpenGL REQUIRED)
    
    target_link_libraries(renderer PRIVATE
        ${X11_LIBRARIES}
        ${OPENGL_LIBRARIES}
        GL
        Xcursor
        Xi
    )
    target_compile_definitions(renderer PRIVATE
        GLFW_EXPOSE_NATIVE_X11
        BGFX_CONFIG_RENDERER_OPENGL=1
        BGFX_CONFIG_RENDERER_VULKAN=1
    )
endif()

# ============================================================================
# Compile definitions
# ============================================================================

target_compile_definitions(renderer PRIVATE
    # Project version
    RENDERBRIDGE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    RENDERBRIDGE_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    RENDERBRIDGE_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    
    # Transport options
    ${USE_SHARED_MEMORY}
    ${USE_LOCAL_SOCKET}
    
    # Debug
    $<$<CONFIG:Debug>:
        RENDERBRIDGE_DEBUG=1
        BGFX_CONFIG_DEBUG=1
    >
    
    # Release
    $<$<CONFIG:Release>:
        RENDERBRIDGE_RELEASE=1
        NDEBUG=1
    >
    
    # Platform
    BGFX_CONFIG_RENDERER_OPENGL=1
    BGFX_CONFIG_RENDERER_VULKAN=1
)

# ============================================================================
# Compile options
# ============================================================================

target_compile_options(renderer PRIVATE
    # Warning level
    -Wall
    -Wextra
    -Wpedantic
    
    # Disable specific warnings
    -Wno-unused-function
    -Wno-unused-variable
    
    # Debug info
    $<$<CONFIG:Debug>:
        -g
        -O0
    >
    
    # Release optimization
    $<$<CONFIG:Release>:
        -O3
        -DNDEBUG
    >
)

# ============================================================================
# Shader compilation
# ============================================================================

# Compile all shaders
file(GLOB SHADER_FILES "shaders/*.sc")
foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME_WE)
    compile_shader(${SHADER_NAME})
endforeach()

# Add shader directory as a dependency
target_include_directories(renderer PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_dependencies(renderer shader_compilation)

# ============================================================================
# Post-build: Copy shaders to output directory
# ============================================================================

add_custom_command(TARGET renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_BINARY_DIR}/shaders
        $<TARGET_FILE_DIR:renderer>/shaders
    COMMENT "Copying compiled shaders to output directory"
)

# ============================================================================
# Install targets
# ============================================================================

install(TARGETS renderer
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders/
    DESTINATION bin/shaders
)

# ============================================================================
# Tests (optional)
# ============================================================================

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# Documentation
# ============================================================================

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================================
# Summary
# ============================================================================

message(STATUS "")
message(STATUS "RenderBridge C++ Renderer Configuration")
message(STATUS "========================================")
message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "GLFW found: ${glfw3_FOUND}")
message(STATUS "GLFW version: ${glfw3_VERSION}")
message(STATUS "bgfx directory: ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bgfx")
message(STATUS "Shader compiler: ${SHADERC_EXECUTABLE}")
message(STATUS "")
