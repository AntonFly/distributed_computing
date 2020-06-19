cmake_minimum_required (VERSION 2.6)
project (lab5)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror -pedantic")


add_executable(peer
        main.c
        ipc.c
        common.h
        ipc.h
        banking.h
        queue.c
        queue.h
        pa2345.h
        messages.c
        process.c
        critical.c queue.c queue.h)

find_library(libruntime libruntime.so paths ${PROJECT_SOURCE_DIR}/lib64/)
#target_link_libraries(peer ${libruntime})