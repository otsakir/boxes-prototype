cmake_minimum_required(VERSION 3.7)

project(sdl-game)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})


find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

add_executable(sdl-game sdl-game.cpp game.cpp engine.cpp utils.cpp)
target_link_libraries(sdl-game ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARY})
