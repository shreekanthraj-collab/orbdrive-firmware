cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/../products/common
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(orbdrive_fault_manager_test)