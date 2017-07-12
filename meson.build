project('dlg', 'cpp')

build_library = get_option('build_library')

inc = include_directories('.')
defs = []
sources = []
libs = []

headers = files([
	'dlg/color.hpp',
	'dlg/config.hpp',
	'dlg/dlg.hpp',
	'dlg/fmt.hpp'
])

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

install_headers(headers, subdir: 'dlg')
dlg_dep = declare_dependency(
	include_directories: inc,
	link_with: libs,
	compile_args: defs,
	sources: sources)
