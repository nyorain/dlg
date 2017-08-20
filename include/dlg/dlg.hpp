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
#include <vector>
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

/// Tag type for string_view typesafety.
/// When using this type in a dlg log/assert macro it will be detected
/// and automatically added to tags.
struct Tag {
	std::string_view tag;
};

// Whole origin of a log/assertion call.
struct Origin {
	std::string_view file;
	unsigned int line;
	std::string_view func;
	Level level;
	std::vector<std::string_view> tags;
	std::string_view expr = nullptr; // assertion expression, nullptr for logging
};

// Used to store a currently applied tag.
// If the func member is not null it holds the __FUNC__ to which the tag
// should be applied.
struct CurrentTag : public Tag {
	const char* func {};
};

// Retrieves the current tags in the calling thread.
std::vector<CurrentTag*>& current_tags_ref();

// RAII guard that adds current tags for its lifetime
class TagsGuard {
public:
	TagsGuard(std::string_view tags, const char* func) : TagsGuard({tags}, func) {}
	TagsGuard(std::initializer_list<std::string_view> tags, const char* func = nullptr);
	~TagsGuard();

	TagsGuard(const TagsGuard&) = delete;
	TagsGuard& operator=(const TagsGuard&) = delete;

protected:
	std::vector<CurrentTag> tags_;
};

// Literals to easily create a tag
namespace literals {
	inline Tag operator"" _tag(const char* s, std::size_t n) { return {{s, n}}; }
} // namespace literals

// -- Logging macros --
#ifdef DLG_DISABLE

	#define dlg_trace(...) {}
	#define dlg_debug(...) {}
	#define dlg_info(...) {}
	#define dlg_warn(...) {}
	#define dlg_error(...) {}
	#define dlg_critical(...) {}
	#define dlg_assert_debug(...) {}
	#define dlg_assert_warn(...) {}
	#define dlg_assert_error(...) {}
	#define dlg_assert_critical(...) {}
	#define dlg_check(code) {}
	#define dlg_check_tagged(tags, code) {}
	#define dlg_tag(...) {}
	#define dlg_tag_global(...) {}
	#define dlg_log(...) {}
	#define dlg_assert(...) {}

#else // DLG_DISABLE

#if DLG_LOG_LEVEL <= DLG_LEVEL_TRACE
	#define dlg_trace(...) dlg::detail::do_log(DLG_FILE, __LINE__, \
		__FUNCTION__, dlg::Level::trace)(__VA_ARGS__)
#else
	#define dlg_trace(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_DEBUG
	#define dlg_debug(...) dlg::detail::do_log(DLG_FILE, __LINE__, \
		__FUNCTION__, dlg::Level::debug)(__VA_ARGS__)
#else
	#define dlg_debug(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_INFO
	#define dlg_info(...) dlg::detail::do_log(DLG_FILE, __LINE__, \
		__FUNCTION__, dlg::Level::info)(__VA_ARGS__)
#else
	#define dlg_info(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_WARN
	#define dlg_warn(...) dlg::detail::do_log(DLG_FILE, __LINE__, \
		__FUNCTION__, dlg::Level::warn)(__VA_ARGS__)
#else
	#define dlg_warn(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_ERROR
	#define dlg_error(...) dlg::detail::do_log(DLG_FILE, __LINE__,  \
		__FUNCTION__, dlg::Level::error)(__VA_ARGS__)
#else
	#define dlg_error(...) {}
#endif

#if DLG_LOG_LEVEL <= DLG_LEVEL_CRITICAL
	#define dlg_critical(...) dlg::detail::do_log(DLG_FILE, __LINE__, \
		__FUNCTION__, dlg::Level::critical)(__VA_ARGS__)
#else
	#define dlg_critical(...) {}
#endif

// -- Assertion macros --
#if DLG_ASSERT_LEVEL <= DLG_LEVEL_DEBUG
	#define dlg_assert_debug(...) \
		dlg::detail::do_assert(DLG_FILE, __LINE__, __FUNCTION__, dlg::Level::debug, \
			DLG_MM_FIRST_STR(__VA_ARGS__))(__VA_ARGS__)
#else
	#define dlg_assert_debug(expr, ...) {}
#endif

#if DLG_ASSERT_LEVEL <= DLG_LEVEL_WARN
	#define dlg_assert_warn(...) \
		dlg::detail::do_assert(DLG_FILE, __LINE__, __FUNCTION__, dlg::Level::warn, \
			DLG_MM_FIRST_STR(__VA_ARGS__))(__VA_ARGS__)
#else
	#define dlg_assert_warn(expr, ...) {}
#endif

#if DLG_ASSERT_LEVEL <= DLG_LEVEL_ERROR
	#define dlg_assert_error(...) \
		dlg::detail::do_assert(DLG_FILE, __LINE__, __FUNCTION__, dlg::Level::error, \
			DLG_MM_FIRST_STR(__VA_ARGS__))(__VA_ARGS__)
