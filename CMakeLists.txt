cmake_minimum_required(VERSION 2.8.4)
project(chimps)

set(SOURCE_FILES
    defs.h
    chimps.c
    cpu.c
    )
add_executable(chimps ${SOURCE_FILES})
