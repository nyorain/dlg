project('dlg', 'cpp',
	version: '0.1.0',
	default_options: 'cpp_std=c++1z')

build_library = get_option('library')
build_sample = get_option('sample')

# dep variables depending on build type
inc = include_directories('include')
sources = []
libs = []

add_project_arguments(
	'-Wall',
	'-Wextra',
	'-Wpedantic',
	language: 'cpp')

headers = files([
	'include/dlg/output.hpp',
	'include/dlg/config.hpp',
	'include/dlg/dlg.hpp',
	'include/dlg/fmt.hpp'
])

# build library
if build_library
	dlg_lib = library('dlg',
		'src/dlg/dlg.cpp',
		include_directories: inc)
	libs = [dlg_lib]
else
	sources = ['src/dlg/dlg.cpp']
endif

# dependency
dlg_dep = declare_dependency(
	include_directories: inc,
	link_with: libs,
	sources: sources)

# sample
if build_sample
	sample = executable('sample', 'sample.cpp', dependencies: dlg_dep)
endif

# install
install_headers(headers, subdir: 'dlg')
