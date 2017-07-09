#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <cstring>

#define DLG_IMPLEMENTATION
#include "dlg/dlg.hpp"

using namespace dlg::literals;

#define custom_trace(...) dlg_trace("sample"_project, "custom_trace"_scope, __VA_ARGS__)
#define custom_assert(expr, ...) dlg_assert(expr, "sample"_project, "custom_assert"_scope, __VA_ARGS__)

dlg::Logger* mySelector(dlg::Origin origin)
{
	// if(origin.level == dlg::Level::trace || (origin.source.src[1] == "render"))
	// 	return nullptr;

	return &dlg::defaultLogger;
}

struct MyInfo {};
std::ostream& operator<<(std::ostream& os, const MyInfo& info)
{
	os << "oh boi!";
	return os;
}

int main()
{
	std::cout.sync_with_stdio(false);

	dlg::selector(&mySelector);
	std::vector<int> v = {1, 4, 6, 3, 2, 4, 1};

	dlg_trace("network"_module, "trace {}", 1);
	dlg_debug("debug {}", 2);
	dlg_info("info"_module, "info {}, but...: {}", 3, MyInfo {});
	dlg_warn("warn {} [{}]", 4, fmt::join(v.begin(), v.end(), ", "));
	dlg_error("error {}", 5);
	dlg_critical("render"_module, "critical {}", 6);

	custom_trace("some custom trace");

	std::cout << "just some normal output\n";

	dlg_check("main::main_debug::e1"_src, {
		dlg_assert(5 == 6, "assert failed");
		custom_assert(1 == 1, "you should really not see this");
		custom_assert(1 == 2, "uuuugh...");
	});

	{
		dlg::SourceGuard guard1("my"_project);
		dlg::SourceGuard guard("::mod::scope"_src);
		dlg_info("just some {}", "source guard test");
	}

	dlg_log("Loggin is easy!");
	dlg_assert(5 == 6, "render"_module, "uuh... {}", "whatdoiknowgoawayyoulittleshit!");
	dlg_assert_debug(5 == 6, "ny::x11::data"_src, "uuh... {}", "whatdoiknowgoawayyoulittleshit!");
}
