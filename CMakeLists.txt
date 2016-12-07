cmake_minimum_required(VERSION 3.6)
project(serial)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.c data_radio_protocol.c data_radio_protocol.h serial.h serial.c main.h data_radio_protocol.c main.c serial.c)
add_executable(serial ${SOURCE_FILES})
#add_library(serial -lpthread)
target_link_libraries(serial libpthread.so)