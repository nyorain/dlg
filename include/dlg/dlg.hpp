// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef _DLG_DLG_HPP_
#define _DLG_DLG_HPP_

// XXX: override the default (via undef) if a dlg.h was included before this file?

// By default this header automatically uses a different, typesafe formatting
// function. Make sure to never include dlg.h in your translation unit before 
// including dlg.hpp to make this work.
// The new formatting function works like a type-safe version of printf, see dlg::format.
#ifndef DLG_FMT_FUNC
	#define DLG_FMT_FUNC ::dlg::detail::tlformat
#endif

// The default string to replace by the dlg::*format functions.
// Used as default by tlformat (set as new DLG_FMT_FUNC) or dlg::format.
// If a custom replace string is required in certain situations without
// overriding this macro, use dlg::rformat or dlg::gformat. 
#ifndef DLG_FORMAT_DEFAULT_REPLACE
	#define DLG_FORMAT_DEFAULT_REPLACE "{}"
#endif

#include <dlg/dlg.h>
#include <streambuf>
#include <ostream>
#include <functional>
#include <cstring>
#include <sstream>

namespace dlg {

// Sets dlg tags on its construction and removes them on its destruction.
// Instead of explicitly constructing an object, just use the dlg_tags and
// dlg_tags_global macros which will construct one in the current scope.
// Just forwards the arguments on construction to dlg_add_tag, so if func
// is nullptr the tags will be applied even to called functions from the current
// scope, otherwise only to calls coming directly from the current function.
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
	// Constructs a dlg::TagsGuard in the current scope, passing correctly the 
	// current function, i.e. only dlg calls made from other functions
	// that are called in the current scope will not use the given tags.
	// Expects the tags to be set as parameters like this:
	// ```dlg_tags("tag1", "tag2")```.
	#define dlg_tags(...) 

	// Constructs a dlg::TagsGuard in the current scope, passing nullptr as func.
	// This means that even dlg calls made from other functions called in the current
	// scope will use those tags.
	// Expects the tags to be set as parameters like this:
	// ```dlg_tags_global("tag1", "tag2")```.
	#define dlg_tags_global(...)
#else
	#define dlg_tags(...) \
		const char* _dlgtags_[] = {__VA_ARGS__, nullptr}; \
		::dlg::TagsGuard _dlgltg_(_dlgtags_, __func__) 
	#define dlg_tags_global(...) \
		const char* _dlgtags_[] = {__VA_ARGS__, nullptr}; \
		::dlg::TagsGuard _dlggtg_(_dlgtags_.begin(), nullptr)
#endif

/// Alternative output handler that allows to e.g. set lambdas or member functions.
using Handler = std::function<void(const struct dlg_origin& origin, const char* str)>;

// Allows to set a std::function as dlg handler.
// The handler should not throw, all exceptions (and non-exceptions) are caught
// in a wrapper since they must not be passed through dlg (since it's c and dlg
// might be called from c code).
void set_handler(Handler handler);

// TODO: maybe don't use exceptions for wrong formats?

/// Generic version of dlg::format, allows to set the special string sequence
/// to be replaced with arguments instead of using DLG_FORMAT_DEFAULT_REPLACE.
/// Simply replaces all occurrences of 'replace' in 'fmt' with the given
/// arguments (as printed using operator<< with an ostream) in order and 
/// prints everything to the given ostream.
/// Throws std::invalid_argument if there are too few or too many arguments.
/// If you want to print the replace string without being replaced, wrap
/// it into backslashes (\\). If you want to print your own types, simply 
/// overload operator<< for ostream correctly. The replace string itself
/// must not be a backslash character.
///  - gformat("%", "number: '%', string: '%'", 42, "aye"); -> "number: '42', string: 'aye'"
///  - gformat("{}", "not replaced: \\{}\\"); -> "not replaced: {}"
///  - gformat("$", "{} {}", 1); -> std::invalid_argument, too few arguments
///  - gformat("$", "{}", 1, 2); -> std::invalid_argument, too many arguments
///  - gformat("$", "{} {}", std::setw(5), 2); -> "     2"
template<typename Arg, typename... Args>
void gformat(std::ostream& os, const char* replace, const char* fmt, Arg&& arg, Args&&... args);
void gformat(std::ostream& os, const char* replace, const char* fmt);

/// Simply calls gformat with a local stringstream and returns the stringstreams
/// contents.
template<typename... Args>
std::string rformat(const char* replace, const char* fmt, Args&&... args) {
	std::stringstream sstream;
	gformat(sstream, replace, fmt, std::forward<Args>(args)...);
	return sstream.str();
}


/// Simply calls rformat with DLG_FORMAT_DEFAULT_REPLACE (defaulted to '{}') as
/// replace string.
template<typename... Args>
std::string format(const char* fmt, Args&&... args) {
	return rformat(DLG_FORMAT_DEFAULT_REPLACE, fmt, std::forward<Args>(args)...);
}

// - Private interface & implementation -
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

// Implements std::basic_streambuf into the dlg_thread_buffer.
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

// Like std::strstr but only matches if target is not wrapped in backslashes,
// otherwise prints the target.
const char* find_next(std::ostream& os, const char*& src, const char* target) {
	auto len = std::strlen(target);
	const char* next = std::strstr(src, target);
	while(next && next > src && *(next - 1) == '\\' && *(next + len) == '\\') {
		os.write(src, next - src - 1);
		os.write(target, len);
		src = next + len + 1;
		next = std::strstr(next + 1, target);
	}

	return next;
}

// Used as DLG_FMT_FUNC, uses a threadlocal stringstream to not allocate
// a new buffer on every call
template<typename... Args>
const char* tlformat(const char* fmt, Args&&... args) {
	std::size_t* size;
	char** dbuf = dlg_thread_buffer(&size);
	detail::StreamBuffer buf(*dbuf, *size);
	std::ostream output(&buf);
	gformat(output, DLG_FORMAT_DEFAULT_REPLACE, fmt, std::forward<Args>(args)...);
	output.put('\0'); // terminating null char since we will use the buffer as string
	return *dlg_thread_buffer(nullptr);
}

} // namespace detail

void gformat(std::ostream& os, const char* replace, const char* fmt) {
	if(detail::find_next(os, fmt, replace)) {
		throw std::invalid_argument("Too few arguments given to format");
	}

	os << fmt;
}

template<typename Arg, typename... Args>
void gformat(std::ostream& os, const char* replace, const char* fmt, Arg&& arg, Args&&... args) {
	const char* next = detail::find_next(os, fmt, replace);
	if(!next) {
		throw std::invalid_argument("Too many arguments to format supplied");
	}

	os.write(fmt, next - fmt); // XXX: could cause problems since unformatted?
	os << std::forward<Arg>(arg);
	auto len = std::strlen(replace);
	return gformat(os, replace, next + len, std::forward<Args>(args)...);
}

void set_handler(Handler handler) {
	static Handler handler_;
	handler_ = std::move(handler);
	dlg_set_handler(&detail::handler_wrapper, &handler_);
}

} // namespace dlg

#endif // header guard