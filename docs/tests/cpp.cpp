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
	// utility functions
	EXPECT(dlg::rformat("$", "\\$\\") == "$");
	EXPECT(dlg::rformat("$", "$\\", 0) == "0\\");
	EXPECT(dlg::rformat("$", "\\$", 1) == "\\1");
	EXPECT(dlg::rformat("$", "\\$\\$\\", 2) == "$2\\");
	EXPECT(dlg::rformat("@", "@", "ayyy") == "ayyy");
	EXPECT(dlg::format("{{}}", 2) == "{2}");
	EXPECT(dlg::format("\\{}\\") == "{}");
	EXPECT(dlg::format("\\{{}}\\", 2) == "\\{2}\\");

	// check output
	enum check {
		check_line = 1,
		check_tags = 2,
		check_expr = 4,
		check_string = 8,
		check_level = 16,
		check_fire = 32
	};

	struct {
		unsigned int check;
		const char* str;
		bool fired;
	} expected {};

	dlg::set_handler([&](const struct dlg_origin& origin, const char* str){
		expected.fired = true;
		if(expected.check & check_string) {
			if((str == nullptr) != (expected.str == nullptr) || 
					(str && std::strcmp(str, expected.str) != 0)) {
				std::printf("$$$ handler: invalid string [%d]\n", origin.line);
				++gerror;
			}
		}

		// output
		dlg_win_init_ansi();
		dlg_generic_output_stream(nullptr, ~0u, &origin, str, dlg_default_output_styles);
	});

	{
		dlg_tags("a", "b");
		expected.check &= check_string;
		expected.str = "Just some formatted info";
		dlg_infot(("tag1", "tag2"), "Just some {} info", "formatted");
	}
	
	expected = {};
	dlg_warnt(("tag2", "tag3"), "Just some {} warning: {} {}", "sick", std::setw(10), 69);
	dlg_assertm(true, "eeeehhh... {}", "wtf");
	dlg_assertm(false, "should fire... {} {}", "!", 24);
	
	auto entered = false;
	dlg_checkt(("checked"), {
		entered = true;
		dlg_info("from inside the check block");
		EXPECT(expected.fired);
	});
	EXPECT(entered);

	dlg_cleanup();
	return gerror;
}
