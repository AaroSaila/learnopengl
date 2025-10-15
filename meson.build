project(
  'learnopengl',
  'cpp',
  'c',
  version : '0.1',
  meson_version : '>= 1.3.0',
  default_options : [
    'buildtype=debug',
  ],
)

dependencies = [
  dependency('glfw3'),
  dependency('libglvnd')
]

compile_flags = [
  '-std=c++23',
  '-ggdb',
  '-Wfatal-errors',
]

exe = executable(
  'learnopengl',
  'src/main.cpp',
  'src/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : compile_flags,
  include_directories : [ include_directories('src/headers') ]
)
