#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

// #include "config.h"
// TODO: make 100% sure the library of the c api does not depend on config.h!

// the number of tags in DLG_DEFAULT_TAGS added to the maximum number of tags
// given to a log/assert function must never exceed this value
// just increase it if you need more tags. The compiler will output a warning/error
// if this size is exceeded somewhere.
// The number of parameters passed in dlg_tag can also not exceed this.
#define DLG_TAGS_SIZE 20
#define DLG_FILE "todo"
#define DLG_FMT_FUNC fmt_func

#define DLG_LOG_LEVEL dlg_level_trace
#define DLG_ASSERT_LEVEL dlg_level_trace
#define DLG_DEFAULT_TAGS "a", "b"

#ifdef __GNUC__
	#define PRINTF_ATTRIB(a, b) __attribute__ ((format (printf, a, b)))
#else
	#define PRINTF_ATTRIB(a, b)
#endif

char* fmt_func(const char* str, ...) PRINTF_ATTRIB(1, 2);
char* fmt_func(const char* str, ...)
{
	va_list vlist;
	va_start(vlist, str);

	va_list vlistcopy;
	va_copy(vlistcopy, vlist);

	unsigned int size = vsnprintf(NULL, 0, str, vlist);
	va_end(vlist);

	char* ret = calloc(size + 1, 1);
	vsnprintf(ret, size + 1, str, vlistcopy);
	va_end(vlistcopy);

	return ret;
}

enum dlg_level {
	dlg_level_trace = 0, // temporary used debug, e.g. to check if control reaches function
	dlg_level_debug, // general debugging, prints e.g. all major events
	dlg_level_info, // general useful information
	dlg_level_warn, // warning, something went wrong but might have no (really bad) side effect
	dlg_level_error, // something really went wrong; expect serious issues
	dlg_level_critical // fatal error; application is likely to crash/exit
};

typedef const char* dlg_tags[DLG_TAGS_SIZE];

struct dlg_origin {
	const char* file;
	unsigned int line;
	const char* func;
	enum dlg_level level;
	const char** tags; // null-terminated
	const char* expr; // assertion expression, otherwise null
};

// utility
#define DLG_CREATE_TAGS(...) (dlg_tags) {__VA_ARGS__, DLG_DEFAULT_TAGS, NULL}
#define DLG_EVAL(...) __VA_ARGS__

// Tagged/Untagged logging with variable level
#define dlg_log(level, ...) if(level >= DLG_LOG_LEVEL) \
	dlg_do_log(level, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), NULL)
#define dlg_logt(level, tags, ...) if(level >= DLG_LOG_LEVEL) \
	dlg_do_logt(level, DLG_CREATE_TAGS tags, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), NULL)

// Tagged/Untagged assertion with variable level
#define dlg_assertl(expr, level, ...) if(level >= DLG_ASSERT_LEVEL && !(expr)) \
	dlg_do_log(level, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), #expr)
#define dlg_assertlt(expr, level, tags, ...) if(level >= DLG_ASSERT_LEVEL && !(expr)) \
	dlg_do_logt(level, DLG_CREATE_TAGS tags, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), #expr)

// Untagged leveled logging
#define dlg_trace(...) dlg_log(dlg_level_trace, __VA_ARGS__)
#define dlg_debug(...) dlg_log(dlg_level_debug, __VA_ARGS__)
#define dlg_info(...) dlg_log(dlg_level_info, __VA_ARGS__)
#define dlg_warn(...) dlg_log(dlg_level_warn, __VA_ARGS__)
#define dlg_error(...) dlg_log(dlg_level_error, __VA_ARGS__)
#define dlg_critical(...) dlg_log(dlg_level_critical, __VA_ARGS__)

