#include "dlg.h"
#include <stdio.h>

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


// Writes the given utf-8 string to the given stream.
// On windows it makes sure that if stream
// is stderr or stdout and a tty, the string will be correctly printed (even
// outside ascii range) which fwrite does not.
// Works around stupid windows bullshit basically.
void dlg_output(FILE* stream, const char* string);

// Like output, but also applies the given style to it.
// Resets the style afterwards.
void dlg_styled_output(FILE* stream, const char* string, const struct dlg_style style);

// Returns the null-terminated escape sequence for the given style into buf.
// Undefined behvaiour if any member of style has a value outside its enum range (will
// probably result in a buffer overflow or garbage being printed).
// If all member of style are 'none' will simply nullterminate the first buf char.
void dlg_escape_sequence(const struct dlg_style style, char buf[12]);

// Returns the reset style escape sequence.
const char* dlg_get_reset_sequence();

// fprintf wrapper that allows to print utf8 on windows consoles.
// If not on windows (or not printing to a console) simply evaluates to printf.
#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define dlg_fprintf(stream, ...) \
		if(dlg__win_get_console_handle(stream)) { \
			dlg__win_output(stream, L DLG_FIRST(__VA_ARGS__), __VA_ARGS__); \
		} else { \
			fprintf(stream, __VA_ARGS__); \
		}
#else
	#define dlg_fprintf(stream, ...) fprintf(stream, __VA_ARGS__);
#endif

#define DLG_FIRST(...) DLG_FIRST_HELPER(__VA_ARGS__, "DLG_FIRST_ERROR")
#define DLG_FIRST_HELPER(first, ...) first

// - Private interface not part of the api -
void* dlg__win_get_console_handle(FILE* stream);
void dlg__win_output(FILE* stream, wchar_t* format, char* oldformat, ...) DLG_PRINTF_ATTRIB(3, 4);
