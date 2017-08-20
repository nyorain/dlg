// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

// Central configuration macros:
// - DLG_DEBUG (0 or 1), determines defaults for config below, defaulted to !NDEBUG
// - DLG_CHECK (0 or 1), if defined, dlg_check scopes will be executed, defaulted to DLG_DEBUG
// - DLG_LOG_LEVEL [0;6], the minimum log level
//   - the lower it is the more logs will be evaluated
//   - default: DLG_DEBUG ? trace : info
// - DLG_ASSERT_LEVEL [0;6], the minimum assert level
//   - the lower it is the more assertions will be evaluated
//   - default: DLG_DEBUG ? trace : info
//   - note that there are no trace or info assertions, so not all levels make sense here
// - DLG_DEFAULT_LOG, one of the (lowercase) log levels, Will be used for the dlg_log default macro
//   - defaulted to debug
// - DLG_DEFAULT_ASSERT, one of the (lowercase) log levels, used for the dlg_assert default macro
//   - defaulted to error
// - DLG_DEFAULT_TAGS, not defined by default. If defined before including dlg.hpp all
//    calls will have the given tags added. They must be in form {"tag1", "tag2", "tag3"}
//    Could be use to associate a tag with all dlg calls in a file.
// - DLG_FILE, the current file, defined as dlg::stripPath(__FILE__) per default
//   - if this is defined it will be use instead of stripping the __FILE__ macro
//   - may e.g. be set from the build system for custom file paths
// - DLG_BASE_PATH, the base path stripped away from the __FILE__ values, empty string by default
//   - is not used if DLG_FILE has a custom value set
//   - relative beginnings of __FILE__ will always be stripped away
//   - makes sense to define this on a per-project level, should end with a '/' character
// - DLG_DISABLE, if defined, will disable all dlg functionality. Note that this defining
//     this macro will lead to issues if your code uses anything else than dlg macros
// - DLG_DISABLE_EMPTY_LOG define this macro to make empty logging calls like
//     e.g. dlg_critical() result in a compile-time error. Not defined by default.
// - DLG_EMPTY_LOG the string to use when any dlg logging macro is called without
//     any arguments. Defaulted to "<dlg: no arguments supplied>"
//     Has no effect if DLG_DISABLE_EMPTY_LOG is defined

#ifndef DLG_CONFIG_HPP
#define DLG_CONFIG_HPP

#pragma once

// Level definitions
#define DLG_LEVEL_TRACE 1
#define DLG_LEVEL_DEBUG 2
#define DLG_LEVEL_INFO 3
#define DLG_LEVEL_WARN 4
#define DLG_LEVEL_ERROR 5
#define DLG_LEVEL_CRITICAL 6

// DLG_DEBUG
#ifndef DLG_DEBUG
	#ifndef NDEBUG
		#define DLG_DEBUG 1
	#else
		#define DLG_DEBUG 0
	#endif
#endif

// DLG_CHECK
#ifndef DLG_CHECK
	#define DLG_CHECK DLG_DEBUG
#endif

// DLG_LOG_LEVEL
#ifndef DLG_LOG_LEVEL
	#define DLG_LOG_LEVEL (3 - DLG_DEBUG * 3)
#endif

// DLG_ASSERT_LEVEL
#ifndef DLG_ASSERT_LEVEL
	#define DLG_ASSERT_LEVEL (3 - DLG_DEBUG * 3)
#endif

// DLG_DEFAULT_LOG
#ifndef DLG_DEFAULT_LOG
	#define DLG_DEFAULT_LOG debug
#endif

// DLG_DEFAULT_ASSERT
#ifndef DLG_DEFAULT_ASSERT
	#define DLG_DEFAULT_ASSERT error
#endif

// DLG_FILE
#ifndef DLG_FILE
	#ifndef DLG_BASE_PATH
		#define DLG_BASE_PATH ""
	#endif
	#define DLG_FILE ::dlg::detail::strip_path(__FILE__, DLG_BASE_PATH)
#endif

// DLG_EMPTY_LOG
#ifndef DLG_EMPTY_LOG
	#define DLG_EMPTY_LOG "<dlg: no arguments supplied>"
#endif // DLG_EMPTY_LOG

#endif // header guard
