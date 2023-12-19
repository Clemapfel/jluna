project('jluna_example', 'cpp',
    version: '0.1.0',
    default_options: ['cpp_std=c++20']
)
cpp_compiler = meson.get_compiler('cpp')

# find jluna
jluna = dependency('jluna',
    required: true,
    version: '>=1.1.0',
    method: 'cmake'
)
jluna_include_directories = include_directories(jluna.get_variable('JLUNA_INCLUDE_DIRECTORIES'))

# find julia
julia = cpp_compiler.find_library('julia',
    dirs: jluna.get_variable('JULIA_LIBRARY_PATH'),
    required: true
)

# declare project
project_sources = [
    'jluna_example.hpp',
    'jluna_example.cpp',
    'main.cpp'
]

target = executable('jluna_example',
    sources: project_sources,
    dependencies: [jluna, julia],
    include_directories: jluna_include_directories,
    install: false
)

