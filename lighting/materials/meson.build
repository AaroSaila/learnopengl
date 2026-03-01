project(
  'materials',
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
  'materials',
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
  'ex02',
  'exercises/ex02/main.cpp',
  'src/Camera.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../../common/glad.c',
  'exercises/ex02/materials.cpp',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
  ],
  include_directories : [
    includes,
    include_directories('exercises/ex02/headers/')
  ]
)
