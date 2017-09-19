#define DLG_DEFAULT_TAGS "dlg"

#include <dlg/dlg.hpp>
#include <dlg/output.h>

void foo(const char* const*);

int main() {
	dlg::set_handler([&](const struct dlg_origin& origin, const char* str){
		dlg_win_init_ansi();
		dlg_generic_output(nullptr, ~0, &origin, str, dlg_default_output_styles);
	});
	
	{
		dlg_tags("a", "b");
		dlg_infot(("tag1", "tag2"), "Just some {} info", "formatted");
	}
	
	dlg_warnt(("tag2", "tag3"), "Just some {} warning: {}", "sick", 69);
	dlg_assertm(2 == 0 + 2, "eeeehhh... {}", "wtf");
	dlg_assertm(1 == 2, "should fire... {} {}", "!", 24);

	dlg_cleanup();
}

#define DLG_ADD_TAGS(tags, ...) (DLG_MM_HELPER tags, __VA_ARGS__)
#define DLG_EVAL(...) __VA_ARGS__
#define ny_warn(...) dlg_warnt(("ny"), __VA_ARGS__)
