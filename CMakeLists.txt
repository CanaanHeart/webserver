cmake_minimum_required(VERSION 3.20)
project(WEB_SERVER)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

set(LIBPATH ${PROJECT_SOURCE_DIR}/lib)
set(HEADPATH ${PROJECT_SOURCE_DIR}/inc)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${HEADPATH})

add_subdirectory(${PROJECT_SOURCE_DIR}/logger)
add_subdirectory(${PROJECT_SOURCE_DIR}/load_config)
add_subdirectory(${PROJECT_SOURCE_DIR}/threadpool)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/time)
add_subdirectory(${PROJECT_SOURCE_DIR}/sqlpool)
add_subdirectory(${PROJECT_SOURCE_DIR}/http)


link_directories(${LIBPATH})

add_executable(http_server ${PROJECT_SOURCE_DIR}/http_server.cpp)
# add_executable(client ${PROJECT_SOURCE_DIR}/client.cpp)
# add_executable(echo_server ${PROJECT_SOURCE_DIR}/echo_server.cpp)

target_link_libraries(http_server PUBLIC http timestamp)
# target_link_libraries(client PUBLIC serv)
# target_link_libraries(echo_server PUBLIC echo timestamp)
