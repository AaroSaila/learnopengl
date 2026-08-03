project(
  'diffuse-irradiance',
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
  '-DSHADERS_PATH="../src/shaders/"',
  '-DMODELS_PATH="../../../common/models/"'
]

includes = [
  include_directories(
    '../../common/headers/',
    'src/headers/'
    )
]

dependencies = [
  dependency('glfw3'),
  dependency('libglvnd'),
  dependency('assimp')
]

executable(
  'diffuse-irradiance',
  'src/main.cpp',
  'src/Camera.cpp',
  'src/Mesh.cpp',
  'src/Model.cpp',
  'src/Shader.cpp',
  'src/letters.cpp',
  'src/quit.cpp',
  'src/error_handling.cpp',
  'src/stb_impl.cpp',
  '../../common/glad.c',
  dependencies : dependencies,
  native : true,
  cpp_args : [
    cpp_args,
  ],
  include_directories : includes
)

# Exercises

