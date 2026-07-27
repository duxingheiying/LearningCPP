#include "LearningCPP.h"

#include <syncstream>

#include <fstream>
#include <chrono>
#include <map>
#include <string>

#include <mutex>
#include <thread>
#include <condition_variable>

#include <vector>
#include <algorithm>
#include <execution>

#include <atomic>

// test base thread
std::mutex page_mutex;
std::map<std::string, std::string> pages;


// test lock guard
std::mutex counter_mutex;
int counter{ 0 };

// unique lock
std::mutex work_thread_mutex;
std::condition_variable cv;
std::string data;
bool ready = false;
bool processed = false;

// aomic
std::atomic_int acnt;
int cnt;

void savePage(const std::string& url)
{
	std::cout << __FUNCTION__ << std::endl;
	std::cout << "sleep time : " << std::chrono::seconds(1).count() << std::endl;

	page_mutex.lock();

	// simulate a long page fetch
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::string result = "fake content";
	pages[url] = result;

	page_mutex.unlock();
}

void PrintFunction()
{
	for (int i = 0; i < 10; i++) {
		std::cout << "you have runed a function in thread!" << std::endl;
	}
}

// test lock_guard
void ConterFunction()
{
	std::lock_guard<std::mutex>lock(counter_mutex);
	for (int i = 0; i < 100000; i++) {
		counter++;
	}
}


// test try catch function
void tyCatch()
{
	try {
		std::ofstream file_stream("c:/text.txt");
		if (file_stream.is_open()) {
			file_stream << "hello world!!";
		}
		else {
			std::cout << "can't open file" << std::endl;
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
}

// test std::sort function
void sortData()
{
	std::vector<int>vec{ 5,3,6,9,1,4,2,7,8 };
	std::sort(std::execution::par_unseq, vec.begin(), vec.end(), [](const int& a, const int& b)->bool {return a > b; });
	for (const auto& value : vec) {
		std::cout << value << "  ";
	}
	std::cout << std::endl;
}

// test condition_variable
void workerThread()
{
	std::unique_lock lk(work_thread_mutex);
	cv.wait(lk, []() {return ready; });
	std::cout << "worker thread is processing data" << std::endl;;
	data += " after process\n";
	processed = true;
	std::cout << "worker thread signals data processing completed" << std::endl;;

	lk.unlock();
	cv.notify_one();
}

// test atomic
void f()
{
	for (auto n{ 10000 }; n; --n) {
		++acnt;
		//acnt.fetch_add(1, std::memory_order_relaxed);
		++cnt;
	}
}

int solution(std::vector<int>& A) {
	// Implement your solution here

	std::sort(A.begin(), A.end(), [](int a, int b) {return a < b; });

	int temp_value{};
	for (size_t i = 0; i < A.size(); i++) {
		if (i == 0) {
			temp_value = A[i];
		}
		else {
			if (A[i] - temp_value ==1) {
				temp_value = A[i];
			}
			else if (A[i] - temp_value == 0) {
				temp_value = A[i];
			}
			else {
				break;
			}
		}
	}

	if (temp_value >= 0) {
		return temp_value + 1;
	}
	else {
		return 1;
	}
}

int main()
{
	//std::jthread jthread;
	//std::thread temp_thread(PrintFunction);
	//temp_thread.join();
	//temp_thread.detach();
	//std::cout << "you have created a thread!! " << std::endl;

	//// test base thread joint function
	//std::thread thread_1(savePage, "http://foo");
	//std::thread thread_2(savePage, "http://bar");
	//thread_1.join();
	//thread_2.join();
	//for (const auto& [url, page] : pages) {
	//	std::cout << url << " => " << page << std::endl;
	//}

	//// test lock_guard
	//std::thread thread_counter_1(ConterFunction);
	//std::thread thread_counter_2(ConterFunction);
	//thread_counter_1.join();
	//thread_counter_2.join();
	//std::cout << counter << std::endl;


	//// test condition variable
	//std::thread worker_thread(workerThread);
	//data = "Example data\n";
	//{
	//	std::lock_guard lk(work_thread_mutex);
	//	ready = true;
	//	std::cout << "main() signals data ready for processing" << std::endl;;
	//}
	//cv.notify_one();

	//{
	//	std::unique_lock lk(work_thread_mutex);
	//	cv.wait(lk, [] {return processed; });
	//}

	//std::cout << "Back in main(), data=" << data << std::endl;

	//worker_thread.join();


	//ThreadSafeCounter counter;
	//std::mutex cout_mutex; // 在文件顶部添加

	//// 替换原有 lambda 中的输出部分
	//auto increment_and_print = [&counter, &cout_mutex]()
	//	{
	//		for (int i = 0; i != 3; ++i) {
	//			counter.increment();
	//			{
	//				std::lock_guard<std::mutex> lock(cout_mutex);
	//				std::cout << std::this_thread::get_id() << ' ' << counter.get() << '\n';
	//			}
	//		}
	//	};

	//std::thread shear_thread_1(increment_and_print);
	//std::thread shear_thread_2(increment_and_print);
	//shear_thread_1.join();
	//shear_thread_2.join();

	//// test atomic
	//{
	//	std::vector<std::thread> pool;
	//	for (int n = 0; n < 10; ++n) {
	//		pool.emplace_back(f);
	//	}
	//	for (auto& th : pool) {
	//		th.join();
	//	}
	//}

	//std::cout << " The atomic counter is " << acnt << "\n"
	//	<< "The non-atomic counter is " << cnt << "\n";

	//sortData();
	//tyCatch();

	std::vector<int> A{ 1, 3, 6, 4, 1, 2 };

	std::cout << *A.rbegin() << std::endl;;

	return 0;
}

