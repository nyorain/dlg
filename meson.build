project('dlg', 'cpp',
	version: '0.1.0',
	default_options: 'cpp_std=c++1z')

build_library = get_option('build_library')
build_sample = get_option('build_sample')

# dep variables depending on build type
inc = include_directories('.')
defs = []
sources = []
libs = []

headers = files([
	'dlg/output.hpp',
	'dlg/config.hpp',
	'dlg/dlg.hpp',
	'dlg/fmt.hpp'
])

# build library
if build_library
	dlg_lib = library('dlg', 'dlg/dlg.cpp',
		include_directories: inc,
		cpp_args: '-DDLG_HEADER_ONLY=0')
	libs = [dlg_lib]
else
	defs = ['-DDLG_HEADER_ONLY=0']
	sources = ['dlg/dlg.cpp']
	headers += ['dlg/dlg.cpp']
endif

# dependency
dlg_dep = declare_dependency(
	include_directories: inc,
	link_with: libs,
	compile_args: defs,
	sources: sources)

# sample
if build_sample
	executable('sample', 'sample.cpp', dependencies: dlg_dep)
endif

# install
install_headers(headers, subdir: 'dlg')
