// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#include <string_view>
#include <cstdarg>
#include <sstream>
#include <iostream>

#include <dlg/dlg.h>

class StreamBuffer : public std::basic_streambuf<char> {
public:
	StreamBuffer(char*& buf, std::size_t& size) : buf_(buf), size_(size) {
		setp(buf, buf + size); // we will only read from it
	}

	int_type overflow(int_type = traits_type::eof()) override {
		 tODO: use argument, call setp
		buf_ = static_cast<char*>(std::realloc(buf_, size_ * 2 + 1));
		size_ = buf_ ? size_ * 2 + 1 : 0;
		return buf_ ? traits_type::eof() : 0;
	}

protected:
	char*& buf_;
	size_t& size_;
};

void tformat(std::string_view fmt, std::ostream& output)
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
void tformat(std::string_view fmt, std::ostream& output, Arg&& arg, Args&&... args)
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
/// for an ostream. The returned pointer must not be freed but it is
/// only guaranteed to be valid until the next dlg logging/assertion call.
template<typename... Args>
const char* format(std::string_view fmt, Args&&... args)
{
	std::size_t* size;
	StreamBuffer buf(*dlg_thread_buffer(&size), *size);
	std::ostream output(&buf);
	tformat(fmt, output, std::forward<Args>(args)...);
	return *dlg_thread_buffer(nullptr);
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
