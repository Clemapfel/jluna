project('jluna', 'cpp',
    version: '1.1.0',
    default_options : ['cpp_std=c++20'],
)

### DETECT JULIA ###

julia_executable = find_program(get_option('JULIA_EXECUTABLE'), 'julia', required: false)
if not julia_executable.found()
    error('Unable to detect Julia executable. Please set the JULIA_EXECUTABLE meson option to the absolute path of the executable, for example, if the executable is located at `/usr/bin/julia`, call `meson setup build -DJULIA_EXECUTABLE="/usr/bin/julia"')
endif

cpp_compiler = meson.get_compiler('cpp')

result = run_command(julia_executable, '-e "print(stdout, Sys.BINDIR)"', check: true).stdout().strip()
result = '/home/clem/Applications/julia-1.9.3/julia-1.9.3/bin'

julia_include_directory = join_paths(result, '../include')
julia_library_directory = join_paths(result, '../lib')

message(julia_include_directory)

julia_library = cpp_compiler.find_library(
    'julia',
    dirs: julia_library_directory,
    has_headers: 'julia/julia.h',
    header_include_directories: include_directories(julia_include_directory),
    required: true
)