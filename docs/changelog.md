2023-04-16 (cumulative)
	- Add `dlg_assert_or` macro that will execute code in case the 
	  assertion fails.
	  Note that this will even happen when dlg is disabled, so suited
	  for early-outs and defensive coding.
	- Add `DLG_FAILED_ASSERTION_TEXT(x)`, a wrapper around the failed
	  expression of an assert. Only ever called when the assertion
	  has failed, so can be used to add additional handling (e.g.
	  throwing) in that case for certain build configs. Found to be
	  useful for static analysis, to silence warnings asserted upon.
	- Rework how DLG_DISABLE works for api non-macro functions.
	  The functions are now independent from DLG_DISABLE to prevent
	  linking issues when DLG_DISABLE is defined

=== Release of v0.3 ===

2019-12-07
	- Add dlg_get_handler to allow handler chaining for debugging
	  [api addition]
	- dlg.h previously required DLG_DISABLE to be defined to 1, this
	  was already documented differently in api.md and examples.
	  Now it's enough if DLG_DISABLE is defined at all
	  [breaking change]
	- rework dlg__strip_root_path to actually check for prefix.
	  If the base path isn't a prefix of the file, we won't strip it.
	- change naming of header guard to not start with underscore

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
