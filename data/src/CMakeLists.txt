cmake_minimum_required(VERSION 3.2)


add_definitions(-DFOOBAR)
add_compile_options(-Wall)

set(test_src
  m1.cpp
  m2.cpp
  m3.cpp
  m4.cpp
  no_default.cpp
  simple_template_struct2.cpp
  simple_template_struct.cpp
)

add_library(test_lib STATIC
  ${test_src}
)
