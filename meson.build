project('dlg', ['c', 'cpp'],
	version: '0.1.0',
	default_options: ['c_std=c11', 
		'cpp_std=c++11', 
		'warning_level=3',
		'werror=true'])

build_library = get_option('library')
build_sample = get_option('sample')
tests = get_option('tests')

# dep variables depending on build type
inc = include_directories('include')
sources = []
libs = []

# base path, correctly escaped on windows
source_root = meson.source_root().split('\\')
add_project_arguments('-DDLG_BASE_PATH="' + '/'.join(source_root) + '/"', language: 'c')
headers = files([
	'include/dlg/output.h',
	'include/dlg/dlg.h',
	'include/dlg/dlg.hpp'
])

# TODO: import args?
# default warn settings
dlg_args = []
common_args = []
deps = []
if meson.get_compiler('c').get_id() == 'msvc'
	dlg_args += '/DDLG_API=__declspec(dllexport)'

	# make utf-8 examples work; ignore unused parameter and
	# constant conditional expression warnings and a plainly wrong warning
	common_args += ['/utf-8', '/wd4100', '/wd4127', '/wd4090'] 
else
	common_args += '-Wno-unused-parameter'
endif

# build library
dep_threads = dependency('threads')
if build_library
	dlg_lib = library('dlg',
		'src/dlg/dlg.c',
		c_args: dlg_args + common_args,
		dependencies: dep_threads,
		include_directories: inc)
	libs = [dlg_lib]
else
	sources = ['src/dlg/dlg.c']
	deps = [threads]
endif

# dependency
dlg_dep = declare_dependency(
	include_directories: inc,
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
endif

# tests
if tests
	subdir('docs/tests')
endif

# install
install_headers(headers, subdir: 'dlg')
