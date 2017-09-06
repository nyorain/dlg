#include <dlg/dlg.hpp>
#include <dlg/output.hpp>
#include <iostream>
#include <vector>
#include <string_view>

int main()
{
	using namespace dlg::literals;

	// setup
	enum Expect : unsigned int {
		none = 0,
		file = 1,
		expr = 2,
		line = 3,
		tags = 4,
		level = 5,
		msg = 6
	};

	struct {
		Expect mask;
		std::string_view file;
		std::string_view expr;
		unsigned int line;
		std::vector<std::string_view> tags;
		dlg::Level level;
		std::string_view msg;
	} expect;

	auto failed = 0;

	dlg::output_handler([&](const dlg::Origin& origin, std::string_view msg){
		#define EXPECT(type, expr) \
			if(expect.mask & Expect::type && expect.type != expr) { \
				std::cout << #type << ":" << origin.line << " are wrong" << "\n"; \
				++failed; \
			}

		auto saved = failed;

		EXPECT(file, origin.file);
		EXPECT(expr, origin.expr);
		EXPECT(line, origin.line);
		EXPECT(tags, origin.tags);
		EXPECT(level, origin.level);
		EXPECT(msg, msg);

		if(failed > saved) {
			std::cout << "tags: {";
			for(auto& tag : origin.tags)
				std::cout << tag << ", ";
			std::cout << "}\n";
			dlg::default_output_handler(origin, msg);
		}

		#undef EXPECT
	});

	// testing
	expect.mask = static_cast<Expect>(~0u);
	expect.file = "docs/tests/basic.cpp";
	expect.expr = {};
	expect.tags = {"sample_tag"};
	expect.level = dlg::Level::warn;
	expect.msg = "some sample message: 10";
	expect.line = __LINE__ + 1;
	dlg_warn(dlg::Tag{"sample_tag"}, "some sample message: {}", 10);

	{
		dlg_tag("scope_tag1", "scope_tag2");
		expect.msg = "msg";
		expect.tags = {"scope_tag1", "scope_tag2", "log_tag1", "log_tag2"};
		expect.level = dlg::Level::trace;
		expect.line = __LINE__ + 1;
		dlg_trace("log_tag1"_tag, "log_tag2"_tag, "msg");
	}

	expect.mask = static_cast<Expect>(expect.mask & ~(Expect::line | Expect::msg));
	expect.level = dlg::Level::critical;
	expect.expr = "1 == 2";
	expect.msg = {};
	expect.tags = {"t"};
	dlg_assert_critical(1 == 2, "t"_tag);

	std::cout << "failed: " << failed << "\n";
	return failed;
}
