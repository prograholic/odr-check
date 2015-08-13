cmake_minimum_required(VERSION 3.2)

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -pedantic)
endif()

add_executable(enum enum1.cpp enum2.cpp)
add_executable(class class1.cpp class2.cpp)
add_executable(pack pack1.cpp pack2.cpp)
