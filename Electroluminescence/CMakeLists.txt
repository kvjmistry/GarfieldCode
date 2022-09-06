cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(Electroluminescence)
if(NOT TARGET Garfield::Garfield)
  find_package(Garfield REQUIRED)
endif()

# ---Define executables---------------------------------------------------------
add_executable(Mesh Mesh.C)
target_link_libraries(Mesh Garfield::Garfield)

add_executable(PlotEField PlotEField.C)
target_link_libraries(PlotEField Garfield::Garfield)