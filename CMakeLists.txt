# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c 
                src/firmware/hardware.c 
                src/firmware/flash.c 
                src/validation.c
                src/display.c
                src/logic_control.c
            )

            
target_sources(app PRIVATE  src/conectivity/ble.c)   


add_subdirectory(driver/EPD)

# Wyklucz oryginalny plik
set_source_files_properties(${ZEPHYR_BASE}/modules/lvgl/lvgl_display.c,
                            PROPERTIES HEADER_FILE_ONLY ON)

# add_subdirectory(lvgl)
# target_compile_definitions(main PRIVATE ZEPHYR_BASE)
