// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef DLG_MAIN_CPP
#define DLG_MAIN_CPP

#include <iostream>
#include <sstream>
#include <codecvt>
#include <locale>

#include <dlg/dlg.hpp>
#include <dlg/output.hpp>

#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
	#define DLG_OS_UNIX
	#include <unistd.h>
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define DLG_OS_WIN
	#define WIN32_LEAN_AND_MEAN
	#define DEFINE_CONSOLEV2_PROPERTIES
	#include <windows.h>
	#include <io.h>

	// thank you for nothing, microsoft
	#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
	#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
	#endif
#else
	#error Cannot determine platform (needed for color and utf-8 and stuff)
#endif

namespace dlg {

#ifdef DLG_OS_WIN
std::u16string to_utf16(const std::string_view& utf8)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes(&utf8.front(), &utf8.back() + 1);
}

bool check_ansi_support()
{
	HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE err = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if(out == INVALID_HANDLE_VALUE || err == INVALID_HANDLE_VALUE)
		return false;

	DWORD outMode, errMode;
	if(!::GetConsoleMode(out, &outMode) || !::GetConsoleMode(err, &errMode))
	   return false;

	outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	errMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	if (!SetConsoleMode(out, outMode) || !::SetConsoleMode(out, errMode))
		return false;

	return true;
}

bool ansi_supported()
{
	static bool ret = check_ansi_support();
	return ret;
}

