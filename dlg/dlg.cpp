// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef DLG_MAIN_CPP
#define DLG_MAIN_CPP

#include <iostream>
#include <sstream>
#include "dlg.hpp"
#include "color.hpp"

#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
	#define OS_UNIX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define OS_WIN
#else
	#error Cannot determine platform
#endif

namespace dlg {

StreamLogger defaultLogger {std::cout};
Logger* defaultSelector(const Origin&)
{
	return &defaultLogger;
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

Selector& selector()
{
	static Selector selector = &defaultSelector;
	return selector;
}

} // namespace dlg

#endif // header guard
