add_executable(point_in_polygon_tests
  mini_test.h
  test_utils.h
  test_main.cpp
  test_basic.cpp
 "test_degenerate.cpp")

target_link_libraries(point_in_polygon_tests PRIVATE point_in_polygon)

if(MSVC)
  target_compile_options(point_in_polygon_tests PRIVATE /W4 /permissive-)
else()
  target_compile_options(point_in_polygon_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME point_in_polygon_tests COMMAND point_in_polygon_tests)