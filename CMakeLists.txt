cmake_minimum_required(VERSION 3.22)
project(APC_System VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Global definitions for all subdirectories
set(JUCE_USE_WIN_WEBVIEW2 ON CACHE BOOL "" FORCE)
add_definitions(-DJUCE_USE_WIN_WEBVIEW2=1)

message(STATUS "--- APC BUILD SYSTEM DIAGNOSTICS ---")
message(STATUS "Root Directory: ${CMAKE_CURRENT_SOURCE_DIR}")

# 1. External Tools
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_tools/JUCE")
set(VISAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_tools/visage")

if(NOT EXISTS "${JUCE_DIR}")
    message(FATAL_ERROR "JUCE missing at ${JUCE_DIR}")
endif()
add_subdirectory("${JUCE_DIR}")

# Visage is only needed for Visage UI framework plugins
# if(NOT EXISTS "${VISAGE_DIR}")
#     message(FATAL_ERROR "Visage missing at ${VISAGE_DIR}")
# endif()
# add_subdirectory("${VISAGE_DIR}")

# if(TARGET visage AND NOT TARGET visage::visage)
#     add_library(visage::visage ALIAS visage)
# endif()

# 2. Plugin Discovery (Debug Mode)
message(STATUS "Scanning for plugins in: ${CMAKE_CURRENT_SOURCE_DIR}/plugins")

file(GLOB PLUGIN_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/plugins/*")

foreach(PLUGIN_DIR ${PLUGIN_DIRS})
    get_filename_component(DIR_NAME ${PLUGIN_DIR} NAME)
    message(STATUS "Checking folder: ${DIR_NAME}")
    
    if(IS_DIRECTORY "${PLUGIN_DIR}")
        if(EXISTS "${PLUGIN_DIR}/CMakeLists.txt")
            message(STATUS " -> FOUND VALID PLUGIN: ${DIR_NAME} (Adding subdirectory...)")
            add_subdirectory("${PLUGIN_DIR}")
        else()
            message(STATUS " -> SKIPPING: ${DIR_NAME} (No CMakeLists.txt found)")
        endif()
    else()
        message(STATUS " -> SKIPPING: ${DIR_NAME} (Not a directory)")
    endif()
endforeach()

message(STATUS "--- END DIAGNOSTICS ---")