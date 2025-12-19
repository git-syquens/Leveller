# Top-level CMakeLists.txt for Leveller Project
cmake_minimum_required(VERSION 3.16)

# Set project name and supported languages
set(PROJECT_NAME "leveller")

# Include ESP-IDF CMake build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the project
project(${PROJECT_NAME})
