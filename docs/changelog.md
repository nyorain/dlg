=== Release of v0.2.2 ===

2018-5-20
	Rework the tags array building macro due to new errors in gcc8.
	Seems like the C++ way was undefined behavior before, now uses
	initializer_list.

2018-3-20
	Fix default_output_handler output on wsl, add option to disable special
	windows console handling. Also add options to always use color in
	default output handler (to work around wsl color limitation).

2018-3-19
	dlg.hpp: Allow tlformat (default dlg.hpp formatter) to be called with just a
	non-string object (which will then just be printed)

2018-01-17
	output.h: add dlg_output_msecs for more time output precision

=== Release of v0.2.1 ===
