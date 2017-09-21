#define DLG_DEFAULT_TAGS "dlg", 

#include <dlg/dlg.hpp>
#include <dlg/output.h>
#include <iomanip>

unsigned int gerror = 0;

// TODO: pretty much todo... test all of the header correctly

#define EXPECT(a) if(!(a)) { \
	printf("$$$ Expect '" #a "' failed [%d]\n", __LINE__); \
	++gerror; \
}

int main() {
	EXPECT(dlg::rformat("$", "\\$\\") == "$");
	EXPECT(dlg::rformat("$", "$\\", 0) == "0\\");
	EXPECT(dlg::rformat("$", "\\$", 1) == "\\1");
	EXPECT(dlg::rformat("$", "\\$\\$\\", 2) == "$2\\");
	EXPECT(dlg::rformat("@", "@", "ayyy") == "ayyy");
	EXPECT(dlg::format("{{}}", 2) == "{2}");
	EXPECT(dlg::format("\\{}\\") == "{}");
	EXPECT(dlg::format("\\{{}}\\", 2) == "\\{2}\\");

	dlg::set_handler([&](const struct dlg_origin& origin, const char* str){
		dlg_win_init_ansi();
		dlg_generic_output_stream(nullptr, ~0, &origin, str, dlg_default_output_styles);
	});

	{
		dlg_tags("a", "b");
		dlg_infot(("tag1", "tag2"), "Just some {} info", "formatted");
	}
	
	dlg_warnt(("tag2", "tag3"), "Just some {} warning: {} {}", "sick", std::setw(10), 69);
	dlg_assertm(true, "eeeehhh... {}", "wtf");
	dlg_assertm(false, "should fire... {} {}", "!", 24);

	dlg_cleanup();
	return gerror;
}

// TODO: use or remove
// #define DLG_ADD_TAGS(tags, ...) (DLG_MM_HELPER tags, __VA_ARGS__)
// #define DLG_EVAL(...) __VA_ARGS__
// #define ny_warn(...) dlg_warnt(("ny"), __VA_ARGS__)
