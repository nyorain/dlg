// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef _DLG_OUTPUT_H_
#define _DLG_OUTPUT_H_

#include <dlg/dlg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Text style
enum dlg_text_style {
	dlg_text_style_reset     = 0,
	dlg_text_style_bold      = 1,
	dlg_text_style_dim       = 2,
	dlg_text_style_italic    = 3,
	dlg_text_style_underline = 4,
	dlg_text_style_blink     = 5,
	dlg_text_style_rblink    = 6,
	dlg_text_style_reversed  = 7,
	dlg_text_style_conceal   = 8,
	dlg_text_style_crossed   = 9,
	dlg_text_style_none,
};

// Text color
enum dlg_color {
	dlg_color_black = 0,
	dlg_color_red,
	dlg_color_green,
	dlg_color_yellow,
	dlg_color_blue,
	dlg_color_magenta,
	dlg_color_cyan,
	dlg_color_gray,
	dlg_color_reset = 9,

	dlg_color_black2 = 60,
	dlg_color_red2,
	dlg_color_green2,
	dlg_color_yellow2,
	dlg_color_blue2,
	dlg_color_magenta2,
	dlg_color_cyan2,
	dlg_color_gray2,

	dlg_color_none = 69,
};

struct dlg_style {
	enum dlg_text_style style;
	enum dlg_color fg;
	enum dlg_color bg;
};

// Like fprintf but fixes utf-8 output to console on windows.
DLG_API void dlg_fprintf(FILE* stream, const char* format, ...) DLG_PRINTF_ATTRIB(2, 3);
DLG_API void dlg_vfprintf(FILE* stream, const char* format, va_list list);

// Like dlg_printf, but also applies the given style to this output.
// The style will always be applied (using escape sequences), independent of the given stream.
// On windows escape sequences don't work out of the box, see dlg_win_init_ansi().
DLG_API void dlg_styled_fprintf(FILE* stream, struct dlg_style style, 
	const char* format, ...) DLG_PRINTF_ATTRIB(3, 4);
	
// Features to output from the generic output handler
enum dlg_output_feature {
	dlg_output_tags = 1, // output tags list
	dlg_output_time = 2, // output time of log call (hour:minute:second)
	dlg_output_style = 4, // whether to use the supplied styles
	dlg_output_func = 8, // output function
	dlg_output_file_line = 16, // output file:line
};

// The default level-dependent output styles. The array values represent the styles
// to be used for the associated level (i.e. [0] for trace level).
DLG_API const struct dlg_style dlg_default_output_styles[6];

// Generic output function. Used by the default output handler and might be useful
// for custom output handlers (that don't want to manually format the output).
// Will call the given output func with the given data (and format + args to print)
// for everything it has to print in printf format. 
// See also the *_stream and *_buf specializations for common usage.
// The given output function must not be NULL.
typedef void(*dlg_generic_output_handler)(void* data, const char* format, ...);
DLG_API void dlg_generic_output(dlg_generic_output_handler output, void* data, 
		unsigned int features, const struct dlg_origin* origin, const char* string, 
		const struct dlg_style styles[6]);

// Generic output function. Used by the default output handler and might be useful
// for custom output handlers (that don't want to manually format the output).
// If stream is NULL uses stdout for level < warn, stderr otherwise.
// Automatically uses dlg_fprintf to assure correct utf-8 even on windows consoles.
DLG_API void dlg_generic_output_stream(FILE* stream, unsigned int features,
	const struct dlg_origin* origin, const char* string, 
	const struct dlg_style styles[6]);

// Generic output function (see dlg_generic_output) that uses a buffer instead of 
// a stream. buf must at least point to *size bytes. Will set *size to the number
// of bytes written (capped to the given size), if buf == NULL will set *size
// to the needed size. The size parameter must not be NULL.
DLG_API void dlg_generic_output_buf(char* buf, size_t* size, unsigned int features,
	const struct dlg_origin* origin, const char* string, 
	const struct dlg_style styles[6]);

// Returns if the given stream is a tty. Useful for custom output handlers
// e.g. to determine whether to use color.
DLG_API bool dlg_is_tty(FILE* stream);
	
// Returns the null-terminated escape sequence for the given style into buf.
// Undefined behvaiour if any member of style has a value outside its enum range (will
// probably result in a buffer overflow or garbage being printed).
// If all member of style are 'none' will simply nullterminate the first buf char.
DLG_API void dlg_escape_sequence(struct dlg_style style, char buf[12]);

// The reset style escape sequence.
DLG_API const char* dlg_reset_sequence;

// XXX: let this function take a FILE* parameter and return true/false depending
// on whether ansi could be set AND the given stream is stdout/stderr pointing
// to a terminal?

// Just returns true on non-windows systems.
// On windows tries to set the console mode to ansi to make escape sequences work.
// This works only on newer windows 10 versions. Returns false on error.
// Only the first call to it will have an effect, following calls just return the result.
// The function is threadsafe. Automatically called by the default output handler.
// This will only be able to set the mode for the stdout and stderr consoles, so
// other streams to consoles will proably still not work.
DLG_API bool dlg_win_init_ansi(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // header guard
