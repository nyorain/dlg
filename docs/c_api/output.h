#include <stdio.h>

// Text style
enum dlg_text_style {
	DLG_TEXT_STYLE_RESET     = 0,
	DLG_TEXT_STYLE_BOLD      = 1,
	DLG_TEXT_STYLE_DIM       = 2,
	DLG_TEXT_STYLE_ITALIC    = 3,
	DLG_TEXT_STYLE_UNDERLINE = 4,
	DLG_TEXT_STYLE_BLINK     = 5,
	DLG_TEXT_STYLE_RBLINK    = 6,
	DLG_TEXT_STYLE_REVERSED  = 7,
	DLG_TEXT_STYLE_CONCEAL   = 8,
	DLG_TEXT_STYLE_CROSSED   = 9,
	DLG_TEXT_STYLE_NONE,
};

// Text color
enum dlg_color {
	DLG_COLOR_BLACK = 0,
	DLG_COLOR_RED,
	DLG_COLOR_GREEN,
	DLG_COLOR_YELLOW,
	DLG_COLOR_BLUE,
	DLG_COLOR_MAGENTA,
	DLG_COLOR_CYAN,
	DLG_COLOR_GRAY,
	DLG_COLOR_RESET = 9,

	DLG_COLOR_BLACK2 = 60,
	DLG_COLOR_RED2,
	DLG_COLOR_GREEN2,
	DLG_COLOR_YELLOW2,
	DLG_COLOR_BLUE2,
	DLG_COLOR_MAGENTA2,
	DLG_COLOR_CYAN2,
	DLG_COLOR_GRAY2,

	DLG_COLOR_NONE = 69,
};

struct dlg_style {
	enum dlg_text_style style;
	enum dlg_color fg;
	enum dlg_color bg;
};

inline void escape_sequence(const struct dlg_style style, char buf[10]) {
	unsigned int nums[3];
	unsigned int count = 0;

	if(style.fg != DLG_COLOR_NONE) {
		nums[count++] = style.fg + 30;
	}

	if(style.bg != DLG_COLOR_NONE) {
		nums[count++] = style.fg + 40;
	}

	if(style.style != DLG_TEXT_STYLE_NONE) {
		nums[count++] = style.style;
	}

	switch(count) {
		case 1: snprintf(buf, 10, "\033[%dm", nums[0]); break;
		case 2: snprintf(buf, 10, "\033[%d;%dm", nums[0], nums[1]); break;
		case 3: snprintf(buf, 10, "\033[%d;%d;%dm", nums[0], nums[1], nums[2]); break;
		default: buf[0] = '\0'; break;
	}
}
