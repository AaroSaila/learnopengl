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
  '-Og',
  '-ggdb',
  '-Wall',
  '-Wextra'
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
  'coordsys',
  'src/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DVERTEX_SHADER_PATH="../shaders/shader.vert"',
    '-DFRAGMENT_SHADER_PATH="../shaders/shader.frag"',
  ],
  include_directories : includes
)


# Exercises

executable(
  'ex1',
  'exercises/ex1/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DVERTEX_SHADER_PATH="../shaders/shader.vert"',
    '-DFRAGMENT_SHADER_PATH="../shaders/shader.frag"',
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
    '-DVERTEX_SHADER_PATH="../shaders/shader.vert"',
    '-DFRAGMENT_SHADER_PATH="../shaders/shader.frag"',
  ],
  include_directories : includes
)

executable(
  'ex3',
  'exercises/ex3/main.cpp',
  'src/Shader.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  '../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
    '-DVERTEX_SHADER_PATH="../shaders/shader.vert"',
    '-DFRAGMENT_SHADER_PATH="../shaders/shader.frag"',
  ],
  include_directories : includes
)
