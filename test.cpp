#define DLG_IMPLEMENTATION
#include "dlg/dlg.hpp"
using namespace dlg::literals;

dlg::Source expected;

dlg::Logger* mySelector(dlg::Origin origin)
{
	if(origin.source.src[0] != expected.src[0])
		std::cout << "0: '" << origin.source.src[0] << "' != '" << expected.src[0] << "'\n";

	if(origin.source.src[1] != expected.src[1])
		std::cout << "0: '" << origin.source.src[1] << "' != '" << expected.src[1] << "'\n";

	if(origin.source.src[2] != expected.src[2])
		std::cout << "0: '" << origin.source.src[2] << "' != '" << expected.src[2] << "'\n";

	return nullptr;
}

int main()
{
	dlg::selector(&mySelector);

	expected = {};
	dlg_debug("just some test");

	expected.src[0] = "project";
	dlg_debug("project"_project, "just some test");

	expected.src[0] = "p2";
	dlg_debug("project"_project, "p2"_project, "just some test");

	{
		dlg::SourceGuard guard1("0::1::2"_src);
		dlg::SourceGuard guard2("mod"_module);

		expected.src[0] = "0";
		expected.src[1] = "mod";
		expected.src[2] = "2";
		dlg_debug("just some test");

		expected.src[0] = "proj";
		expected.src[1] = "";
		expected.src[2] = "";
		dlg_debug(dlg::Source("proj"_project, dlg::Source::Force::full), "just some test");

		expected.src[0] = "0";
		expected.src[1] = "mod";
		expected.src[2] = "scope";
		dlg_debug("scope"_scope, "just some test");

		expected.src[0] = "";
		expected.src[1] = "";
		expected.src[2] = "";
		dlg_debug(dlg::clear_source(), "just some test");

		expected.src[0] = "";
		expected.src[1] = "";
		expected.src[2] = "mscope";
		dlg_debug(dlg::clear_source(), "mscope"_scope, "just some test");
	}

	dlg::SourceGuard guard2({"test"_module, "my"_scope});
}
