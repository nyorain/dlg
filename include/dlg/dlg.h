// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef _DLG_DLG_H_
#define _DLG_DLG_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

// Hosted at https://github.com/nyorain/dlg.
// There are examples and documentation. 
// Issue reports and contributions appreciated.

#ifdef __cplusplus
extern "C" {
#endif

// - CONFIG -
// Define this macro to make all dlg macros have no effect at all
// #define DLG_DISABLE

// the log/assertion levels below which logs/assertions are ignored
#ifndef DLG_LOG_LEVEL
	#define DLG_LOG_LEVEL dlg_level_trace
#endif

#ifndef DLG_ASSERT_LEVEL
	#define DLG_ASSERT_LEVEL dlg_level_trace
#endif

// the assert level of dlg_assert
#ifndef DLG_DEFAULT_ASSERT
	#define DLG_DEFAULT_ASSERT dlg_level_error
#endif

// evaluated to the 'file' member in dlg_origin
#ifndef DLG_FILE
	#define DLG_FILE dlg__strip_root_path(__FILE__, DLG_BASE_PATH)

	// the base path stripped from __FILE__. If you don't override DLG_FILE set this to 
	// the project root to make 'main.c' from '/some/bullshit/main.c'
	#ifndef DLG_BASE_PATH
		#define DLG_BASE_PATH ""
	#endif
#endif

// Default tags applied to all logs/assertions (in the defining file).
// Must be in format ```#define DLG_DEFAULT_TAGS "tag1", "tag2", ``` (with the last comma!),
// or just nothing (as defaulted here)
#ifndef DLG_DEFAULT_TAGS
	#define DLG_DEFAULT_TAGS
#endif

// The function used for formatting. Can have any signature, but must be callable with
// the arguments the log/assertions macros are called with. Must return a const char*
// that will not be freed by dlg, the formatting function must keep track of it.
// The formatting function might use dlg_thread_buffer or a custom owned buffer.
// The returned const char* has to be valid until the dlg log/assertion ends.
// Usually a c function with ... (i.e. using va_list) or a variadic c++ template do
// allow formatting.
#ifndef DLG_FMT_FUNC
	#define DLG_FMT_FUNC dlg__printf_format
#endif

// Don't use this, only if you know what you are doing!
// Could e.g. on windows when compiling with msvc and using a shared library of dlg
// be set to __declspec(import)
#ifndef DLG_API
	#define DLG_API
#endif

// - utility -
// two methods needed since cplusplus does not support compound literals
// and c does not support uniform initialization
#ifdef __cplusplus
	typedef const char* const DLG_STRL_[];
	#define DLG_CREATE_TAGS(...) DLG_STRL_{DLG_DEFAULT_TAGS NULL, __VA_ARGS__, NULL}
#else
	#define DLG_CREATE_TAGS(...) (const char*[]) {DLG_DEFAULT_TAGS NULL, __VA_ARGS__, NULL}
#endif

#ifdef __GNUC__
	#define DLG_PRINTF_ATTRIB(a, b) __attribute__ ((format (printf, a, b)))
#else
	#define DLG_PRINTF_ATTRIB(a, b)
#endif

// Represents the importance of a log/assertion call.
enum dlg_level {
	dlg_level_trace = 0, // temporary used debug, e.g. to check if control reaches function
	dlg_level_debug, // general debugging, prints e.g. all major events
	dlg_level_info, // general useful information
	dlg_level_warn, // warning, something went wrong but might have no (really bad) side effect
	dlg_level_error, // something really went wrong; expect serious issues
	dlg_level_fatal // critical error; application is likely to crash/exit
};

// Holds various information associated with a log/assertion call.
// Forwarded to the output handler.
struct dlg_origin {
	const char* file;
	unsigned int line;
	const char* func;
	enum dlg_level level;
	const char** tags; // null-terminated
	const char* expr; // assertion expression, otherwise null
};

// Type of the output handler, see dlg_set_handler.
typedef void(*dlg_handler)(const struct dlg_origin* origin, const char* string, void* data);

#ifdef DLG_DISABLE
	// Tagged/Untagged logging with variable level
	// Tags must always be in the format `("tag1", "tag2")` (including brackets)
	#define dlg_log(level, ...)
	#define dlg_logt(level, tags, ...)

	// Dynamic level assert macros in various versions for additional arguments
	#define dlg_assertl(level, expr) // assert without tags/message
	#define dlg_assertlt(level, tags, expr) // assert with tags
	#define dlg_assertlm(level, expr, ...) // assert with message
	#define dlg_assertltm(level, tags, expr, ...) // assert with tags & message

	// Sets the handler that is responsible for formatting and outputting log calls.
	// This function is not thread safe and the handler is set globally.
	// The handler must not change dlg tags or call a dlg macro theirself.
	// Can also be used various other things such as dealing with critical failed
	// assertions or filtering calls based on the passed tags.
	// The default handler is dlg_default_output (see its doc for more info).
	// If using c++ make sure the registered handler cannot throw (since this is UB)
	// e.g. by wrapping everything into a try-catch blog.
	inline void dlg_set_handler(dlg_handler handler, void* data) {}

	// The default output handler.
	// Only use this to reset the output handler, prefer to use 
	// dlg_generic_output (from output.h) which this function simply calls.
	// It also flushes the stream used.
	inline void dlg_default_output(const struct dlg_origin* o, const char* str, void* data) {}

	// Adds the given tag associated with the given function to the thread specific list.
	// If func is not NULL the tag will only applied to calls from the same function.
	// Remove the tag again calling dlg_remove_tag (with exactly the same pointers!).
	// Does not check if the tag is already present.
	inline void dlg_add_tag(const char* tag, const char* func) {}

	// Removes a tag added with dlg_add_tag (has no effect for tags no present).
	// The pointers must be exactly the same pointers that were supplied to dlg_add_tag,
	// this function will not check using strcmp. When the same tag/func combination
	// is added multiple times, this function remove exactly one candidate, it is
	// undefined which. Returns whether a tag was found (and removed).
	inline bool dlg_remove_tag(const char* tag, const char* func) { return true; }

	// Returns the thread-specific buffer and its size for dlg.
	// The buffer should only be used by formatting functions.
	// The buffer can be reallocated and the size changed, just make sure
	// to update both values correctly.
	inline char** dlg_thread_buffer(size_t** size) {}

	// Frees all thread-specific resources dlg allocated in the calling thread.
	// When dlg is used again they will be reinitialized so this could e.g. be called
	// when dlg is never again used (or at least not for a long time, it's not broken 
	// after this call). If a thread uses dlg and never calls this, resources may leak.
	inline void dlg_cleanup(void) {}

#else // DLG_DISABLE
	#define dlg_log(level, ...) if(level >= DLG_LOG_LEVEL) \
		dlg__do_log(level, DLG_CREATE_TAGS(NULL), DLG_FILE, __LINE__, __func__,  \
		DLG_FMT_FUNC(__VA_ARGS__), NULL)
	#define dlg_logt(level, tags, ...) if(level >= DLG_LOG_LEVEL) \
		dlg__do_log(level, DLG_CREATE_TAGS tags, DLG_FILE, __LINE__, __func__, \
		DLG_FMT_FUNC(__VA_ARGS__), NULL)

	#define dlg_assertl(level, expr) if(level >= DLG_ASSERT_LEVEL && !(expr)) \
		dlg__do_log(level, DLG_CREATE_TAGS(NULL), DLG_FILE, __LINE__, __func__, NULL, #expr)
	#define dlg_assertlt(level, tags, expr) if(level >= DLG_ASSERT_LEVEL && !(expr)) \
		dlg__do_log(level, DLG_CREATE_TAGS tags, DLG_FILE, __LINE__, __func__, NULL, #expr)
	#define dlg_assertlm(level, expr, ...) if(level >= DLG_ASSERT_LEVEL && !(expr)) \
		dlg__do_log(level, DLG_CREATE_TAGS(NULL), DLG_FILE, __LINE__, __func__,  \
		DLG_FMT_FUNC(__VA_ARGS__), #expr)
	#define dlg_assertltm(level, tags, expr, ...) if(level >= DLG_ASSERT_LEVEL && !(expr)) \
		dlg__do_log(level, DLG_CREATE_TAGS tags, DLG_FILE, __LINE__,  \
		__func__, DLG_FMT_FUNC(__VA_ARGS__), #expr)

	DLG_API void dlg_set_handler(dlg_handler handler, void* data);
	DLG_API void dlg_default_output(const struct dlg_origin* origin, const char* string, void* stream);
	DLG_API void dlg_add_tag(const char* tag, const char* func);
	DLG_API bool dlg_remove_tag(const char* tag, const char* func);
	DLG_API char** dlg_thread_buffer(size_t** size);
	DLG_API void dlg_cleanup();

	// - Private interface: not part of the abi/api but needed in macros -
	// Formats the given format string and arguments as printf would, uses the thread buffer.
	DLG_API const char* dlg__printf_format(const char* format, ...) DLG_PRINTF_ATTRIB(1, 2);
	DLG_API void dlg__do_log(enum dlg_level lvl, const char* const* tags, const char* file, int line,
		const char* func, const char* string, const char* expr);
	DLG_API const char* dlg__strip_root_path(const char* file, const char* base);

#endif // DLG_DISABLE

// Untagged leveled logging
#define dlg_trace(...) dlg_log(dlg_level_trace, __VA_ARGS__)
#define dlg_debug(...) dlg_log(dlg_level_debug, __VA_ARGS__)
#define dlg_info(...) dlg_log(dlg_level_info, __VA_ARGS__)
#define dlg_warn(...) dlg_log(dlg_level_warn, __VA_ARGS__)
#define dlg_error(...) dlg_log(dlg_level_error, __VA_ARGS__)
#define dlg_fatal(...) dlg_log(dlg_level_fatal, __VA_ARGS__)

// Tagged leveled logging
#define dlg_tracet(tags, ...) dlg_logt(dlg_level_trace, tags, __VA_ARGS__)
#define dlg_debugt(tags, ...) dlg_logt(dlg_level_debug, tags, __VA_ARGS__)
#define dlg_infot(tags, ...) dlg_logt(dlg_level_info, tags, __VA_ARGS__)
#define dlg_warnt(tags, ...) dlg_logt(dlg_level_warn, tags, __VA_ARGS__)
#define dlg_errort(tags, ...) dlg_logt(dlg_level_error, tags, __VA_ARGS__)
#define dlg_fatalt(tags, ...) dlg_logt(dlg_level_fatal, tags, __VA_ARGS__)

// Assert macros useing DLG_DEFAULT_ASSERT as level
#define dlg_assert(expr) dlg_assertl(DLG_DEFAULT_ASSERT, expr)
#define dlg_assertt(tags, expr) dlg_assertlt(DLG_DEFAULT_ASSERT, tags, expr)
#define dlg_assertm(expr, ...) dlg_assertlm(DLG_DEFAULT_ASSERT, expr, __VA_ARGS__)
#define dlg_asserttm(tags, expr, ...) dlg_assertltm(DLG_DEFAULT_ASSERT, tags, expr, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // header guard
