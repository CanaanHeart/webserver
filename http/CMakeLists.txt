cmake_minimum_required(VERSION 3.20)
project(HTTP_LIB)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC)
include_directories(${HEADPATH})

set(LIBRARY_OUTPUT_PATH ${LIBPATH})

add_library(http SHARED ${SRC})

target_link_libraries(http PUBLIC logger load_config serv sqlpool pthread uuid)