cmake_minimum_required(VERSION 3.2)

find_package(Boost)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  set(boost_src boost.cpp)
endif()

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
  string_literal1.cpp
  string_literal2.cpp
)

add_library(test_lib STATIC
  ${test_src}
  ${boost_src}
)


add_executable(odr
  main.cpp
)

target_link_libraries(odr test_lib)
