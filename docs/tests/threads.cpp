#include <dlg/dlg.hpp>
#include <thread>
#include <chrono>

// mainly used to test that everything works multithreaded and that
// there will be no leaks

int main() {
	auto tid1 = std::this_thread::get_id();
	dlg::set_handler([&](const struct dlg_origin& origin, const char* str){
		auto t = (std::this_thread::get_id() == tid1) ? "thread-1: " : "thread-2: ";
		dlg_win_init_ansi();
		dlg_fprintf(stdout, "%s %s", t, dlg::generic_output(~0u, origin, str).c_str());
	});


	dlg_info("main");
	dlg_info("initing thread 2");

	std::thread t([&](){
		dlg_info("hello world from thread 2");
		dlg_warn("Just a small {}", "warning");
		dlg_info("Goodbye from thread 2");

		for(auto i = 0u; i < 10; ++i) {
			dlg_warn("Some nice yellow warning (hopefully)");
			std::this_thread::sleep_for(std::chrono::nanoseconds(10));
		}
	});

	dlg_error("Just some messages from the main thread");
	dlg_info("Waiting on thread 2 to finally spawn...");
	dlg_info("Hurry ffs!");
	dlg_info("Well i guess i go to sleep for now...");
	dlg_info("Wake me up then!");

	for(auto i = 0u; i < 10; ++i) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(i * 10));
		dlg_info("Did he said something already?");
	}

	dlg_info("Hopefully he said something by now...");
	dlg_info("Goodbye from main thread");
	t.join();
}