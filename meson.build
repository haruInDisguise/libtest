project('test', 'c',
  version: '0.1.0',
  license: 'TODO',
  meson_version: '>=0.59.0',
  default_options: [
    'c_std=c99',
    'warning_level=3',
    'werror=true',
    'buildtype=release',
    'b_sanitize=address,undefined',
    'b_ndebug=if-release'
  ])

test_lib = declare_dependency(include_directories: 'include')

if not meson.is_subproject()
  subdir('test')
endif
