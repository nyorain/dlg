#include <dlg/output.h>
#include <dlg/dlg.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void* xalloc(size_t size) {
	void* ret = calloc(size, 1);
	if(!ret) fprintf(stderr, "calloc returned NULL, things are going to burn");
	return ret;
}

static void* xrealloc(void* ptr, size_t size) {
	void* ret = realloc(ptr, size);
	if(!ret) fprintf(stderr, "realloc returned NULL, things are going to burn");
	return ret;
}

// platform-specific
#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
	#define DLG_OS_UNIX
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64)
	#define DLG_OS_WIN
	#define WIN32_LEAN_AND_MEAN
	#define DEFINE_CONSOLEV2_PROPERTIES
	#include <windows.h>
	#include <io.h>

	// thank you for nothing, microsoft
	#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
	#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
	#endif

	static bool init_ansi_console() {
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE err = GetStdHandle(STD_OUTPUT_HANDLE);
		if(out == INVALID_HANDLE_VALUE || err == INVALID_HANDLE_VALUE)
			return false;

		DWORD outMode, errMode;
		if(!GetConsoleMode(out, &outMode) || !GetConsoleMode(err, &errMode))
		   return false;

		outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		errMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		if (!SetConsoleMode(out, outMode) || !SetConsoleMode(out, errMode))
			return false;

		return true;
	}
	
	void win_write_heap(void* handle, size_t needed, const char* format, va_list args) {
		char* buf1 = xalloc(3 * needed + 3 + (needed % 2));
		wchar_t* buf2 = (wchar_t*) (buf1 + needed + 1 + (needed % 2));
		vsnprintf(buf1, needed + 1, format, args);
	    needed = MultiByteToWideChar(CP_UTF8, 0, buf1, needed + 1, buf2, needed + 1);
		WriteConsoleW(handle, buf2, needed, NULL, NULL);
		free(buf1);
	}
	
	void win_write_stack(void* handle, size_t needed, const char* format, va_list args) {
		char buf1[needed + 1];
		wchar_t buf2[needed + 1];
		vsnprintf(buf1, needed + 1, format, args);
	    needed = MultiByteToWideChar(CP_UTF8, 0, buf1, needed + 1, buf2, needed + 1);
		WriteConsoleW(handle, buf2, needed, NULL, NULL);
	}

#else
	#error Cannot determine platform (needed for color and utf-8 and stuff)
#endif

// general
void dlg_escape_sequence(const struct dlg_style style, char buf[12]) {
	int nums[3];
	unsigned int count = 0;

	if(style.fg != dlg_color_none) {
		nums[count++] = style.fg + 30;
	}

	if(style.bg != dlg_color_none) {
		nums[count++] = style.fg + 40;
	}

	if(style.style != dlg_text_style_none) {
		nums[count++] = style.style;
	}

	switch(count) {
		case 1: snprintf(buf, 12, "\033[%dm", nums[0]); break;
		case 2: snprintf(buf, 12, "\033[%d;%dm", nums[0], nums[1]); break;
		case 3: snprintf(buf, 12, "\033[%d;%d;%dm", nums[0], nums[1], nums[2]); break;
		default: buf[0] = '\0'; break;
	}
}

const char* dlg_get_reset_sequence() {
	static const char* reset_sequence = "\033[0m";
	return reset_sequence;	
}

void dlg_fprintf(FILE* stream, const char* format, ...) {
	va_list args;
	va_start(args, format);
	
#ifdef DLG_OS_WIN
	void* handle = NULL;
	if(stream == stdout) {
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
	} else if(stream == stderr) {
		handle = GetStdHandle(STD_ERROR_HANDLE);
	}
	
	if(handle) {
		va_list args_copy;
		va_copy(args_copy, args);
		size_t needed = vsnprintf(NULL, 0, format, args_copy);
		va_end(args_copy);

		// we don't allocate more than 64kb on the stack
		if(needed > 64 * 1024) {
			win_write_heap(handle, needed, format, args);
			va_end(args);
			return;
		} else {
			win_write_stack(handle, needed, format, args);
			va_end(args);
			return;
		}
	}
#endif

	vfprintf(stream, format, args);
	va_end(args);
}

// TODO: make this public accesible
static const struct dlg_style level_styles[] = {
	{dlg_text_style_italic, dlg_color_green, dlg_color_none},
	{dlg_text_style_dim, dlg_color_gray, dlg_color_none},
	{dlg_text_style_none, dlg_color_cyan, dlg_color_none},
	{dlg_text_style_none, dlg_color_yellow, dlg_color_none},
	{dlg_text_style_none, dlg_color_red, dlg_color_none},
	{dlg_text_style_bold, dlg_color_red, dlg_color_none}
};

