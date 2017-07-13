// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef DLG_MAIN_CPP
#define DLG_MAIN_CPP

#include <iostream>
#include <sstream>
#include <codecvt>
#include <locale>

#include "dlg.hpp"
#include "output.hpp"

#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
	#define DLG_OS_UNIX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define DLG_OS_WIN
	#define WIN32_LEAN_AND_MEAN
	#define DEFINE_CONSOLEV2_PROPERTIES
	#include <windows.h>

	// i hate you, microsoft
	#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
	#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
	#endif
#else
	#error Cannot determine platform
#endif

namespace dlg {

#ifdef DLG_OS_WIN
std::u16string toUtf16(const std::string_view& utf8)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes(&utf8.front(), &utf8.back() + 1);
}

bool checkAnsiSupport()
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

bool ansiSupported()
{
	static bool ret = checkAnsiSupport();
	return ret;
}

WORD reverseRGB(WORD rgb)
{
	static const WORD rev[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
	return rev[rgb];
}

void set(WORD& current, Background bg)
{
	current &= 0xFF0F;
	auto wbg = static_cast<WORD>(bg);
	if(wbg >= 100) current |= (0x8 | reverseRGB(wbg - 100)) << 4;
	else current |= reverseRGB(wbg - 40) << 4;
}

void set(WORD& current, Foreground fg)
{
	current &= 0xFFF0;
	auto wfg = static_cast<WORD>(fg);
	if(wfg >= 90) current |= (0x8 | reverseRGB(wfg - 90));
	else current |= reverseRGB(wfg - 30);
}

void set(WORD& current, Style style)
{
	if(style == Style::reset)
		current = (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

HANDLE consoleHandle(std::ostream& ostream)
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

#endif // DLG_OS_WIN

void write(std::ostream& ostream, std::string_view message)
{
#ifdef DLG_OS_WIN
	// This is needed to enable correct utf-8 output on the windows
	// command line. Because doing it the right way would be too simple, eh?
	auto handle = consoleHandle(ostream);
	if(handle) {
		auto str = toUtf16(message);
		if(::WriteConsoleW(handle, str.c_str(), str.size(), nullptr, nullptr))
			return;
	}
#endif

	ostream << message;
}

void defaultOutput(TextStyle style, std::string_view message)
{
	std::ostream& os = std::cout;

#ifdef DLG_OS_WIN
	auto handle = consoleHandle(os);
	if(ansiSupported()) {
		auto string = escapeSequence(style);
		string.append(message).append(escapeSequence({Foreground::none, Style::reset}));
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
	os << escapeSequence(style);
	os << message;
	os << escapeSequence({Foreground::none, Style::reset});
#endif
}

std::string defaultMessage(const Origin& origin, std::string_view msg)
{
	std::string ret;
	ret += "[";

	auto src = source_string(origin.source);
	if(!src.empty())
		ret.append(src).append(" | ");

	ret.append(origin.file).append(":").append(std::to_string(origin.line));
	ret.append("] ");

	if(!origin.expr.empty())
		ret.append("Assertion failed: '").append(origin.expr).append("' ");

	ret.append(msg).append("\n");
	return ret;
}

void defaultOutputHandler(const Origin& origin, std::string_view msg)
{
	// text styles used by the default logger for the different log levels.
	static constexpr TextStyle textStyles[] = {
		{Foreground::green, Style::italic},
		{Foreground::gray, Style::dim},
		{Foreground::cyan},
		{Foreground::yellow},
		{Foreground::red},
		{Foreground::red, Style::bold}
	};

	auto str = defaultMessage(origin, msg);
	auto textStyle = textStyles[static_cast<unsigned int>(origin.level)];
	defaultOutput(textStyle, str);
}

Source source(std::string_view str, Source::Force force, std::string_view sep) {
	Source ret {};
	ret.force = force;

	for(auto i = 0u; i < 3; ++i) {
		auto pos = str.find(sep);
		ret.src[i] = str.substr(0, pos);

		if(pos == str.npos)
			return ret;

		str.remove_prefix(pos + sep.length());
	}

	return ret;
}

std::string source_string(const Source& src, unsigned int lvl, std::string_view sep) {
	std::string ret;
	ret.reserve(50);

	bool first = true;
	lvl = std::min(lvl, 3u);
	for(auto i = 0u; i < lvl; ++i) {
		auto level = src.src[i];
		if(!level.empty()) {
			if(first) first = false;
			else ret += sep;
			ret += level;
		}
	}

	return ret;
}

void apply_source(const Source& src1, Source& src2, Source::Force force)
{
	using F = Source::Force;
	if(force == F::none)
		force = src1.force;

	if(force == F::full) {
		src2.src = src1.src;
		return;
	}

	for(auto i = 0u; i < 3u; ++i)
		if(!src1.src[i].empty() && (src2.src[i].empty() || force == F::override))
			src2.src[i] = src1.src[i];
}

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

std::string escapeSequence(TextStyle style)
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

OutputHandler outputHandler(OutputHandler set)
{
	auto& handler = outputHandler();
	auto cpy = std::move(handler);
	handler = set;
	return cpy;
}

OutputHandler& outputHandler()
{
	static OutputHandler handler = &defaultOutputHandler;
	return handler;
}

CurrentSource& current_source()
{
	thread_local CurrentSource source = {};
	return source;
}

void do_output(Origin& origin, std::string_view msg)
{
	return outputHandler()(origin, msg);
}

SourceGuard::SourceGuard(const Source& source, const char* func) : old(current_source())
{
	apply_source(source, current_source());
	current_source().func = func;
}

SourceGuard::~SourceGuard()
{
	current_source() = old;
}

} // namespace dlg

#endif // header guard
