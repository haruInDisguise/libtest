cc = meson.get_compiler('c')
cc_flags = []

cc_flags = [ '-DTEST_DEBUG', ]

test_exe = executable('run_tests', dependencies: [ test_lib ], sources: ['main.c', 'test_assert.c'])
