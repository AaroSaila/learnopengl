project(
  'text-rendering',
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
  # '-DTEXTURES_PATH="../../../common/textures/"',
  # '-DSHADERS_PATH="../src/shaders/"',
  # '-DMODELS_PATH="../../../common/models/"'
  '-DFONTS_PATH="../../../common/fonts/"'
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
  dependency('assimp'),
  dependency('freetype2')
]

executable(
  'text-rendering',
  'src/error_handling.cpp',
  'src/main.cpp',
  'src/quit.cpp',
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

