cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/../products/common
    ${CMAKE_CURRENT_LIST_DIR}/../framework/core
${CMAKE_CURRENT_LIST_DIR}/../framework/platform
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(orbdrive_fault_manager_test)
