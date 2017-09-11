// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <string_view>
#include <cstdarg>
#include <sstream>
#include <iostream>

void tformat(std::string_view fmt, std::stringstream& output)
{
	// check that no '{}' is left
	bool pending = false;
	for(auto c : fmt) {
		if(pending) {
			pending = false;
			if(c == '}') {
				throw std::invalid_argument("Too few arguments given to format");
			}
		} else if(c == '{') {
			pending = true;
		}
	}
	
	output << fmt;
}

template<typename Arg, typename... Args>
void tformat(std::string_view fmt, std::stringstream& output, Arg&& arg, Args&&... args)
{
	bool pending = false;
	for(auto i = 0u; i < fmt.size(); ++i) {
		auto c = fmt[i];
		if(pending) {
			pending = false;
			if(c == '}') {
				output << std::forward<Arg>(arg);
				return tformat(fmt.substr(i + 1), output, std::forward<Args>(args)...);
			} else {
				output << "{" << c;
			}
		} else if(c == '{') {
			pending = true;
		} else {
			output << c;
		}
	}
	
	// we must not finish the string when there are arguments lefts
	throw std::invalid_argument("Too many arguments to format supplied");
}

/// Formats the given string with the given arguments.
/// Simply replaces '{}' with the arguments in order.
/// Throws std::invalid_argument if the string does not match
/// the arugment count. The arguments must implement the << operator
/// for an ostream.
template<typename... Args>
std::string format(std::string_view fmt, Args&&... args)
{
	std::stringstream output;
	tformat(fmt, output, std::forward<Args>(args)...);
	return output.str();
}

/// printf-like format
#ifdef __GNUC__
	#define PRINTF_ATTRIB(a, b) __attribute__ ((format (printf, a, b)))
#else
	#define PRINTF_ATTRIB(a, b)
#endif

std::string pformat(const char* str, ...) PRINTF_ATTRIB(1, 2);
std::string pformat(const char* str, ...)
{
	va_list vlist;
	va_start(vlist, str);
	
	va_list vlistcopy;
	va_copy(vlistcopy, vlist);

	auto size = std::vsnprintf(nullptr, 0, str, vlist);
	va_end(vlist);

	std::string ret;
	ret.resize(size + 1);
	std::vsnprintf(&ret[0], ret.size(), str, vlistcopy);
	va_end(vlistcopy);
	
	return ret;
}

// TODO: remove test area
int main()
{
	std::cout << format("ayy {} {}\n", 3, "test");
	std::cout << pformat("ayy %d %s %d\n", 3, "test", 2);
}
