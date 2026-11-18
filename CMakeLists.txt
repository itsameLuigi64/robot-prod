# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)
                                                                                
set(CMAKE_C_STANDARD 11)                                                        
set(CMAKE_CXX_STANDARD 17)                                                      
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

if(WIN32)                                                                       
    set(USERHOME $ENV{USERPROFILE})                                             
else()                                                                          
    set(USERHOME $ENV{HOME})                                                    
endif() 

set(sdkVersion 2.2.0)                                                           
set(toolchainVersion 13_3_Rel1)                                                 
set(picotoolVersion 2.2.0)                                                      
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)                   
if (EXISTS ${picoVscode})                                                       
    include(${picoVscode})                                                      
endif()  

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(robot C CXX ASM)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

#set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

#include(example_auto_set_url.cmake)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

if (TARGET tinyusb_device)
	add_executable(robot
   	     robot.c
		 glcdfont.c
		 Adafruit_4383.c
		 graphics.c
		 syscalls.c
		 tft_stdout.c
		 bootrom_api.c
		 sensors-tft.c
		 motors.c
   	     )
	
	# pull in common dependencies
	target_link_libraries(
		robot
		pico_stdlib
		hardware_spi
		hardware_gpio
		hardware_pwm
		hardware_adc
		cmsis_core
		pico_multicore
	)
	pico_enable_stdio_usb(robot 1)
	pico_enable_stdio_uart(robot 0)
	
	# create map/bin/hex file etc.
	pico_add_extra_outputs(robot)
	
	# add url via pico_set_program_url
#	example_auto_set_url(robot)
elseif(PICO_ON_DEVICE)
	message(WARNING "not building hello_usb because TinyUSB submodule is not initialized in the SDK")
endif()
