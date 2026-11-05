cmake_minimum_required(VERSION 3.16)
project(CourtConnect CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(court_core
  src/geo.cpp src/data_loader.cpp src/merge_sort.cpp
  src/filter.cpp src/heap_sort.cpp
  src/timing.cpp src/input.cpp src/menu.cpp)
target_include_directories(court_core PUBLIC include src)

add_executable(court_connect src/main.cpp)
target_link_libraries(court_connect court_core)

add_executable(court_connect_tests tests/test_main.cpp)
target_link_libraries(court_connect_tests court_core)

add_executable(generate_players tools/generate_players.cpp)
target_include_directories(generate_players PRIVATE include)
