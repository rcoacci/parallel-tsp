project('tsp', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++17',
                     'cpp_args=-Wno-unused-parameter -Wno-unknown-pragmas'])

openmp = dependency('openmp')
mpi = dependency('mpi', language:'cpp')
subdir('src')
exe = executable('tsp', src, install : true)
exe_omp = executable('tsp-omp', src, install : true, dependencies: openmp)
exe_mpi = executable('tsp-mpi', src_mpi, install : true, dependencies: mpi)
