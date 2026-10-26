# Findbgfx.cmake - CMake module to find bgfx library
# This module is provided as a fallback if bgfx is not found via add_subdirectory

# This file is part of the RenderBridge project.
# Copyright (c) 2024 RenderBridge Contributors
# Licensed under MIT License

find_path(BGFX_INCLUDE_DIR
    NAMES bgfx/bgfx.h
    PATHS
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bgfx/include
        ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/bgfx/include
        /usr/local/include
        /usr/include
)

find_library(BGFX_LIBRARY
    NAMES bgfx
    PATHS
        ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/bgfx/.libs
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bgfx/.libs
        /usr/local/lib
        /usr/lib
)

find_library(BGFX_DEBUG_LIBRARY
    NAMES bgfxd bgfx_debug
    PATHS
        ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/bgfx/.libs
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/bgfx/.libs
        /usr/local/lib
        /usr/lib
)

if(BGFX_INCLUDE_DIR AND (BGFX_LIBRARY OR BGFX_DEBUG_LIBRARY))
    set(BGFX_FOUND TRUE)
    
    if(NOT BGFX_FIND_QUIETLY)
        message(STATUS "Found bgfx:")
        message(STATUS "  Include dir: ${BGFX_INCLUDE_DIR}")
        message(STATUS "  Library: ${BGFX_LIBRARY}")
        message(STATUS "  Debug library: ${BGFX_DEBUG_LIBRARY}")
    endif()
    
    # Set up imported target
    if(NOT TARGET bgfx)
        add_library(bgfx UNKNOWN IMPORTED)
        
        if(BGFX_LIBRARY AND BGFX_DEBUG_LIBRARY)
            set_target_properties(bgfx PROPERTIES
                IMPORTED_LOCATION "${BGFX_LIBRARY}"
                IMPORTED_LOCATION_DEBUG "${BGFX_DEBUG_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${BGFX_INCLUDE_DIR}"
            )
        elseif(BGFX_LIBRARY)
            set_target_properties(bgfx PROPERTIES
                IMPORTED_LOCATION "${BGFX_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${BGFX_INCLUDE_DIR}"
            )
        endif()
    endif()
else()
    set(BGFX_FOUND FALSE)
    
    if(BGFX_FIND_REQUIRED)
        message(FATAL_ERROR "bgfx not found. Please ensure bgfx is built or use add_subdirectory(3rdparty/bgfx)")
    endif()
endif()
