# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Add Marauder board components (LCD, GUI, Touch)
set(EXTRA_COMPONENT_DIRS "boards/marauder/components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(leveller)