#else
	#define dlg_assert_error(...) {}
#endif

#if DLG_ASSERT_LEVEL <= DLG_LEVEL_CRITICAL
	#define dlg_assert_critical(...) \
		dlg::detail::do_assert(DLG_FILE, __LINE__, __FUNCTION__, dlg::Level::critical, \
			DLG_MM_FIRST_STR(__VA_ARGS__))(__VA_ARGS__)
#else
	#define dlg_assert_critical(expr, ...) {}
#endif

// -- Debug check scope --
#if DLG_CHECK
	#define dlg_check(code) { code }
	#define dlg_check_tagged(tags, code) {  \
		const ::dlg::TagsGuard _dlg_check_tag_guard(tags, __FUNCTION__); \
		code \
	}
#else
	#define dlg_check_unnamed(code) {}
	#define dlg_check(scope, code) {}
#endif

// -- Scope guard macros
#define dlg_tag(...) const ::dlg::TagsGuard _dlg_local_tag_guard({__VA_ARGS__}, __FUNCTION__);
#define dlg_tag_global(...) const ::dlg::TagsGuard _dlg_local_global_tag_guard({__VA_ARGS__});

// -- Utility macro magic --
#define DLG_MM_PASTE(x,y) x ## _ ## y
#define DLG_MM_EVALUATE(x,y) DLG_MM_PASTE(x, y)

#define DLG_MM_FIRST_STR(...) DLG_MM_FIRST_STR_HELPER(__VA_ARGS__, Must pass at least one argument)
#define DLG_MM_FIRST_STR_HELPER(first, ...) #first

// -- Default macros --
#define dlg_log(...) DLG_MM_EVALUATE(dlg, DLG_DEFAULT_LOG)(__VA_ARGS__)
#define dlg_assert(...) DLG_MM_EVALUATE(dlg_assert, DLG_DEFAULT_ASSERT)(__VA_ARGS__)

#endif // DLG_DISABLE

// -- Implementation --
// Bridge to output.hpp, will call the current outputHandler with the given arguments
void do_output(Origin& origin, std::string_view msg);

namespace detail {

// Strips the base path from the given file name.
// file is usually taken from __FILE__ and base DLG_BASE_PATH.
// If it detects that file is relative, will only strip away the relative beginning, otherwise
// will strip the given base path.
std::string_view strip_path(std::string_view file, std::string_view base);

template<typename... Args>
void output(Origin& origin, Args&&... args)
{
	// we only land in this function with 0 arguments if empty logging is disabled,
	// see the additional function below. This is more helpful than
	// an error about the fmt::format function
	static_assert(sizeof...(args) > 0, "Empty logging is not allowed");

	auto msg = fmt::format(std::forward<Args>(args)...);
	do_output(origin, msg);
}

template<typename... Args>
void output(Origin& origin, Tag tag, Args&&... args)
{
	origin.tags.push_back(tag.tag);
	output(origin, std::forward<Args>(args)...);
}

#ifndef DLG_DISABLE_EMPTY_LOG

inline void output(Origin& origin)
{
	do_output(origin, origin.expr.empty() ? "" : DLG_EMPTY_LOG);
}

#endif // DLG_DISABLE_EMPTY_LOG

struct LogDummy {
	Origin origin {};

	template<typename... Args>
	void operator()(Args&&... args) {
		output(origin, std::forward<Args>(args)...);
	}
};

struct AssertDummy {
	Origin origin;

	template<typename E, typename... Args>
	void operator()(const E& expr, Args&&... args) {
		if(!(expr))
			output(origin, std::forward<Args>(args)...);
	}
};

inline void init_origin(Origin& origin, std::string_view func)
{
#ifdef DLG_DEFAULT_TAGS
	auto dtags = DLG_DEFAULT_TAGS;
	origin.tags.insert(origin.tags.end(), dtags.begin(), dtags.end());
#endif

	for(auto ctag : current_tags_ref()) {
		if(!ctag->func || ctag->func == func) {
			origin.tags.push_back(ctag->tag);
		}
	}
}

// -- Macro functions --
inline LogDummy do_log(std::string_view file, unsigned int line,
	std::string_view func, Level level)
{
	LogDummy ret;
	ret.origin = {file, line, func, level, {}};
	init_origin(ret.origin, func);
	return ret;
}

inline AssertDummy do_assert(std::string_view file, unsigned int line,
	std::string_view func, Level level, const char* expr)
{
	AssertDummy ret;
	ret.origin = {file, line, func, level, {}, expr};
	init_origin(ret.origin, func);
	return ret;
}

} // namespace detail
} // namespace dlg

#endif // header guard
