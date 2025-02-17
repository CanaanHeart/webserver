cmake_minimum_required(VERSION 3.20)
project(LOAD_CONFIG_LIB)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)

set(LIBRARY_OUTPUT_PATH ${LIBPATH})

link_directories(${LIBPATH})

add_library(load_config SHARED ${SRC})

target_link_libraries(load_config logger)