void dlg_default_output(const struct dlg_origin* origin, const char* string, void* data) {
	FILE* stream = (FILE*) data;
	char buf[12];
	if(!stream) {
		stream = (origin->level < dlg_level_warn) ? stdout : stderr;
	}
	
	// TODO: we currently call this to make sure it is initialized...
	dlg_win_init_ansi();

	// XXX: we could add additional tones to the style.
	// e.g. differentiate between assertion/log
	dlg_escape_sequence(level_styles[origin->level], buf);
	fputs(buf, stream);
	
	// time
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	char timebuf[32];
	strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);

	// message
	if(origin->expr && string) {
		dlg_fprintf(stream, "[%s - %s:%u] assertion '%s' failed: '%s'", timebuf, 
			origin->file, origin->line, origin->expr, string);
	} else if(origin->expr) {
		dlg_fprintf(stream, "[%s - %s:%u] assertion '%s' failed", timebuf, 
			origin->file, origin->line, origin->expr);
	} else if(string) {
		dlg_fprintf(stream, "[%s - %s:%u] %s", timebuf, origin->file, origin->line, string);
	} else {
		// this can probably not happen
		dlg_fprintf(stream, "[%s - %s:%u]", timebuf, origin->file, origin->line);
	}

	fputs(dlg_get_reset_sequence(), stream);
	fputs("\n", stream);
}

bool dlg_win_init_ansi() {
#ifdef DLG_OS_WIN
	static volatile LONG status = 0;
	LONG res = InterlockedCompareExchange(&status, 1, 0);
	if(res == 0) { // not initialized
		InterlockedExchange(&status, 3 + init_ansi_console());
	}
	
	while(status == 1); // currently initialized in another thread, spinlock
	return (status == 4);
#endif
	return true;
}

// small dynamic vec/array implementation
#define vec__raw(vec) (((unsigned int*) vec) - 2)

static void* vec_do_create(unsigned int size) {
	unsigned int* begin = xalloc(2 * sizeof(unsigned int) + 2 * size);
	begin[0] = size;
	begin[1] = 2 * size;
	return begin + 2;
}

static void vec_do_erase(void* vec, unsigned int pos, unsigned int size) {
	// TODO: can be more efficient if we are allowed to reorder vector
	unsigned int* begin = vec__raw(vec);
	begin[0] -= size;
	char* buf = (char*) vec;
	memcpy(buf + pos, buf + pos + size, size);
}

