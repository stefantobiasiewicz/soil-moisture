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

if(CONFIG_BT)
    target_sources(app PRIVATE  src/conectivity/ble.c)   
endif()
