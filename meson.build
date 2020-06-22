project('dlg', ['c', 'cpp'],
	version: '0.2.2',
	meson_version: '>=0.46.0',
	default_options: [
		'c_std=c11',
		'cpp_std=c++11',
		'warning_level=3',
		'werror=true'])

buildlib = get_option('buildlib')
build_sample = get_option('sample')
tests = get_option('tests')
win_console = get_option('win_console')
default_output_always_color = get_option('default_output_always_color')

# dep variables depending on build type
inc = include_directories('include')
sources = []
libs = []

# base path, correctly escaped on windows
base_path = join_paths([meson.source_root(), ''])
add_project_arguments('-DDLG_BASE_PATH="' + base_path + '"', language: ['c', 'cpp'])
message('DLG_BASE_PATH: ' + base_path)

headers = files([
	'include/dlg/output.h',
	'include/dlg/dlg.h',
	'include/dlg/dlg.hpp',
])

# TODO: import args (dllimport)?
# default warn settings
dlg_args = []
common_args = []
deps = []
if meson.get_compiler('c').get_id() == 'msvc'
	# make utf-8 examples work; ignore unused parameter and
	# constant conditional expression warnings and a plainly wrong warning
	common_args += ['/utf-8', '/wd4100', '/wd4127', '/wd4090']
endif

if win_console
	dlg_args += '-DDLG_WIN_CONSOLE=1'
endif

if default_output_always_color
	dlg_args += '-DDLG_DEFAULT_OUTPUT_ALWAYS_COLOR=1'
endif

# build library
dep_threads = dependency('threads')
dep_args = []

if buildlib
	# TODO: temporary workaround, see https://github.com/mesonbuild/meson/issues/3304
	# The problem is that on windows, function symbols must be declared
	# differently when building a static library.
	libtype = get_option('default_library')
	build_static = libtype == 'both' or libtype == 'static'
	build_shared = libtype == 'both' or libtype == 'shared'

	static_args = dlg_args + ['-DDLG_STATIC']
	shared_args = dlg_args
	if host_machine.system() == 'windows'
		shared_args += ['-DDLG_API=__declspec(dllexport)']
	endif

	if build_shared
		shared_lib = shared_library('dlg',
			'src/dlg/dlg.c',
			c_args: shared_args + common_args + dep_args,
			dependencies: dep_threads,
			install: true,
			include_directories: inc)

		libs += [shared_lib]
	endif

	if build_static
		if get_option('static_msvc_libname')
			static_lib = static_library('dlg',
				'src/dlg/dlg.c',
				c_args: static_args + common_args + dep_args,
				dependencies: dep_threads,
				install: true,
				name_prefix: '',
				name_suffix: '.lib',
				include_directories: inc)
		else
			static_lib = static_library('dlg',
				'src/dlg/dlg.c',
				c_args: static_args + common_args + dep_args,
				dependencies: dep_threads,
				install: true,
				include_directories: inc)
		endif

		if not build_shared
			dep_args += static_args
			libs += [static_lib]
		endif
	endif

	pkg = import('pkgconfig')
	pkg.generate(
		libs,
		name: 'dlg',
		filebase: 'dlg',
		subdirs: ['.', 'dlg'],
		version: meson.project_version(),
		description: 'C/C++ logging and debug library')
else
	sources = ['src/dlg/dlg.c']
	deps = [dep_threads]
endif

# dependency
dlg_dep = declare_dependency(
	include_directories: inc,
	compile_args: dep_args,
	link_with: libs,
	dependencies: deps,
	sources: sources)

# sample
if build_sample
	sample = executable('sample',
		'docs/examples/example.cpp',
		cpp_args: common_args,
		dependencies: dlg_dep)
	test('sample', sample)

	sample_c = executable('sample-c',
		'docs/examples/example.c',
		c_args: common_args,
		dependencies: dlg_dep)
	test('sample_c', sample_c)

	sample_chain = executable('sample-chain',
		'docs/examples/chain.c',
		c_args: common_args,
		dependencies: dlg_dep)
	test('sample_chain', sample_chain)
endif

# tests
if tests
	subdir('docs/tests')
endif

# install
install_headers(headers, subdir: 'dlg')
