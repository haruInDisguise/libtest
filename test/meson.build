cc = meson.get_compiler('c')
cc_flags = []

cc_flags = [ '-DTEST_DEBUG', ]

test_exe = executable('run_tests', dependencies: [ libtest_dep ], sources: ['main.c', 'test_assert.c'])
