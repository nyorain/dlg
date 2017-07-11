// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef DLG_INC_HPP
#define DLG_INC_HPP

#pragma once

#include <string>
#include <functional>
#include <ostream>
#include <string>
#include <array>

#include "fmt.hpp"
#include "config.hpp"

namespace dlg {

// Logging level/importance.
// Use Level::warn for warnings e.g. deprecated or not implemented stuff.
// Use Level::error for e.g. system errors, rather unexpected events that will have an effect
// Use Level::critical for e.g. critical logic failures and really impossible situations
enum class Level {
	trace = 0, // temporary used debug, e.g. to check if control reaches function
	debug, // general debugging, prints e.g. all major events
	info, // general useful information
	warn, // warning, something went wrong but might have no (really bad) side effect
	error, // something really went wrong; expect serious issues
	critical // fatal error; application is likely to crash/exit
};

inline bool operator<(Level a, Level b)
	{ return static_cast<unsigned int>(a) < static_cast<unsigned int>(b); }
inline bool operator<=(Level a, Level b)
	{ return static_cast<unsigned int>(a) <= static_cast<unsigned int>(b); }
inline bool operator>(Level a, Level b)
	{ return static_cast<unsigned int>(a) > static_cast<unsigned int>(b); }
inline bool operator>=(Level a, Level b)
	{ return static_cast<unsigned int>(a) >= static_cast<unsigned int>(b); }

// Object representing the ith level of an origin source.
// Used as dummy objects detected by templates for the different source levels.
template<unsigned int I, typename = std::enable_if_t<I < 3>>
struct Src { std::string_view name; };

// Represents a source name for a call to log/assert.
// The string_view array represents the differents source levels, e.g. {"proj", "class", "func"}.
struct Source {
	// Specifies in which way a source should be applied.
	enum class Force {
		// Unspecified, meta value
		none = 0,

		// Only non-empty src levels are applied to empty dst levels.
		no_override,

		// All non-empty src levels are applied, they may override
		override,

		// All src levels are applied, i.e. the source if fully assigned.
		full
	};

	template<unsigned int I1 = 0>
	Source(Src<I1> src1 = {}, Force f = Force::override) {
		src[I1] = src1.name;
		force = f;
	}

	template<unsigned int I1, unsigned int I2>
	Source(Src<I1> src1, Src<I2> src2, Force f = Force::override) {
		src[I1] = src1.name;
		src[I2] = src2.name;
		force = f;
	}

	template<unsigned int I1, unsigned int I2, unsigned int I3>
	Source(Src<I1> src1, Src<I2> src2, Src<I3> src3, Force f = Force::override) {
		src[I1] = src1.name;
		src[I2] = src2.name;
		src[I3] = src3.name;
		force = f;
	}

	Source(std::string_view name, Force force = Force::override,
			std::string_view sep = DLG_DEFAULT_SEP);

	// Represents the source levels.
	// Usually src[0] is project level, src[1] module level and src[2] scope level.
	// E.g. something like {"dlg", "Source", "Source"} for the sources constructor,
	// but they can also be used in a more abstract matter (i.e. grouping different
	// parts of a project independent from class/file/function).
	std::array<std::string_view, 3> src {};

