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
#include "color.hpp"

#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
	#define OS_UNIX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define OS_WIN
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#error Cannot determine platform
#endif

namespace dlg {

inline std::u16string toUtf16(const std::string& utf8)
{
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	return converter.from_bytes(utf8);
}

DLG_API void StreamLogger::write(const std::string& string)
{
	#ifdef OS_WIN
		static const auto default_out_buf = std::cout.rdbuf();
		static const auto default_err_buf = std::cerr.rdbuf();
		static const auto default_log_buf = std::clog.rdbuf();

		HANDLE handle;

		if(ostream->rdbuf() == default_out_buf)
			handle = ::GetStdHandle(STD_OUTPUT_HANDLE);

		if(ostream->rdbuf() == default_err_buf || os.rdbuf() == default_log_buf)
			handle = ::GetStdHandle(STD_ERROR_HANDLE);

		if(handle) {
			auto str2 = nytl::toUtf16(string);
			::WriteConsoleW(handle, str2.c_str(), str2.size(), nullptr, nullptr);
			return;
		}
	#endif

	*ostream << string;
}

DLG_API void Logger::output(const Origin& origin, std::string msg)
{
	std::stringstream sstream; // TODO cache

	{
		sstream << apply(textStyles[static_cast<unsigned int>(origin.level)]);
		sstream << "[";

		auto src = source_string(origin.source);
		if(!src.empty())
			sstream << src << " | ";

		sstream << origin.file << ":" << origin.line;
		sstream << "] ";

		if(!origin.expr.empty())
			sstream << "Assertion failed: '" << origin.expr << "' ";

		sstream << msg;
		sstream << apply(TextStyle{Foreground::none, Style::reset});
		sstream << "\n";
	}

	write(sstream.str());
}

DLG_API Source source(std::string_view str, std::string_view sep) {
	Source ret {};

	for(auto i = 0u; i < 3; ++i) {
		auto pos = str.find(sep);
		ret.src[i] = str.substr(0, pos);

		if(pos == str.npos)
			return ret;

		str.remove_prefix(pos + 2);
	}

	return ret;
}

DLG_API std::string source_string(const Source& src, std::string_view sep, unsigned int lvl) {
	std::string ret;
	ret.reserve(50);

	bool first = true;
	lvl = std::min(lvl, 3u);
	for(auto i = 0u; i < 3u; ++i) {
		auto lvl = src.src[i];
		if(!lvl.empty()) {
			if(first) first = false;
			else ret += sep;
			ret += lvl;
		}
	}

	return ret;
}

DLG_API void apply_source(const Source& src1, Source& src2, bool force)
{
	for(auto i = 0u; i < 3u; ++i)
		if((src2.src[i].empty() || force) && !src1.src[i].empty())
			src2.src[i] = src1.src[i];
}

DLG_API std::string_view strip_path(std::string_view file, std::string_view base)
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

// linux
DLG_API std::string apply(TextStyle style)
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

Selector selector(Selector set)
{
	auto& sel = selector();
	auto cpy = std::move(sel);
	sel = set;
	return cpy;
}

Logger* defaultSelector(const Origin&)
{
	return &defaultLogger;
}

#if defined(DLG_IMPLEMENTATION) || !DLG_HEADER_ONLY
	StreamLogger defaultLogger {std::cout};
	Selector& selector()
	{
		static Selector selector = &defaultSelector;
		return selector;
	}

	Source& current_source()
	{
		thread_local Source source = {};
		return source;
	}
#endif

DLG_API SourceGuard::SourceGuard(Source source, bool full) : old(current_source())
{
	if(full) {
		current_source() = source;
		return;
	}

	apply_source(source, current_source());
}

DLG_API SourceGuard::~SourceGuard()
{
	current_source() = old;
}

} // namespace dlg

#endif // header guard
