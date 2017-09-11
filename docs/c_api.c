#include <stdbool.h>
#include <stddef.h>

// #include "config.h"
// TODO: make 100% sure the library of the c api does not depend on config.h!

#define DLG_TAGS_SIZE 20
#define DLG_FILE "todo"
#define DLG_FMT_FUNC fmt_func

const char* fmt_func(const char* fmt, ...) { return NULL; }

enum dlg_level {
	dlg_level_trace = 0, // temporary used debug, e.g. to check if control reaches function
	dlg_level_debug, // general debugging, prints e.g. all major events
	dlg_level_info, // general useful information
	dlg_level_warn, // warning, something went wrong but might have no (really bad) side effect
	dlg_level_error, // something really went wrong; expect serious issues
	dlg_level_critical // fatal error; application is likely to crash/exit
};

bool dlg_level_less(enum dlg_level a, enum dlg_level b);

struct dlg_tags {
	const char* tags[DLG_TAGS_SIZE];
};

struct dlg_tags_list {
	struct dlg_tags tags;
	struct dlg_tags_list* next;
};

struct dlg_origin {
	const char* file;
	unsigned int line;
	const char* func;
	enum dlg_level level;
	const char** tags; // null-terminated
	const char* expr; // assertion expression, otherwise null
};

struct dlg_current_tags {
	struct dlg_tags tags;
	const char* func;
};

#define DLG_CREATE_TAGS(...) (struct dlg_tags) {__VA_ARGS__}

#define dlg_log(level, ...) dlg_do_log(level, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), NULL)
#define dlg_logt(level, tags, ...) dlg_do_logt(level, DLG_CREATE_TAGS tags, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), NULL)

#define dlg_assertl(expr, level, ...) if(!(expr)) dlg_do_log(level, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), #expr)
#define dlg_assertlt(expr, level, tags, ...) if(!(expr)) dlg_do_logt(level, DLG_CREATE_TAGS tags, DLG_FILE, __LINE__, __FUNCTION__, DLG_FMT_FUNC(__VA_ARGS__), #expr)

#define dlg_trace(...) dlg_log(dlg_level_trace, __VA_ARGS__)
#define dlg_debug(...) dlg_log(dlg_level_debug, __VA_ARGS__)
#define dlg_info(...) dlg_log(dlg_level_info, __VA_ARGS__)
#define dlg_warn(...) dlg_log(dlg_level_warn, __VA_ARGS__)
#define dlg_error(...) dlg_log(dlg_level_error, __VA_ARGS__)
#define dlg_critical(...) dlg_log(dlg_level_critical, __VA_ARGS__)

#define dlg_tracet(tags, ...) dlg_log(dlg_level_trace, tags, __VA_ARGS__)
#define dlg_debugt(tags, ...) dlg_log(dlg_level_debug, tags, __VA_ARGS__)
#define dlg_infot(tags, ...) dlg_log(dlg_level_info, tags, __VA_ARGS__)
#define dlg_warnt(tags, ...) dlg_log(dlg_level_warn, tags, __VA_ARGS__)
#define dlg_errort(tags, ...) dlg_log(dlg_level_error, tags, __VA_ARGS__)
#define dlg_criticalt(tags, ...) dlg_log(dlg_level_critical, tags, __VA_ARGS__)

#define dlg_assert_trace(expr, ...) dlg_assertl(expr, dlg_level_trace, __VA_ARGS__)
#define dlg_assert_debug(expr, ...) dlg_assertl(expr, dlg_level_debug, __VA_ARGS__)
#define dlg_assert_info(expr, ...) dlg_assertl(expr, dlg_level_info, __VA_ARGS__)
#define dlg_assert_warn(expr, ...) dlg_assertl(expr, dlg_level_warn, __VA_ARGS__)
#define dlg_assert_error(expr, ...) dlg_assertl(expr, dlg_level_error, __VA_ARGS__)
#define dlg_assert_critical(expr, ...) dlg_assertl(expr, dlg_level_critical, __VA_ARGS__)

#define dlg_assert_tracet(expr, tags, ...) dlg_assertlt(expr, dlg_level_trace, tags, __VA_ARGS__)
#define dlg_assert_debugt(expr, tags, ...) dlg_assertlt(expr, dlg_level_debug, tags, __VA_ARGS__)
#define dlg_assert_infot(expr, tags, ...) dlg_assertlt(expr, dlg_level_info, tags, __VA_ARGS__)
#define dlg_assert_warnt(expr, tags, ...) dlg_assertlt(expr, dlg_level_warn, tags, __VA_ARGS__)
#define dlg_assert_errort(expr, tags, ...) dlg_assertlt(expr, dlg_level_error, tags, __VA_ARGS__)
#define dlg_assert_criticalt(expr, tags, ...) dlg_assertlt(expr, dlg_level_critical, tags, __VA_ARGS__)

#define dlg_tagged(tags, code) { \
	int _dlg_tagged_id_var = dlg_add_tags(tags); \
	code \
	dlg_remove_tags(_dlg_tagged_id_var); \
}

#if 1 // todo
	#define dlg_check(code) { code }
	#define dlg_checkt(tags, code) dlg_tagged(tags, code)
#else
#endif // DLG_CHECK

void dlg_do_log(enum dlg_level lvl, const char* file, int line, const char* func, const char* string, const char* expr)
{

}

void dlg_do_logt(enum dlg_level lvl, struct dlg_tags tags, const char* file, int line, const char* func, const char* string, const char* expr)
{

}

int test()
{
	dlg_logt(dlg_level_trace, ("a", "b"), "test");
	dlg_assertl(1 == 2, dlg_level_trace, "test %d\n", 5);
	dlg_warn("ayy%s\n", "oyy");
	return 0;
}

// implementation
#include <stdio.h>
#include <stdlib.h>

struct dlg_tag_func_pair { const char* tag; const char* func; };
struct dlg_tags_list {
	size_t capacity;
	size_t size;
	struct dlg_tag_func_pair pairs[];
};

struct dlg_tags_list* dlg_current_tags() // null-terminated
{
	// TODO: don't use _Thead_local, depending on build config?
	// or make it possible to use another keyword (for older/non-c11 compilers)
	static _Thread_local struct dlg_tags_list* list = NULL;
	if(!list) {
		list = calloc(10, sizeof(struct dlg_tags_list) + sizeof(struct dlg_tag_func_pair[20]));
		if(!list) {
			printf("dlg_current_tags: failed to calloc storage");
		}
	}

	return list;
}

// does nothing if tag is already present
void dlg_add_tag(const char* tag, const char* func)
{

}

// finds tag with strcmp
void dlg_remove_tag(const char* tag)
{

}
