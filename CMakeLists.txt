# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(EspScale)

add_subdirectory(src/scale)
add_subdirectory(src/touch)
add_subdirectory(src/display)

target_sources(app PRIVATE src/main.c)
