cmake_minimum_required(VERSION 3.1)
project(hungarian)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug")

add_executable(${PROJECT_NAME} hungarian_main.cc)


set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)
message("Source tree: ${PROJECT_SOURCE_DIR}")
message("Install tree: " ${CMAKE_INSTALL_PREFIX})

# lib will be created under directory install
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

