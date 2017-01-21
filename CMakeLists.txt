cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 14)
project (arduinopc)
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

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/C++/arduino/include/")

set(SOURCE_BASE /opt/GitHub/arduinopc/)

set (ARDUINO_SOURCES "${SOURCE_BASE}/src/C++/arduino/src/arduino.cpp")


add_library(arduinopc SHARED "${ARDUINO_SOURCES}")

add_library(arduinopcstatic STATIC "${ARDUINO_SOURCES}")
target_link_libraries(arduinopc tjlutils pthread)
                        
set_target_properties(arduinopcstatic PROPERTIES OUTPUT_NAME arduinopc)
