
cc = meson.get_compiler('c')
test_c_args = []
coverage_c_args = ['-fprofile-instr-generate', '-fcoverage-mapping']

if cc.has_multi_arguments(coverage_c_args)
    test_c_args += coverage_c_args
endif

executable(
    'test_parsing',
    [ files('test_driver.c', 'parsing.c'), lib_vvvhttp_src ],
    include_directories: [vvvhttp_inc, lib_vvvhttp_inc],
    c_args: test_c_args,
    link_args: test_c_args,
)

executable(
    'test_serializing',
    [ files('test_driver.c', 'serializing.c'), lib_vvvhttp_src ],
    include_directories: [vvvhttp_inc, lib_vvvhttp_inc],
    c_args: test_c_args,
    link_args: test_c_args,
)
