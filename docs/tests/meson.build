tests = [
  ['core', 'core.c'],
  ['cpp', 'cpp.cpp'],
]

foreach test : tests
  test(test[0], 
  	executable(test[0], test[1], 
	  	c_args: common_args,
	  	cpp_args: common_args,
		dependencies: dlg_dep))
endforeach
