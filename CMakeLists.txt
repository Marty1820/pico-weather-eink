cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Include the SDK
include(pico_sdk_import.cmake)

# Define project name
project(pico_test)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executables
add_executable(${PROJECT_NAME}
  main.c
  src/wifi.c
)

pico_set_program_version(${PROJECT_NAME} "0.2")

# Enable USB output so we can see printf
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/config
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${CMAKE_CURRENT_LIST_DIR}/lib
)

# Add any user requested libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
  pico_stdlib
  pico_cyw43_arch_lwip_threadsafe_background
  pico_cyw43_driver
  pico_lwip
)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})
