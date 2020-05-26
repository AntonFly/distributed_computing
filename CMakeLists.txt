cmake_minimum_required (VERSION 2.6)
project (lab5)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Werror -pedantic")


add_executable(peer
        main.c
        ipc.c
        common.h
        ipc.h
        io.h
        banking.h
        pa2345.h
        bank_robbery.c
        messages.c
        process.c
        logging.c
        debug.h
        critical.c
        critical.h priority.h priority.c self_conversion.h)

find_library(libruntime lib64/libruntime.so paths ${PROJECT_SOURCE_DIR}/lib64/)
#target_link_libraries(lab5 ${libruntime})