static void* vec_do_add(void* vec, unsigned int size) {
	unsigned int* begin = vec__raw(vec);
	unsigned int needed = begin[0] + size;
	if(needed >= begin[1]) {
		begin = (unsigned int*) xrealloc(vec, sizeof(unsigned int) * 2 + needed * 2);
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
	char* buffer;
	size_t buffer_size;
};

struct dlg_data* dlg_data() {
	// NOTE: maybe don't hardcode _Thead_local, depending on build config?
	// or make it possible to use another keyword (for older/non-c11 compilers)
	static _Thread_local struct dlg_data* data;
	if(!data) {
		data = xalloc(sizeof(struct dlg_data));
		vec_init(data->tags, 20);
		vec_init(data->pairs, 20);
		data->buffer_size = 100;
		data->buffer = xalloc(100);
	}

	return data;
}

void dlg_add_tag(const char* tag, const char* func) {
	struct dlg_data* data = dlg_data();
	struct dlg_tag_func_pair* pair = vec_add(data->pairs);
	pair->tag = tag;
	pair->func = func;
}

void dlg_remove_tag(const char* tag, const char* func) {
	struct dlg_data* data = dlg_data();
	for(unsigned int i = 0; i < vec_size(data->pairs); ++i) {
		if(data->pairs[i].func == func && data->pairs[i].tag == tag) {
			vec_erase(data->pairs, i);
			return;
		}
	}
}

char** dlg_thread_buffer(size_t** size) {
	struct dlg_data* data = dlg_data();
	if(size) {
		*size = &data->buffer_size;
	}
	return &data->buffer;
}

static dlg_handler g_handler = dlg_default_output;
static void* g_data = NULL;

void dlg_set_handler(dlg_handler handler, void* data) {
	g_handler = handler;
	g_data = data;
}

unsigned int dlg__push_tags(const char** tags) {
	struct dlg_data* data = dlg_data();

	// TODO: vec utility for this?
	// like vec_insert that takes a null-terminated array or sth.
	// then also add vec_insert_vec (that takes another vec) or vec_cat
	unsigned int tag_count = 0;
	while(tags[tag_count])
		vec_push(data->tags, tags[tag_count++]);
	return tag_count;
}

void dlg__pop_tags(unsigned int count) {
	vec_popc(dlg_data()->tags, count);
}

const char** dlg__get_tags() {
	return dlg_data()->tags;
}

dlg_handler dlg__get_handler() {
	return g_handler;
}

void* dlg__get_handler_data() {
	return g_data;	
}

char* dlg__printf_format(const char* str, ...) {
	va_list vlist;
	va_start(vlist, str);

	va_list vlistcopy;
	va_copy(vlistcopy, vlist);
	unsigned int needed = vsnprintf(NULL, 0, str, vlist);
	va_end(vlist);

	size_t* buf_size;
	char** buf = dlg_thread_buffer(&buf_size);
	if(*buf_size <= needed) {
		*buf_size = (needed + 1) * 2;
		buf = xrealloc(buf_size, *buf_size);
	}

	vsnprintf(*buf, *buf_size, str, vlistcopy);
	va_end(vlistcopy);

	return *buf;
}

void dlg__do_log(enum dlg_level lvl, const char* file, int line, const char* func,
		char* string, const char* expr) {
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
	if(string != dlg_data()->buffer)
		free(string);
}

void dlg__do_logt(enum dlg_level lvl, const char** tags, const char* file, int line,
		const char* func, char* string, const char* expr) {
	struct dlg_data* data = dlg_data();

	// TODO: vec utility for this?
	// like vec_insert that takes a null-terminated array or sth.
	// then also add vec_insert_vec (that takes another vec) or vec_cat
	unsigned int tag_count = 0;
	while(tags[tag_count])
		vec_push(data->tags, tags[tag_count++]);

	dlg__do_log(lvl, file, line, func, string, expr);
	vec_popc(data->tags, tag_count);
}

const char* dlg__strip_root_path(const char* file, const char* base) {
	if(!file) {
		return NULL;
	}

	const char* saved = file;
	if(*file == '.') {
		while(*(++file) == '.' || *file == '/');
		if(*file == '\0') {
			return saved;
		}

		return file;
	}

	if(base && *base != '\0' && *file != '\0') {
		while(*(++file) != '\0' && *(++base) != '\0');
		if(*file == '\0') {
			return saved;
		}
	}

	return file;
}

// TODO
#define DLG_EVAL(...) __VA_ARGS__

int main() {
	// int prev = _setmode(_fileno(stdout), 0x00020000); // _U_16TEXT
	// wprintf(L"some unicode tääääxt %hs\n", "shclaer");
	
	// char buf1[512];
	// snprintf(buf1, 512, u8"some unicode tääääxt %s\n", u8"shclöüäüßßßer");
	
	// wchar_t buf2[512];
	// int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    // std::wstring wstrTo( size_needed, 0 );
    // MultiByteToWideChar(CP_UTF8, 0, buf1, strlen(buf1), buf2, 512);
	// WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), buf2, wcslen(buf2), NULL, NULL);
	
	dlg_logt(dlg_level_trace, ("a", "b"), "test");
	dlg_assertltm(dlg_level_trace,("taaag"), 1 == 2, "yoyoyo %d %s", 42, "64");
	dlg_warn("ayy1%s", "oyy");
	dlg_warnt(("tag"), "ayy2%s", "oyy");

	// #define DLG_MM_HELPER(...) __VA_ARGS__
	// #define DLG_ADD_TAGS(tags, ...) (DLG_MM_HELPER tags, __VA_ARGS__)
	// #define ny_warnt(tags, ...) dlg_warnt(DLG_ADD_TAGS(tags, "ny"), __VA_ARGS__)
	// #define ny_warn(...) dlg_warnt(("ny"), __VA_ARGS__)
	// ny_warnt(("a", "b"), "test1");
	// ny_warn("test2");

	return 0;
}

// TODO: not sure if it is a good idea to add them
/*
	#define dlg_tag_base(global, tags, code) { \
		const char* _dlg_tag_tags[] = {DLG_EVAL tags}; \
		const char** _dlg_tag_ptr = _dlg_tag_tags;; \
		const char* _dlg_tag_func = global ? NULL : __FUNCTION__; \
		while(_dlg_tag_ptr)  \
			dlg_add_tag(_dlg_tag_ptr++, _dlg_tag_func); \
		code \
		_dlg_tag_ptr = _dlg_tag_tags; \
		while(_dlg_tag_ptr) \
			 dlg_remove_tag(_dlg_tag_ptr++, _dlg_tag_func); \
	}

#if DLG_CHECK
	#define dlg_check(code) { code }
	#define dlg_checkt(tags, code) dlg_tag(tags, code)
#else
	#define dlg_check(code)
	#define dlg_checkt(tags, code)
#endif // DLG_CHECK

#define dlg_tag(tags, code) dlg_tag_base(false, tags, code)
#define dlg_tag_global(tags, code) dlg_tag_base(true, tags, code)
*/