	// In which way this source is applied.
	// The attribute is only used in certain source object usages.
	Force force {Force::override};
};

// Whole origin of a log/assertion call.
struct Origin {
	std::string_view file;
	unsigned int line;
	Level level;
	Source source;
	std::string_view expr = nullptr; // assertion expression, nullptr for logging
};

// Copies the valies src fields from src1 to src2, leaves the other ones untouched.
// Only if override is false, will not override already set fields in src1.
// The Force parameter can be used to override the force used to apply the source,
// if it is Force::none the force value from src1 will be used.
DLG_API void apply_source(const Source& src1, Source& src2,
		Source::Force force = Source::Force::none);

// Parses a string like project::module::scope into a Source object.
// Separator represents the separator between the source scopes.
// E.g. source("project.module.some_scope", ".") => {"project", "module", "some_scope"}
DLG_API Source source(std::string_view str, Source::Force force = Source::Force::override,
		std::string_view sep = DLG_DEFAULT_SEP);

// Builds a source string from a given source up to the given lvl.
// E.g. source_string({"project", "module", "some_scope"}, ".", 2) => "project.module". If lvl
// would be 3, the source string would hold some_scope as well.
DLG_API std::string source_string(const Source& src,
	unsigned int lvl = 3u, std::string_view sep = DLG_DEFAULT_SEP);

Source::Source(std::string_view name, Force force, std::string_view sep)
{
	*this = source(name, force, sep);
}

// Base logger class.
// Must implement the write function that outputs the given string.
// Might override the output function to alter the information presentation.
class Logger {
public:
	virtual ~Logger() = default;
	virtual void write(const std::string& string) = 0;
	virtual void output(const Origin& origin, std::string msg);
};

// Logger implementation that simply writes a string to a given std::ostream.
class StreamLogger : public Logger {
public:
	inline StreamLogger(std::ostream& os) : ostream(&os) {}
	DLG_API void write(const std::string& string) override;
	std::ostream* ostream;
};

// The selector function.
// This function is called when dlg is used for logging/assertion and must return
// the Logger dlg should use for the given origin. If nothing should be outputted,
// can return a nullptr (in which case the log/assertion error is discarded).
// Can e.g. use the origin to channel different origins to different outputs.
using Selector = std::function<Logger*(const Origin&)>;

/// Sets a new Selector. Must be valid i.e. not an empty std::function object.
/// Returns the old selector.
Selector selector(Selector set);

/// Receives the current selector.
Selector& selector();

// thread local static variable getter that can be set to the current source.
// even if the source levels are set they can be overriden when logging, they just
// serve as fallback.
Source& current_source();

// Constructs a source object that clears any applied source.
Source clear_source()
{
	Source s;
	s.force = Source::Force::full;
	return s;
}

// Default stream logger returned by the default selector.
extern StreamLogger defaultLogger;

// RAII guard that sets the thread_local source of dlg for its liftime.
struct SourceGuard {
	DLG_API SourceGuard(const Source& source);
	DLG_API ~SourceGuard();

	SourceGuard(const SourceGuard&) = delete;
	SourceGuard& operator=(const SourceGuard&) = delete;

	Source old {};
};

// Literals to easily create source or source level objects.
// Can e.g. be used like this: dlg_log("my_project"_project, "my_class"_src1, "A string");
// Alternatively, a string can be parsed: dlg_log("my_proj::my_mod::my_scope"_src, "A string");
// If you want to use those, 'using namespace dlg::literals'.
namespace literals {
	inline Src<0> operator"" _src0(const char* s, std::size_t n) { return {{s, n}}; }
	inline Src<1> operator""_src1(const char* s, std::size_t n) { return {{s, n}}; }
	inline Src<2> operator"" _src2(const char* s, std::size_t n) { return {{s, n}}; }

	inline Src<0> operator"" _project(const char* s, std::size_t n) { return {{s, n}}; }
	inline Src<1> operator""_module(const char* s, std::size_t n) { return {{s, n}}; }
	inline Src<2> operator"" _scope(const char* s, std::size_t n) { return {{s, n}}; }

