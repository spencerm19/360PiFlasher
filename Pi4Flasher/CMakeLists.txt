cmake_minimum_required(VERSION 3.10)
project(Pi4Flasher C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# Source files
set(SOURCES
    src/main.c
    src/pi4_gpio.c
    src/pi4_spi.c
    src/spiex.c
    src/xbox.c
)

# Executable
add_executable(pi4flasher ${SOURCES})

# Find bcm2835 library
find_library(BCM2835_LIB bcm2835)
if(NOT BCM2835_LIB)
    message(FATAL_ERROR "bcm2835 library not found. Please install libbcm2835-dev")
endif()

# Link libraries
target_link_libraries(pi4flasher ${BCM2835_LIB})

# Include directories
target_include_directories(pi4flasher PRIVATE src)

# Install target
install(TARGETS pi4flasher DESTINATION /usr/local/bin)

# Print build information
message(STATUS "Pi4Flasher build configuration:")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  C Flags: ${CMAKE_C_FLAGS}")
message(STATUS "  BCM2835 Library: ${BCM2835_LIB}")

