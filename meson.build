project('tsp', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++14'])

openmp = dependency('openmp')
subdir('src')
exe = executable('tsp', src,
  install : true,
  dependencies: openmp)

test('basic', exe)
