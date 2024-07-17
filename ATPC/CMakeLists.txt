cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(Electroluminescence)
if(NOT TARGET Garfield::Garfield)
  find_package(Garfield REQUIRED)
endif()

# ---Define executables---------------------------------------------------------
add_executable(ATPC ATPC.C)
target_link_libraries(ATPC Garfield::Garfield)
