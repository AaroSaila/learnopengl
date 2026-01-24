project(
  'learnopengl_shaders',
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
  '-ggdb',
]

includes = [
  include_directories(
    '../common/headers/',
    'src/headers/'
    )
]

dependencies = [
  dependency('glfw3'),
  dependency('libglvnd')
]

executable(
  'shaders',
  'src/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DSHADERS_DIR="../shaders/"'
  ],
  include_directories : includes
)


# Exercises

executable(
  'ex1',
  'src/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DSHADERS_DIR="../shaders/ex1/"'
  ],
  include_directories : includes
)

executable(
  'ex2',
  'ex2/src/main.cpp',
  'ex2/src/Shader.cpp',
  'ex2/src/quit.cpp',
  'ex2/src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : cpp_args,
  include_directories : includes
)

executable(
  'ex3',
  'src/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DSHADERS_DIR="../shaders/ex3/"'
  ],
  include_directories : includes
)

