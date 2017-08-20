
// this is the probably most important config macro (for more see config.hpp):
// if this is defined all dlg macros will do nothing (literally nothing, they
// will produce a single instruction).
// #define DLG_DISABLE

#include <dlg/dlg.hpp> // dlg macros and basic stuff
#include <dlg/output.hpp> // needed for custom output handler set later
using namespace dlg::literals; // for the _tag string literal operator

#include <iostream> // since we manually use std::cout
#include <fstream> // since we might write to a file later on

int main()
{
	// dlg offers assert and log calls mainly
	// asserts work with or without message
	dlg_assert(1 == 1, "This message should never be seen");
	dlg_assert(1 == 1);

	// there are various log levels
	dlg_trace("A trace log");
	dlg_warn("A warn log");
	dlg_debug("A debug log");
	dlg_info("An info log");
	dlg_error("An error log");
	dlg_critical("A critical log");

	// the string literals prefix only work when using
	// the dlg::literals namespace like done in this file
	// alternatively use can simply construct a tag with dlg::Tag{"name"}
	dlg_info("main"_tag, "Use dlg tags to show where a log call came from");
	dlg_info("main"_tag, "We are in the main function");
	dlg_info("main"_tag, "That can also be done using a scoped tag guard");

	{
		// using a dlg_tag guard
		dlg_tag("dlg", "example", "main_sub");

		dlg_info("Now we are using the tags specified above");
		dlg_info("Btw, if this output is confusing for you, look at example.cpp");
		dlg_info("The tags applied are not printed by default");
		dlg_info("But we could output and use them (e.g. as filter) in a custom handler");
	}

	dlg_debug("Let's switch to debug output for some variation!");
	dlg_debug("This string was formatted using the {} library", "fmtlib");
	dlg_debug("Possible since {:2} uses and therefore supports {:1} format", "dlg", "fmtlib");
	dlg_debug("Print data like this: [{}] is pretty damn {}", 42, "convenient");

	std::cout << "\n";
	std::cout << "This is a message sent to std::cout\n";
	std::cerr << "And this one sent to std::cerr\n";
	std::cout << "They should not muddle with dlg in any way, i hope!\n";
	std::cout << "(The empty lines are done explicitly)\n";
	std::cout << "\n";

	// we can also filter out certain message, switch the outputs
	// we set the function that is called everytime somethin is to be outputted
	dlg::output_handler([](const dlg::Origin& origin, std::string_view msg){
		// don't print anything below warning messages
		// note that if this is done for performance reasons or statically project-wide
		// prefer to use the config macros since they will result in zero
		// compile and runtime overhead, this way will not
		if(origin.level < dlg::Level::warn)
			return;

		// we could e.g. also filter out certain tags
		if(std::find(origin.tags.begin(), origin.tags.end(), "filtered") != origin.tags.end())
			return;

		// depending on tags/type/level or even based on the messages content (probably
		// a bad idea) we an print the output to different streams/files
		// in this case we check if the origin has an expression associated, i.e. if
		// it came from an assert call and then write it without color to a file stream.
		// everything else is dumped with color to cout
		// NOTE: the simple color switch here will lead to troubles when cout
		// is e.g. redirected to a file (but could be useful when using unix's 'less -R')
		std::ostream* os = &std::cout;
		bool use_color = true;
		if(!origin.expr.empty()) {
			static std::ofstream log_file ("example_assert_log.txt");
			if(log_file.is_open()) {
				os = &log_file;
				use_color = false;
			}
		}

		// we could add additional switches for file/line

		// we call the generic output handler that will take care
		// of formatting the origin (tags/type/expression/level/file/line) and color
		// and also gives us utf-8 console output support on windows.
		// this could also be handled manually if wished (there is some utility available
		// in dlg for that as well)
		dlg::generic_output_handler(*os, origin, msg, use_color);
	});

	// test out our custom output handler
	// assertions go into a file now
	// there are also custom assertion levels
	dlg_assert_debug(42 * 42 == -42); // should not be printed into file, level too low
	dlg_assert_critical("dlg"[0] == 42); // should be printed into file
	dlg_assert(false, "Error assert message"); // default assert level is error, printed to file

	// test the tag filtering
	dlg_trace("ayyy, this is never shown"); // level too low, not printed
	dlg_warn("Anyone for some beautiful yellow color?"); // printed to cout
	dlg_error(); // allowed as well (can be disabled per config), printed to cout
	dlg_critical("filtered"_tag, "I am {}: feelsbadman", "useless"); // not printed, filtered

	// reset the output handler
	dlg::output_handler(dlg::default_output_handler);

	dlg_assert(true == false, "Assertions are printed to cout again"); // printed to cout
	dlg_trace("filtered"_tag, "I am printed again, yeay"); // printed to cout

	dlg_info("If you don't like the colors here, don't despair; they may be changed!");
	dlg_info("Also make sure to check out the example_assert_log file i printed to\n");

	dlg_critical("What now follows are the important bottom lines");
	dlg_info("You must be really confused right now if you don't know what this program is");
	dlg_info("Just read the dlg example file which uses all this to show off");
	dlg_info("Now, it is time. I'm a busy program after all. A good day m'lady or my dear sir!");

	// some of the utility functions for outputting custom stuff
	auto style = dlg::default_text_style(dlg::Level::trace);
	style.style = dlg::Style::italic;
	dlg::default_output(std::cout, style, "*tips fedora and flies away*\n");
}
