project(
  'learnopengl_textures',
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
  'textures',
  'src/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DVERTEX_SHADER_PATH="../shaders/shader.vs"',
    '-DFRAGMENT_SHADER_PATH="../shaders/shader.fs"',
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
    '-DVERTEX_SHADER_PATH="../shaders/shader.vs"',
    '-DFRAGMENT_SHADER_PATH="../shaders/ex1/shader.fs"',
  ],
  include_directories : includes
)

executable(
  'ex2',
  'exercises/ex2/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DVERTEX_SHADER_PATH="../shaders/shader.vs"',
    '-DFRAGMENT_SHADER_PATH="../shaders/shader.fs"',
  ],
  include_directories : includes
)
