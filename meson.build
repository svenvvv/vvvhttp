project(
	'vvvhttp',
	'c',
	version: '1.0',
	license: 'GPLv3',
	meson_version: '>=0.60.0',
	default_options: [
		'c_std=c99',
		'warning_level=2',
		'werror=true',
	]
)

lib_vvvhttp_inc = include_directories('include/vvvhttp')
vvvhttp_inc = include_directories('include')

subdir('vvvhttp')

executable(
    'vvvhttp_example',
    files('main.c', 'handler_example.c'),
    include_directories: vvvhttp_inc,
    link_with: [lib_vvvhttp],
)

if get_option('fuzzing').enabled()
    cc = meson.get_compiler('c')
    fuzz_opts = '-fsanitize=address,fuzzer,undefined'
    if cc.has_argument(fuzz_opts)
        executable(
            'vvvhttp_libfuzzer',
            [ files('main_libfuzzer.c', 'handler_example.c'), lib_vvvhttp_src ],
            include_directories: [ vvvhttp_inc, lib_vvvhttp_inc ],
            link_args: fuzz_opts,
            c_args: fuzz_opts
        )
    endif
endif

if get_option('testing').enabled()
    subdir('tests')
endif
