project(
  'lighting-maps',
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
  '-DTEXTURES_PATH="../../../common/textures/"',
  '-DSHADERS_PATH="../src/shaders/"'
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
  'lighting_maps',
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
  'ex1',
  'exercises/ex1/main.cpp',
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
  'ex2',
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
    '-DSHADERS_PATH="../exercises/ex2"'
  ],
  include_directories : includes
)

executable(
  'ex3',
  'exercises/ex3/main.cpp',
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
  'ex4',
  'exercises/ex4/main.cpp',
  'src/Camera.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DSHADERS_PATH="../exercises/ex4/shaders"'
  ],
  include_directories : includes
)
