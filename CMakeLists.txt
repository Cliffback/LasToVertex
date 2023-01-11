# CMakeList.txt : CMake project for LasToVertex, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("LasToVertex")

# Add source to this project's executable.
add_executable (LasToVertex "LasToVertex.cpp" "LasToVertex.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LasToVertex PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
