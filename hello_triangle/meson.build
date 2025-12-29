project(
  'hello_triangle',
  'cpp',
  'c',
  version : '0.1',
  meson_version : '>= 1.10.0',
)

dependencies = [
  dependency('glfw3'),
  dependency('libglvnd')
]

cpp_args = [
  '-std=c++23',
  '-ggdb'
]

includes = [
  include_directories('../common/headers')
]

executable(
  'triangle',
  'src/triangle.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : cpp_args,
  include_directories : includes
)

executable(
  'rectangle',
  'src/rectangle.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : cpp_args,
  include_directories : includes
)

# Exercises

executable(
  'ex1',
  'exercises/ex1/ex1.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  cpp_args : cpp_args,
  include_directories : includes
)

executable(
  'ex2',
  'exercises/ex2/ex2.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  cpp_args : cpp_args,
  include_directories : includes
)

executable(
  'ex3',
  'exercises/ex3/ex3.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  cpp_args : cpp_args,
  include_directories : includes
)
