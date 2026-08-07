# aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.22)
project(gg-sdk-python C)

find_package(Python3 REQUIRED COMPONENTS Development.Module)

# Build the C SDK
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(GG_LOG_LEVEL
    "WARN"
    CACHE STRING "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. sdk EXCLUDE_FROM_ALL)

# Build the Python extension module
python3_add_library(gg_sdk MODULE src/gg_sdk_module.c)
target_link_libraries(gg_sdk PRIVATE gg-sdk)
target_include_directories(gg_sdk
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../priv_include)
target_compile_definitions(gg_sdk PRIVATE _GNU_SOURCE)

install(TARGETS gg_sdk LIBRARY DESTINATION .)
install(FILES gg_sdk.pyi DESTINATION .)
