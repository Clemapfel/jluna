project('jluna_example', 'cpp',
    version: '0.1.0',
    default_options: ['cpp_std=c++20']
)

# find jluna
jluna = dependency('jluna', required: true, method: 'cmake')

# find julia
cpp_compiler = meson.get_compiler('cpp')
julia_location = jluna.get_variable('JLUNA_LIBRARIES').split(';')[1].split('/')


julia = cpp_compiler.find_library('julia', dirs: julia_location)
# declare project
project_sources = [
    'jluna_example.hpp',
    'jluna_example.cpp',
    'main.cpp'
]

target = executable('jluna_example',
    sources: project_sources,
    dependencies: [jluna, julia],
    install: false
)