// Tagged leveled logging
#define dlg_tracet(tags, ...) dlg_logt(dlg_level_trace, tags, __VA_ARGS__)
#define dlg_debugt(tags, ...) dlg_logt(dlg_level_debug, tags, __VA_ARGS__)
#define dlg_infot(tags, ...) dlg_logt(dlg_level_info, tags, __VA_ARGS__)
#define dlg_warnt(tags, ...) dlg_logt(dlg_level_warn, tags, __VA_ARGS__)
#define dlg_errort(tags, ...) dlg_logt(dlg_level_error, tags, __VA_ARGS__)
#define dlg_criticalt(tags, ...) dlg_logt(dlg_level_critical, tags, __VA_ARGS__)

// Assert macros useing DLG_DEFAULT_ASSERT as level (defaulted to error)
#define dlg_assert(expr, ...) dlg_assert_l(expr, DLG_DEFAULT_ASSERT, __VA_ARGS__)
#define dlg_assertt(expr, tags, ...) dlg_assert_ln(expr, DLG_DEFAULT_ASSERT, tags, __VA_ARGS__)

#define dlg_tag_base(global, tags, code) { \
	dlg_tags _dlg_tag_tags = {DLG_EVAL tags}; \
	const char** _dlg_tag_ptr = _dlg_tag_tags.tags; \
	const char* _dlg_tag_func = global ? NULL : __FUNCTION__; \
	while(_dlg_tag_ptr)  \
		dlg_add_tag(_dlg_tag_ptr++, _dlg_tag_func); \
	code \
	_dlg_tag_ptr = _dlg_tag_tags.tags; \
	while(_dlg_tag_ptr) \
		 dlg_remove_tag(_dlg_tag_ptr++, _dlg_tag_func); \
}

#define dlg_tag(tags, code) dlg_tag_base(false, tags, code)
#define dlg_tag_global(tags, code) dlg_tag_base(true, tags, code)

#if 1 // todo
	#define dlg_check(code) { code }
	#define dlg_checkt(tags, code) dlg_tag(tags, code)
#else
#endif // DLG_CHECK

void dlg_do_log(enum dlg_level lvl, const char* file, int line, const char* func,
	char* string, const char* expr);

void dlg_do_logt(enum dlg_level lvl, const char** tags, const char* file, int line,
	const char* func, char* string, const char* expr);

int main()
{
	dlg_logt(dlg_level_trace, ("a", "b"), "test");
	dlg_assertlt(1 == 2, dlg_level_trace, ("taaag"), "test %d", 5);
	dlg_warn("ayy1%s", "oyy");
	dlg_warnt(("tag"), "ayy2%s", "oyy");

	#define DLG_MM_HELPER(...) __VA_ARGS__
	#define DLG_ADD_TAGS(tags, ...) (DLG_MM_HELPER tags, __VA_ARGS__)
	#define ny_warnt(tags, ...) dlg_warnt(DLG_ADD_TAGS(tags, "ny"), __VA_ARGS__)
	#define ny_warn(...) dlg_warnt(("ny"), __VA_ARGS__)
	ny_warnt(("a", "b"), "test1");
	ny_warn("test2");

	return 0;
}

// implementation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// small dynamic vec/array implementation
#define vec__raw(vec) (((unsigned int*) vec) - 2)

void* vec_do_create(unsigned int size)
{
	unsigned int* begin = malloc(2 * sizeof(unsigned int) + 2 * size);
	begin[0] = size;
	begin[1] = 2 * size;
	return begin + 2;
}

void vec_do_erase(void* vec, unsigned int pos, unsigned int size)
{
	unsigned int* begin = vec__raw(vec);
	begin[0] -= size;
	char* buf = vec;
	memcpy(buf + pos, buf + pos + size, size);
}

void* vec_do_add(void* vec, unsigned int size)
{
	unsigned int* begin = vec__raw(vec);
	unsigned int needed = begin[0] + size;
	if(needed >= begin[1]) {
		begin = realloc(vec, sizeof(unsigned int) * 2 + needed * 2);
		if(!begin) {
			printf("vec_do_add: realloc returned NULL");
			return NULL;
		}

		begin[1] = needed * 2;
		vec = begin + 2;
	}

	void* ptr = (char*) vec + begin[0];
	begin[0] += size;
	return ptr;
}

