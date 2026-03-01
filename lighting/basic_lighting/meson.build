project(
  'basic_lighting',
  'cpp',
  'c',
  version : '0.1',
  meson_version : '>= 1.10.0',
  default_options : [
    'buildtype=debug',
  ],
)

cpp_args = [
  '-std=c++23',
  '-Og',
  '-ggdb',
  '-Wall',
  '-Wextra',
  '-DTEXTURES_PATH="../../../common/textures"'
]

includes = [
  include_directories(
    '../../common/headers/',
    'src/headers/'
    )
]

dependencies = [
  dependency('glfw3'),
  dependency('libglvnd')
]

executable(
  'basic_lighting',
  'src/main.cpp',
  'src/Camera.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
  ],
  include_directories : includes
)


# Exercises

executable(
  'ex01',
  'exercises/ex01/main.cpp',
  'src/Camera.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
  ],
  include_directories : includes
)

executable(
  'ex03',
  'exercises/ex03/main.cpp',
  'src/Camera.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
  ],
  include_directories : includes
)

executable(
  'ex04',
  'exercises/ex04/main.cpp',
  'src/Camera.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
  ],
  include_directories : includes
)
