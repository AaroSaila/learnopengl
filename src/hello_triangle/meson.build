dependencies = [
  dependency('glfw3'),
  dependency('libglvnd')
]

cpp_args = [
  '-std=c++23',
  '-ggdb'
]

includes = [
  include_directories('../headers')
]

executable(
  'triangle',
  'triangle.cpp',
  '../glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : cpp_args,
  include_directories : includes
)

executable(
  'rectangle',
  'rectangle.cpp',
  '../glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : cpp_args,
  include_directories : includes
)