	inline Source operator""_src(const char* s, std::size_t n) { return source({s, n}); }
}

// Strips the base path from the given file name.
// file is usually taken from __FILE__ and base DLG_BASE_PATH.
// If it detects that file is relative, will only strip away the relative beginning, otherwise
// will strip the given base path.
DLG_API std::string_view strip_path(std::string_view file, std::string_view base);

// -- Logging macros --
#if DLG_LOG_LEVEL <= DLG_LEVEL_TRACE
	#define dlg_trace(...) dlg::do_log(DLG_FILE, __LINE__, dlg::Level::trace, __VA_ARGS__)
#else
	#define dlg_trace(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_DEBUG
	#define dlg_debug(...) dlg::do_log(DLG_FILE, __LINE__, dlg::Level::debug, __VA_ARGS__)
#else
	#define dlg_debug(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_INFO
	#define dlg_info(...) dlg::do_log(DLG_FILE, __LINE__, dlg::Level::info, __VA_ARGS__)
#else
	#define dlg_info(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_WARN
	#define dlg_warn(...) dlg::do_log(DLG_FILE, __LINE__, dlg::Level::warn, __VA_ARGS__)
#else
	#define dlg_warn(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_ERROR
	#define dlg_error(...) dlg::do_log(DLG_FILE, __LINE__, dlg::Level::error, __VA_ARGS__)
#else
	#define dlg_error(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_CRITICAL
	#define dlg_critical(...) dlg::do_log(DLG_FILE, __LINE__, dlg::Level::critical, __VA_ARGS__)
#else
	#define dlg_critical(...) {}
#endif

// -- Assertion macros --
#if DLG_ASSERT_LEVEL <= DLG_LEVEL_DEBUG
	#define dlg_assert_debug(expr, ...) \
		if(!(expr)) dlg::do_assert(DLG_FILE, __LINE__, dlg::Level::debug, #expr, __VA_ARGS__)
#else
	#define dlg_assert_debug(expr, ...) {}
#endif

#if DLG_ASSERT_LEVEL <= DLG_LEVEL_WARN
	#define dlg_assert_warn(expr, ...) \
		if(!(expr)) dlg::do_assert(DLG_FILE, __LINE__, dlg::Level::warn, #expr, __VA_ARGS__)
#else
	#define dlg_assert_warn(expr, ...) {}
#endif

#if DLG_ASSERT_LEVEL <= DLG_LEVEL_ERROR
	#define dlg_assert_error(expr, ...) \
		if(!(expr)) dlg::do_assert(DLG_FILE, __LINE__, dlg::Level::error, #expr, __VA_ARGS__)
#else
	#define dlg_assert_error(expr, ...) {}
#endif

#if DLG_ASSERT_LEVEL <= DLG_LEVEL_CRITICAL
	#define dlg_assert_critical(expr, ...) \
		if(!(expr)) dlg::do_assert(DLG_FILE, __LINE__, dlg::Level::critical, #expr, __VA_ARGS__)
#else
	#define dlg_assert_critical(expr, ...) {}
#endif

// -- Debug check scope --
#if DLG_CHECK
	#define dlg_check_unnamed(code) { code }
	#define dlg_check(scope, code) {  \
		::dlg::SourceGuard dlg_check_scope_guard(scope); \
		code \
	}
#else
	#define dlg_check_unnamed(code) {}
	#define dlg_check(scope, code) {}
#endif

// -- Utility macro magic --
#define DLG_MM_PASTE(x,y) x ## _ ## y
#define DLG_MM_EVALUATE(x,y) DLG_MM_PASTE(x, y)

// -- Default macros --
#define dlg_log(...) DLG_MM_EVALUATE(dlg, DLG_DEFAULT_LOG)(__VA_ARGS__)
#define dlg_assert(expr, ...) DLG_MM_EVALUATE(dlg_assert, DLG_DEFAULT_ASSERT)(expr, __VA_ARGS__)


// -- Implementation --
// -- Output, Source detection --
template<typename... Args>
void output(Origin& origin, Src<0> src0, Args&&... args)
{
	origin.source.src[0] = src0.name;
	output(origin, std::forward<Args>(args)...);
}

template<typename... Args>
void output(Origin& origin, Src<1> src1, Args&&... args)
{
	origin.source.src[1] = src1.name;
	output(origin, std::forward<Args>(args)...);
}

template<typename... Args>
void output(Origin& origin, Src<2> src2, Args&&... args)
{
	origin.source.src[2] = src2.name;
	output(origin, std::forward<Args>(args)...);
}

template<typename... Args>
void output(Origin& origin, Source src, Args&&... args)
{
	apply_source(src, origin.source);
	output(origin, std::forward<Args>(args)...);
}

template<typename... Args>
void output(Origin& origin, Args&&... args)
{
	auto logger = selector()(origin);
	if(!logger)
		return;

	auto content = fmt::format(std::forward<Args>(args)...);
	logger->output(origin, content);
}

// -- Macro functions --
template<typename... Args>
void do_log(std::string_view file, unsigned int line, Level level, Args&&... args)
{
	Origin origin = {file, line, level, {}};
	apply_source(current_source(), origin.source);
	output(origin, std::forward<Args>(args)...);
}

template<typename... Args>
void do_assert(std::string_view file, unsigned int line, Level level,
	const char* expr, Args&&... args)
{
	Origin origin = {file, line, level, {}};
	origin.expr = expr;
	apply_source(current_source(), origin.source);
	output(origin, std::forward<Args>(args)...);
}

} // namespace dlg

#if DLG_HEADER_ONLY
	#include "dlg.cpp"
#endif

#endif // header guard
