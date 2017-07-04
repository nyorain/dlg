// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef DLG_COLOR_HPP
#define DLG_COLOR_HPP

#pragma once

#include <string>
#include "config.hpp"

namespace dlg {

// Text style
enum class Style {
	reset     = 0,
	bold      = 1,
	dim       = 2,
	italic    = 3,
	underline = 4,
	blink     = 5,
	rblink    = 6,
	reversed  = 7,
	conceal   = 8,
	crossed   = 9,

	none
};

// Text forground
enum class Foreground {
	black    = 30,
	red      = 31,
	green    = 32,
	yellow   = 33,
	blue     = 34,
	magenta  = 35,
	cyan     = 36,
	gray     = 37,
	reset    = 39,

	black2   = 90,
	red2     = 91,
	green2   = 92,
	yellow2  = 93,
	blue2    = 94,
	magenta2 = 95,
	cyan2    = 96,
	gray2    = 97,

	none
};

// Text background
enum class Background {
	black    = 40,
	red      = 41,
	green    = 42,
	yellow   = 43,
	blue     = 44,
	magenta  = 45,
	cyan     = 46,
	gray     = 47,
	reset    = 49,

	black2   = 100,
	red2     = 101,
	green2   = 102,
	yellow2  = 103,
	blue2    = 104,
	magenta2 = 105,
	cyan2    = 106,
	gray2    = 107,

	none
};

// Sumed up text style.
struct TextStyle {
	Foreground fg = Foreground::none;
	Style style = Style::none;
	Background bg = Background::none;
};

// text styles used by the default logger for the different log levels.
TextStyle textStyles[] = {
	{Foreground::green, Style::italic},
	{Foreground::gray2, Style::dim},
	{Foreground::blue},
	{Foreground::yellow},
	{Foreground::red},
	{Foreground::red, Style::bold}
};

// Sets the given text style.
DLG_API std::string apply(TextStyle);

}

#endif // header guard