WORD reverse_rgb(WORD rgb)
{
	static const WORD rev[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
	return rev[rgb];
}

void set(WORD& current, Background bg)
{
	current &= 0xFF0F;
	auto wbg = static_cast<WORD>(bg);
	if(wbg >= 100) current |= (0x8 | reverse_rgb(wbg - 100)) << 4;
	else current |= reverse_rgb(wbg - 40) << 4;
}

void set(WORD& current, Foreground fg)
{
	current &= 0xFFF0;
	auto wfg = static_cast<WORD>(fg);
	if(wfg >= 90) current |= (0x8 | reverse_rgb(wfg - 90));
	else current |= reverse_rgb(wfg - 30);
}

void set(WORD& current, Style style)
{
	if(style == Style::reset)
		current = (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

HANDLE console_handle(std::ostream& ostream)
{
	static const auto* default_out_buf = std::cout.rdbuf();
	static const auto* default_err_buf = std::cerr.rdbuf();
	static const auto* default_log_buf = std::clog.rdbuf();

	if(ostream.rdbuf() == default_out_buf)
		return ::GetStdHandle(STD_OUTPUT_HANDLE);

	if(ostream.rdbuf() == default_err_buf || ostream.rdbuf() == default_log_buf)
		return ::GetStdHandle(STD_ERROR_HANDLE);

	return nullptr;
}

bool cout_is_tty()
{
	return _isatty(_fileno(stdout));
}

#else

bool cout_is_tty()
{
	return isatty(fileno(stdout));
}

#endif // DLG_OS_WIN

void write(std::ostream& ostream, std::string_view message)
{
#ifdef DLG_OS_WIN
	// This is needed to enable correct utf-8 output on the windows
	// command line. Because doing it the right way would be too simple, eh?
	auto handle = console_handle(ostream);
	if(handle) {
		auto str = to_utf16(message);
		if(::WriteConsoleW(handle, str.c_str(), str.size(), nullptr, nullptr))
			return;
	}
#endif

	ostream << message;
}

void default_output(std::ostream& os, TextStyle style, std::string_view message)
{
#ifdef DLG_OS_WIN
	auto handle = console_handle(os);
	if(ansi_supported()) {
		auto string = escape_sequence(style);
		string.append(message).append(escape_sequence({Foreground::none, Style::reset}));
		write(os, string);
	} else {
		CONSOLE_SCREEN_BUFFER_INFO info;
		::GetConsoleScreenBufferInfo(handle, &info);

		WORD wstyle = info.wAttributes;
		if(style.bg != Background::none) set(wstyle, style.bg);
		if(style.fg != Foreground::none) set(wstyle, style.fg);
		if(style.style != Style::none) set(wstyle, style.style);

		::SetConsoleTextAttribute(handle, wstyle);
		write(os, message);
		::SetConsoleTextAttribute(handle, info.wAttributes);
	}
#else
	os << escape_sequence(style);
	os << message;
	os << escape_sequence({Foreground::none, Style::reset});
#endif
}

void default_output(std::ostream& os, std::string_view message)
{
	write(os, message);
}

std::string default_message(const Origin& origin, std::string_view msg)
{
	std::string ret;
	ret += "[";
	ret.append(origin.file).append(":").append(std::to_string(origin.line));
	ret.append("] ");

	if(!origin.expr.empty())
		ret.append("Assertion failed: '").append(origin.expr).append("' ");

	ret.append(msg).append("\n");
	return ret;
}

TextStyle default_text_style(Level level)
{
	// text styles used by the default logger for the different log levels.
	static constexpr TextStyle text_styles[] = {
		{Foreground::green, Style::italic},
		{Foreground::gray, Style::dim},
		{Foreground::cyan},
		{Foreground::yellow},
		{Foreground::red},
		{Foreground::red, Style::bold}
	};

	return text_styles[static_cast<unsigned int>(level)];
}

void default_output_handler(const Origin& origin, std::string_view msg)
{
	return generic_output_handler(std::cout, origin, msg, cout_is_tty());
}

void generic_output_handler(std::ostream& os, const Origin& origin, std::string_view msg, bool col)
{
	auto str = default_message(origin, msg);

	if(col) {
		auto text_styles = default_text_style(origin.level);
		default_output(os, text_styles, str);
	} else {
		default_output(os, str);
	}
}

std::string escape_sequence(TextStyle style)
{
	std::string ret;

	if(style.fg != Foreground::none || style.bg != Background::none || style.style != Style::none) {
		ret += "\033[";

		bool first = true;
		if(style.fg != Foreground::none) {
			first = false;
			ret += std::to_string(static_cast<unsigned int>(style.fg));
		}
		if(style.style != Style::none) {
			if(!first) {
				ret += ";";
				first = false;
			}
			ret += std::to_string(static_cast<unsigned int>(style.style));
		}
		if(style.bg != Background::none) {
			if(!first) ret += ";";
			ret += std::to_string(static_cast<unsigned int>(style.bg));
		}

		ret += "m";
	}

	return ret;
}

OutputHandler output_handler(OutputHandler set)
{
	auto& handler = output_handler();
	auto cpy = std::move(handler);
	handler = set;
	return cpy;
}

OutputHandler& output_handler()
{
	static OutputHandler handler = &default_output_handler;
	return handler;
}

std::list<CurrentTag>& current_tags_ref()
{
	thread_local std::list<CurrentTag> tags = {};
	return tags;
}

void do_output(Origin& origin, std::string_view msg)
{
	return output_handler()(origin, msg);
}

TagsGuard::TagsGuard(std::initializer_list<std::string_view> tags, const char* func)
{
	auto& ref = current_tags_ref();
	refs_.reserve(tags.size());
	for(auto tag : tags) {
		refs_.push_back(ref.insert(ref.end(), {tag, func}));
	}
}

TagsGuard::~TagsGuard()
{
	auto& ref = current_tags_ref();
	for(auto& it : refs_) {
		ref.erase(it);
	}
}

namespace detail {
std::string_view strip_path(std::string_view file, std::string_view base)
{
	if(file.empty())
		return file;

	if(file[0] == '.') {
		auto i = 0u;
		for(; i < file.size() && (file[i] == '.' || file[i] == '/'); ++i);
		return file.substr(i);
	}

	const auto start = base.length();
	return file.substr(start > file.size() ? 0 : start);
}

} // namespace detail
} // namespace dlg

#endif // header guard
