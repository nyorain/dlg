// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef _DLG_OUTPUT_H_
#define _DLG_OUTPUT_H_

#include "dlg.h"
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

// Prints the given utf-8 format args to the given stream.
// On windows it makes sure that if stream is stderr or stdout and a tty, 
// the string will be correctly printed (even outside ascii range).
// Works around stupid windows utf-16 bullshit basically. On unix
// a correct locale has to be set, it's just calls printf.
void dlg_fprintf(FILE* stream, const char* format, ...) DLG_PRINTF_ATTRIB(2, 3);

// Like dlg_printf, but also applies the given style to this output.
// The style will always be applied (using escape sequences), independent of the given stream.
// On windows escape sequences don't work out of the box, see dlg_win_init_ansi().
void dlg_styled_fprintf(FILE* stream, const struct dlg_style style, 
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
extern const struct dlg_style dlg_default_output_styles[6];

// Generic output function. Used by the default output handler and might be useful
// for custom output handlers (that don't want to manually format the output).
// If stream is NULL uses stdout for level < warn, stderr otherwise.
void dlg_generic_output(FILE* stream, unsigned int features,
	const struct dlg_origin* origin, const char* string, 
	const struct dlg_style styles[6]);
	
// Returns the null-terminated escape sequence for the given style into buf.
// Undefined behvaiour if any member of style has a value outside its enum range (will
// probably result in a buffer overflow or garbage being printed).
// If all member of style are 'none' will simply nullterminate the first buf char.
void dlg_escape_sequence(const struct dlg_style style, char buf[12]);

// The reset style escape sequence.
extern const char* dlg_reset_sequence;

// Just returns true on non-windows systems.
// On windows tries to set the console mode to ansi to make escape sequences work.
// This works only on newer windows 10 versions. Returns false on error.
// Only the first call to it will have an effect, the function is also threadsafe.
// Automatically called by the default output handler.
bool dlg_win_init_ansi();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // header guard
