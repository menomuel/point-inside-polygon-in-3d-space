cmake_minimum_required(VERSION 3.16)

project(PointInPolygon3d VERSION 1.0 LANGUAGES CXX)

option(PIP_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(PIP_BUILD_TESTS "Build the unit tests" ON)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(point_in_polygon
  include/point_in_polygon.h
  src/point_in_polygon.cpp
  src/vector_math.h
)

target_include_directories(point_in_polygon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(point_in_polygon PUBLIC cxx_std_17)

if(MSVC)
  target_compile_options(point_in_polygon PRIVATE /W4 /permissive-)
  if(PIP_WARNINGS_AS_ERRORS)
    target_compile_options(point_in_polygon PRIVATE /WX)
  endif()
else()
  target_compile_options(point_in_polygon PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
  if(PIP_WARNINGS_AS_ERRORS)
    target_compile_options(point_in_polygon PRIVATE -Werror)
  endif()
endif()

if(PIP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()