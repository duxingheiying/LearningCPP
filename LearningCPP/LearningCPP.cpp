#include "LearningCPP.h"

#include <chrono>
#include <map>
#include <vector>
#include <mutex>
#include <string>
#include <thread>
#include <condition_variable>

std::map<std::string, std::string> pages;
std::mutex page_mutex;

int counter{ 0 };

void savePage(const std::string& url)
{
	std::cout << __FUNCTION__ << std::endl;
	std::cout << "sleep time : " << std::chrono::seconds(2).count() << std::endl;

	std::lock_guard<std::mutex> guard(page_mutex);

	// simulate a long page fetch
	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::string result = "fake content";

	for (int i = 0; i < 100000; i++) {
		counter++;
	}
	pages[url] = result;
}

void PrintFunction()
{
	for (int i = 0; i < 10; i++) {
		std::cout << "you have runed a function in thread!" << std::endl;
	}
}

int main()
{
	//std::jthread jthread;
	//std::thread temp_thread(PrintFunction);
	//temp_thread.join();
	//temp_thread.detach();
	//std::cout << "you have created a thread!! " << std::endl;

	std::thread thread_1(savePage, "http://foo");
	std::thread thread_2(savePage, "http://bar");

	thread_1.join();
	thread_2.join();

	for (const auto& [url, page] : pages) {
		std::cout << url << " => " << page << std::endl;
	}

	std::cout << counter << std::endl;

	std::vector<int> value_list;
	int n = 2;

	// deal with tr catch logic
	try {
		if (n > 1) {
			throw std::logic_error("index is out of range!!");
		}
	}
	catch (const std::out_of_range& range) {
		std::cout << range.what() << std::endl;
	}
	catch (const std::logic_error& logic) {
		std::cout << logic.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}
