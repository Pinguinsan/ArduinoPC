cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 14)
project (tjlarduino)
set (BUILD_SHARED_LIBRARIES)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    set (X86 TRUE)
else()
    set (X86 FALSE)
endif()

if ((CYGWIN OR WIN32) OR (NOT X86))
    set(GCC_ADDITIONAL_COMPILE_FLAGS "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_ADDITIONAL_COMPILE_FLAGS}")
else()
    set(GCC_ADDITIONAL_COMPILE_FLAGS "-fsanitize=undefined -frecord-gcc-switches")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_ADDITIONAL_COMPILE_FLAGS}")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/arduino/include/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/serialport/include/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/tstream/include/")

set(SOURCE_BASE /opt/GitHub/tjlarduino)

set (ARDUINO_SOURCES "${SOURCE_BASE}/arduino/src/arduino.cpp")
set (SERIALPORT_SOURCES "${SOURCE_BASE}/serialport/src/serialport.cpp")


add_library(tjlarduino SHARED "${ARDUINO_SOURCES}"
                              "${SERIALPORT_SOURCES}")

add_library(tjlarduinostatic STATIC "${ARDUINO_SOURCES}"
                                    "${SERIALPORT_SOURCES}")
                        
set_target_properties(tjlarduinostatic PROPERTIES OUTPUT_NAME tjlarduino)
