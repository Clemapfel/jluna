project('jluna', 'cpp',
    version: '1.1.0',
    default_options : ['cpp_std=c++20'],
)

### DETECT JULIA ###

julia_executable = find_program(get_option('JULIA_EXECUTABLE'), 'julia', required: false)
if not julia_executable.found()
    error('Unable to detect Julia executable. Please run `print(joinpath(Sys.BINDIR), "julia")` manually within the Julia REPL, then set the `JULIA_EXECUTABLE` meson option to the returned value. For example, if the executable is `/usr/bin/julia`, call `meson setup build -DJULIA_EXECUTABLE="/usr/bin/julia"')
endif

cpp_compiler = meson.get_compiler('cpp')

julia_bindir = run_command(julia_executable, '-e print(Sys.BINDIR)', check: true).stdout().strip()

julia_include_directory = include_directories(join_paths(julia_bindir, '../include'))
julia_library_directory = join_paths(julia_bindir, '../lib')

julia_library = cpp_compiler.find_library(
    'julia',
    dirs: julia_library_directory,
    has_headers: 'julia/julia.h',
    header_include_directories: julia_include_directory,
    required: true
)

### JLUNA ###

jluna_headers = [
    'jluna.hpp',

    'include/exceptions.hpp',
    'include/typedefs.hpp',
    '.src/typedefs.inl',

    'include/unsafe_utilities.hpp',
    '.src/unsafe_utilities.inl',


    'include/safe_utilities.hpp',
    '.src/safe_utilities.inl',

    'include/concepts.hpp',

    'include/box.hpp',
    '.src/box.inl',

    'include/unbox.hpp',
    '.src/unbox.inl',

    'include/proxy.hpp',
    '.src/proxy.inl',

    'include/array.hpp',
    '.src/array.inl',
    '.src/array_iterator.inl',

    'include/cppcall.hpp',
    '.src/cppcall.inl',

    'include/module.hpp',
    '.src/module.inl',

    'include/symbol.hpp',
    '.src/symbol.inl',

    'include/type.hpp',
    '.src/type.inl',

    'include/generator_expression.hpp',

    'include/usertype.hpp',
    '.src/usertype.inl',

    'include/multi_threading.hpp',
    '.src/multi_threading.inl',

    'include/mutex.hpp',

    '.src/c_adapter.hpp',
]

jluna_source = [
    '.src/exceptions.cpp',
    '.src/unsafe_utilities.cpp',
    '.src/safe_utilities.cpp',
    '.src/proxy.cpp',
    '.src/module.cpp',
    '.src/symbol.cpp',
    '.src/type.cpp',
    '.src/generator_expression.cpp',
    '.src/mutex.cpp',
    '.src/c_adapter.cpp'
]