#define vec_create(type, count) (type*) vec_do_create(count * sizeof(type))
#define vec_init(array, count) array = vec_do_create(count * sizeof(*array))
#define vec_free(vec) free(vec__raw(vec))
#define vec_erase_range(vec, pos, count) vec_do_erase(vec, pos * sizeof(*vec), count * sizeof(*vec))
#define vec_erase(vec, pos) vec_do_erase(vec, pos * sizeof(*vec), sizeof(*vec))
#define vec_size(vec) vec__raw(vec)[0] / sizeof(*vec)
#define vec_capacity(vec) vec_raw(vec)[1] / sizeof(*vec)
#define vec_add(vec) vec_do_add(vec, sizeof(*vec))
#define vec_addc(vec, count) (vec_do_add(vec, sizeof(*vec) * count))
#define vec_push(vec, value) (vec_do_add(vec, sizeof(*vec)), vec_last(vec) = (value))
#define vec_pop(vec) (vec__raw(vec)[0] -= sizeof(*vec))
#define vec_popc(vec, count) (vec__raw(vec)[0] -= sizeof(*vec) * count)
#define vec_last(vec) (vec[vec_size(vec) - 1])

struct dlg_tag_func_pair {
	const char* tag;
	const char* func;
};

struct dlg_data {
	const char** tags; // vec
	struct dlg_tag_func_pair* pairs; // vec
};

struct dlg_data* dlg_data()
{
	// TODO: don't use _Thead_local, depending on build config?
	// or make it possible to use another keyword (for older/non-c11 compilers)
	static _Thread_local struct dlg_data data = {NULL, NULL};
	if(!data.tags || !data.pairs) {
		vec_init(data.tags, 10);
		vec_init(data.pairs, 10);
	}

	return &data;
}

void dlg_add_tag(const char* tag, const char* func)
{
	struct dlg_data* data = dlg_data();
	struct dlg_tag_func_pair* pair = vec_add(data->pairs);
	pair->tag = tag;
	pair->func = func;
}

/// The given pointer must be exactly the same
void dlg_remove_tag(const char* tag, const char* func)
{
	struct dlg_data* data = dlg_data();
	for(unsigned int i = 0; i < vec_size(data->pairs); ++i) {
		if(data->pairs[i].func == func && data->pairs[i].tag == tag) {
			vec_erase(data->pairs, i);
			return;
		}
	}
}

typedef void(*dlg_handler)(const struct dlg_origin* origin, const char* string, void* data);

void dlg_default_handler(const struct dlg_origin* origin, const char* string, void* data)
{
	printf("[%s:%d]: %s\n", origin->file, origin->line, string);
}

static dlg_handler g_handler = dlg_default_handler;
static void* g_data = NULL;

void dlg_set_handler(dlg_handler handler, void* data)
{
	g_handler = handler;
	g_data = data;
}

void dlg_do_log(enum dlg_level lvl, const char* file, int line, const char* func,
	char* string, const char* expr)
{
	struct dlg_data* data = dlg_data();
	struct dlg_origin origin = {
		.level = lvl,
		.file = file,
		.line = line,
		.func = func,
		.expr = expr,
		.tags = data->tags
	};

	g_handler(&origin, string, g_data);
	free(string);
}

void dlg_do_logt(enum dlg_level lvl, const char** tags, const char* file, int line,
	const char* func, char* string, const char* expr)
{
	struct dlg_data* data = dlg_data();

	// TODO: vec utility for this?
	// like vec_insert that takes a null-terminated array or sth.
	// then also add vec_insert_vec (that takes another vec) or vec_cat
	unsigned int tag_count = 0;
	while(tags[tag_count])
		vec_push(data->tags, tags[tag_count++]);

	struct dlg_origin origin = {
		.level = lvl,
		.file = file,
		.line = line,
		.func = func,
		.expr = expr,
		.tags = data->tags
	};

	g_handler(&origin, string, g_data);
	vec_popc(data->tags, tag_count);
	free(string);
}
