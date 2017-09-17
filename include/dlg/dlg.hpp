// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef _DLG_DLG_HPP_
#define _DLG_DLG_HPP_

#ifndef DLG_FMT_FUNC
	#define DLG_FMT_FUNC ::dlg::format
#endif

#include <dlg/dlg.h>
#include <streambuf>
#include <ostream>
#include <functional>

namespace dlg {

class TagsGuard {
public:
	TagsGuard(const char** tags, const char* func) : tags_(tags), func_(func) {
		while(*tags) {
			dlg_add_tag(*tags, func);
			++tags;
		}
	}

	~TagsGuard() {
		while(*tags_) {
			dlg_remove_tag(*tags_, func_);
			++tags_;
		}
	}

protected:
	const char** tags_;
	const char* func_;
};

#ifdef DLG_DISABLE
	#define dlg_tags() 
	#define dlg_tags_global()
#else
	#define dlg_tags(...) \
		const char* _dlgtags_[] = {__VA_ARGS__, nullptr}; \
		::dlg::TagsGuard _dlgltg_(_dlgtags_, __func__) 
	#define dlg_tags_global(...) \
		const char* _dlgtags_[] = {__VA_ARGS__, nullptr}; \
		::dlg::TagsGuard _dlggtg_(_dlgtags_.begin(), nullptr)
#endif

using Handler = std::function<void(const struct dlg_origin& origin, const char* str)>;

namespace detail {
	
void handler_wrapper(const struct dlg_origin* origin, const char* str, void* data) {
	auto& handler = *static_cast<Handler*>(data);
	try {
		handler(*origin, str);
	} catch(const std::exception& err) {
		fprintf(stderr, "dlg.hpp: handler has thrown exception: '%s'\n", err.what());
	} catch(...) {
		fprintf(stderr, "dlg.hpp: handler has thrown something else than std::exception");
	}
}
	
}

// Allows to set a std::function as dlg handler.
// The handler should not throw, all exceptions (and non-exceptions) are caught
// in a wrapper since they must not be passed through dlg (since it's c and dlg
// might be called from c code).
void set_handler(Handler handler) {
	static Handler handler_;
	handler_ = std::move(handler);
	dlg_set_handler(&detail::handler_wrapper, &handler_);
}

class StreamBuffer : public std::basic_streambuf<char> {
public:
	StreamBuffer(char*& buf, std::size_t& size) : buf_(buf), size_(size) {
		setp(buf, buf + size); // we will only read from it
	}

	int_type overflow(int_type ch = traits_type::eof()) override {
		if(pptr() >= epptr()) {
			buf_ = static_cast<char*>(std::realloc(buf_, size_ * 2 + 1));
			size_ = buf_ ? size_ * 2 + 1 : 0;
			setp(buf_, buf_ + size_);
		}
		if(!traits_type::eq_int_type(ch, traits_type::eof())) {
			*pptr() = ch;
		}
		return buf_ ? 2 * traits_type::eof() : traits_type::eof();
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
char* format(std::string_view fmt, Args&&... args)
{
	std::size_t* size;
	char** dbuf = dlg_thread_buffer(&size);
	StreamBuffer buf(*dbuf, *size);
	std::ostream output(&buf);
	tformat(fmt, output, std::forward<Args>(args)...);
	return *dlg_thread_buffer(nullptr);
}

} // namespace dlg

#endif // header guard