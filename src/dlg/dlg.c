#define _XOPEN_SOURCE
#include <dlg/output.h>
#include <dlg/dlg.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char* dlg_reset_sequence = "\033[0m";
const struct dlg_style dlg_default_output_styles[] = {
	{dlg_text_style_italic, dlg_color_green, dlg_color_none},
	{dlg_text_style_dim, dlg_color_gray, dlg_color_none},
	{dlg_text_style_none, dlg_color_cyan, dlg_color_none},
	{dlg_text_style_none, dlg_color_yellow, dlg_color_none},
	{dlg_text_style_none, dlg_color_red, dlg_color_none},
	{dlg_text_style_bold, dlg_color_red, dlg_color_none}
};

static void* xalloc(size_t size) {
	void* ret = calloc(size, 1);
	if(!ret) fprintf(stderr, "dlg: calloc returned NULL, probably crashing (size: %zu)\n", size);
	return ret;
}

static void* xrealloc(void* ptr, size_t size) {
	void* ret = realloc(ptr, size);
	if(!ret) fprintf(stderr, "dlg: realloc returned NULL, probably crashing (size: %zu)\n", size);
	return ret;
}

// platform-specific
#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
	#define DLG_OS_UNIX
	#include <unistd.h>
	
	bool dlg_is_tty(FILE* stream) {
		return isatty(fileno(stream));
	}
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
	
	bool dlg_is_tty(FILE* stream) {
		return _isatty(_fileno(stream));
	}

	static bool init_ansi_console(void) {
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
	
	// TODO: does WriteConsoleW even work for buffers larger than 64 KB??
	// Maybe just don't support it?
	static void win_write_heap(void* handle, size_t needed, const char* format, va_list args) {
		char* buf1 = xalloc(3 * needed + 3 + (needed % 2));
		wchar_t* buf2 = (wchar_t*) (buf1 + needed + 1 + (needed % 2));
		vsnprintf(buf1, needed + 1, format, args);
	    needed = MultiByteToWideChar(CP_UTF8, 0, buf1, needed, buf2, needed + 1); // TODO: handle error
		WriteConsoleW(handle, buf2, needed, NULL, NULL);
		free(buf1);
	}
	
	static void win_write_stack(void* handle, size_t needed, const char* format, va_list args) {
		char buf1[needed + 1];
		wchar_t buf2[needed + 1];
		vsnprintf(buf1, needed + 1, format, args);
	    needed = MultiByteToWideChar(CP_UTF8, 0, buf1, needed, buf2, needed + 1); // TODO: handle error
		WriteConsoleW(handle, buf2, needed, NULL, NULL);
	}

#else
	#error Cannot determine platform (needed for color and utf-8 and stuff)
#endif

// general
void dlg_escape_sequence(struct dlg_style style, char buf[12]) {
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

void dlg_vfprintf(FILE* stream, const char* format, va_list args) {
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
		int needed = vsnprintf(NULL, 0, format, args_copy);
		va_end(args_copy);

		if(needed < 0) {
			fprintf(stderr, "dlg_fprintf: invalid format/arguments given\n");
			return;
		}

		// we don't allocate more than 64kb on the stack
		if(needed > 64 * 1024) {
			win_write_heap(handle, needed, format, args);
			return;
		} else {
			win_write_stack(handle, needed, format, args);
			return;
		}
	}
#endif

	vfprintf(stream, format, args);
}

void dlg_fprintf(FILE* stream, const char* format, ...) {
	va_list args;
	va_start(args, format);
	dlg_vfprintf(stream, format, args);
	va_end(args);
}

void dlg_styled_fprintf(FILE* stream, struct dlg_style style, const char* format, ...) {
	char buf[12];
	dlg_escape_sequence(style, buf);

	fprintf(stream, "%s", buf);
	va_list args;
	va_start(args, format);
	dlg_vfprintf(stream, format, args);
	va_end(args);
	fprintf(stream, "%s", dlg_reset_sequence);
}

void dlg_generic_output(dlg_generic_output_handler output, void* data, 
		unsigned int features, const struct dlg_origin* origin, const char* string, 
		const struct dlg_style styles[6]) {
	if(features & dlg_output_style) {
		char buf[12];
		dlg_escape_sequence(styles[origin->level], buf);
		output(data, "%s", buf);
	}
	
	if(features & (dlg_output_time | dlg_output_file_line | dlg_output_tags | dlg_output_func)) {
		output(data, "[");
	}
	
	bool first_meta = true;
	if(features & dlg_output_time) {
		time_t t = time(NULL);
		struct tm *tm_info = localtime(&t);
		char timebuf[32];
		strftime(timebuf, sizeof(timebuf), "%H:%M:%S", tm_info);
		output(data, "%s", timebuf);
		first_meta = false;
	}
	
	if(features & dlg_output_file_line) {
		if(!first_meta) {
			output(data, " ");
		}
		
		// file names might conatin non-ascii chars
		output(data, "%s:%u", origin->file, origin->line); 
		first_meta = false;
	}
	
	if(features & dlg_output_func) {
		if(!first_meta) {
			output(data, " ");
		}
		
		// NOTE: use dlg_fprintf here? func names should really be ascii
		// and can we otherwise be sure they are utf-8? probably not
		output(data, "%s", origin->func);
		first_meta = false;
	}
	
	if(features & dlg_output_tags) {
		if(!first_meta) {
			output(data, " ");
		}
		
		output(data, "{");
		bool first_tag = true;
		for(const char** tags = origin->tags; *tags; ++tags) {
			if(!first_tag) {
				output(data, ", ");
			}
			
			output(data, "%s", *tags);
			first_tag = false;
		}
		
		output(data, "}");
	}
	
	if(features & (dlg_output_time | dlg_output_file_line | dlg_output_tags | dlg_output_func)) {
		output(data, "] ");
	}
	
	if(origin->expr && string) {
		output(data, "assertion '%s' failed: '%s'", origin->expr, string);
	} else if(origin->expr) {
		output(data, "assertion '%s' failed", origin->expr);
	} else if(string) {
		output(data, "%s", string);
	}
	
	if(features & dlg_output_style) {
		output(data, "%s", dlg_reset_sequence);
	}

	// TODO: really output this newline?
	// maybe someones wants to print multiple outputs in one line?
	output(data, "\n");
}

struct buf {
	char* buf;
	size_t* size;
};

static void print_size(void* size, const char* format, ...) {
	va_list args;
	va_start(args, format);

	// TODO: check for -1? we only use our own formats here
	*((size_t*) size) += vsnprintf(NULL, 0, format, args); 
	va_end(args);
}

static void print_buf(void* dbuf, const char* format, ...) {
	struct buf* buf = dbuf;
	va_list args;
	va_start(args, format);

	// TODO: check for -1? we only use our own formats here
	int printed = vsnprintf(buf->buf, *buf->size, format, args); 
	va_end(args);
	*buf->size -= printed;
	buf->buf += printed;
}

void dlg_generic_output_buf(char* buf, size_t* size, unsigned int features,
		const struct dlg_origin* origin, const char* string, 
		const struct dlg_style styles[6]) {
	if(buf) {
		struct buf mbuf = {buf, size};
		dlg_generic_output(print_buf, &mbuf, features, origin, string, styles);
	} else {
		*size = 0;
		dlg_generic_output(print_size, size, features, origin, string, styles);
	}
}

static void print_stream(void* stream, const char* format, ...) {
	va_list args;
	va_start(args, format);
	dlg_vfprintf(stream, format, args);
	va_end(args);
}

void dlg_generic_output_stream(FILE* stream, unsigned int features,
		const struct dlg_origin* origin, const char* string,
		const struct dlg_style styles[6]) {
	if(!stream) {
		stream = (origin->level < dlg_level_warn) ? stdout : stderr;
	}
	
	dlg_generic_output(print_stream, stream, features, origin, string, styles);
}

void dlg_default_output(const struct dlg_origin* origin, const char* string, void* data) {
	FILE* stream = data;
	if(!stream) {
		stream = (origin->level < dlg_level_warn) ? stdout : stderr;
	}
	
	unsigned int features = dlg_output_file_line;
	if(dlg_is_tty(stream) && dlg_win_init_ansi()) {
		features |= dlg_output_style;
	}

	dlg_generic_output_stream(stream, features, origin, string, dlg_default_output_styles);
	fflush(stream);
}

bool dlg_win_init_ansi(void) {
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

// TODO: use size_t instead of unsigned int?
// small dynamic vec/array implementation
#define vec__raw(vec) (((unsigned int*) vec) - 2)

static void* vec_do_create(unsigned int typesize, unsigned int cap, unsigned int size) {
	cap = (size > cap) ? size : cap;
	unsigned int* begin = xalloc(2 * sizeof(unsigned int) + 2 * cap * typesize);
	begin[0] = size * typesize;
	begin[1] = cap * typesize;
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
		begin = xrealloc(begin, sizeof(unsigned int) * 2 + needed * 2);
		begin[1] = needed * 2;
		vec = begin + 2;
	}

	void* ptr = (char*) vec + begin[0];
	begin[0] += size;
	return ptr;
}

#define vec_create(type, size) (type*) vec_do_create(sizeof(type), size * 2, size)
#define vec_create_reserve(type, size, capacity) (type*) vec_do_create(sizeof(type), capcity, size)
#define vec_init(array, size) array = vec_do_create(sizeof(*array), size * 2, size)
#define vec_init_reserve(array, size, capacity) array = vec_do_create(sizeof(*array), capacity, size)
#define vec_free(vec) free((vec) ? vec__raw(vec) : NULL)
#define vec_erase_range(vec, pos, count) vec_do_erase(vec, pos * sizeof(*vec), count * sizeof(*vec))
#define vec_erase(vec, pos) vec_do_erase(vec, pos * sizeof(*vec), sizeof(*vec))
#define vec_size(vec) vec__raw(vec)[0] / sizeof(*vec)
#define vec_capacity(vec) vec_raw(vec)[1] / sizeof(*vec)
#define vec_add(vec) vec_do_add(vec, sizeof(*vec))
#define vec_addc(vec, count) (vec_do_add(vec, sizeof(*vec) * count))
#define vec_push(vec, value) (vec_do_add(vec, sizeof(*vec)), vec_last(vec) = (value))
#define vec_pop(vec) (vec__raw(vec)[0] -= sizeof(*vec))
#define vec_popc(vec, count) (vec__raw(vec)[0] -= sizeof(*vec) * count)
#define vec_clear(vec) (vec__raw(vec)[0] = 0)
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

static struct dlg_data** dlg_get_data(void) {
	// NOTE: maybe don't hardcode _Thead_local, depending on build config?
	// or make it possible to use another keyword (for older/non-c11 compilers)
	static _Thread_local struct dlg_data* data = NULL;
	return &data;
}

static struct dlg_data* dlg_data(void) {
	struct dlg_data** data = dlg_get_data();
	if(!*data) {
		*data = xalloc(sizeof(struct dlg_data));
		vec_init_reserve((*data)->tags, 0, 20);
		vec_init_reserve((*data)->pairs, 0, 20);
		(*data)->buffer_size = 100;
		(*data)->buffer = xalloc(100);
	}

	return *data;
}

void dlg_add_tag(const char* tag, const char* func) {
	struct dlg_data* data = dlg_data();
	struct dlg_tag_func_pair* pair = vec_add(data->pairs);
	pair->tag = tag;
	pair->func = func;
}

bool dlg_remove_tag(const char* tag, const char* func) {
	struct dlg_data* data = dlg_data();
	for(unsigned int i = 0; i < vec_size(data->pairs); ++i) {
		if(data->pairs[i].func == func && data->pairs[i].tag == tag) {
			vec_erase(data->pairs, i);
			return true;
		}
	}

	return false;
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

void dlg_cleanup() {
	struct dlg_data** data = dlg_get_data();
	if(*data) {
		vec_free((*data)->pairs);
		vec_free((*data)->tags);
		free((*data)->buffer);
		free(*data);
		*data = NULL;
	}
}

const char* dlg__printf_format(const char* str, ...) {
	va_list vlist;
	va_start(vlist, str);

	va_list vlistcopy;
	va_copy(vlistcopy, vlist);
	int needed = vsnprintf(NULL, 0, str, vlist);
	if(needed < 0) {
		printf("dlg__printf_format: invalid format given\n");
		va_end(vlist);
		va_end(vlistcopy);
		return NULL;
	}

	va_end(vlist);

	size_t* buf_size;
	char** buf = dlg_thread_buffer(&buf_size);
	if(*buf_size <= (unsigned int) needed) {
		*buf_size = (needed + 1) * 2;
		*buf = xrealloc(*buf, *buf_size);
	}

	vsnprintf(*buf, *buf_size, str, vlistcopy);
	va_end(vlistcopy);

	return *buf;
}

void dlg__do_log(enum dlg_level lvl, const char* const* tags, const char* file, int line,
		const char* func, const char* string, const char* expr) {
	struct dlg_data* data = dlg_data();
	unsigned int tag_count = 0; 
	
	// push default tags
	while(tags[tag_count]) { 
		vec_push(data->tags, tags[tag_count++]);
	}
	
	// push current global tags
	for(size_t i = 0; i < vec_size(data->pairs); ++i) {
		// TODO: really use strcmp here? does == not work?
		const struct dlg_tag_func_pair pair = data->pairs[i];
		if(pair.func == NULL || !strcmp(pair.func, func)) {
			vec_push(data->tags, pair.tag);
		}
	}
	
	// push call-specific tags, skip first terminating NULL
	++tag_count;
	while(tags[tag_count]) {
		vec_push(data->tags, tags[tag_count++]);
	}

	vec_push(data->tags, NULL); // terminating NULL
	struct dlg_origin origin = {
		.level = lvl,
		.file = file,
		.line = line,
		.func = func,
		.expr = expr,
		.tags = data->tags
	};

	g_handler(&origin, string, g_data);
	vec_clear(data->tags);
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
