// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef DLG_COLOR_HPP
#define DLG_COLOR_HPP

#pragma once

#include <string>
#include "config.hpp"
#include "dlg.hpp"

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

// Writes the given utf-8 string to the given ostream.
// Will also work correctly on windows terminals (that do usually not correctly
// output utf-8). Note that the output is written using the underlaying
// windows api if on windows and therefore, if the ostream is not synced
// with stdio, this might be shown before output written using the ostream api.
void write(std::ostream& ostream, std::string_view message);

// The default output function.
// Will simply print the given string with the given style to std::cout.
void defaultOutput(TextStyle style, std::string_view message);

// The output function.
// Can be set using the dlg::output function.
// Will be called everytime a log or assert is outputted.
// Might abandon it or otherwise chose where to log it.
using OutputHandler = std::function<void(const Origin&, std::string_view msg)>;

// Default message formatting.
// Will print the given origin and message in the given format:
// [source | file:line] message
// If the origin is a assertion, will preprend the assertion expression
// to the message.
std::string defaultMessage(const Origin& origin, std::string_view msg);

// The default output handler.
// Uses 'defaultMessage' and 'defaultOutput' (from dlg/output.hpp) in every case.
void defaultOutputHandler(const Origin& origin, std::string_view msg);

// Sets a new Selector. Must be valid i.e. not an empty std::function object.
// The default Outputter is dlg::defaultOutput.
// Returns the old selector.
OutputHandler outputHandler(OutputHandler set);

// Receives the current selector.
OutputHandler& outputHandler();

// Returns a unix escape sequence for the given text style.
std::string escapeSequence(TextStyle style);

} // namespace dlg

#endif // header guard
