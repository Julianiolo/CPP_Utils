#include "ThreadPool.h"

#include <chrono>
#include <ratio>


cppu::ThreadPool::ThreadPool(size_t num_threads_) : num_threads(num_threads_) {
	if(num_threads == (decltype(num_threads))-1)
		num_threads = std::thread::hardware_concurrency();
}
cppu::ThreadPool::~ThreadPool() {
	stop();
}

void cppu::ThreadPool::start() {
	if (threads.size() != 0) // already running
		return;

	should_terminate = false;

	std::unique_lock<std::mutex> lock(queue_mutex);
	threads.reserve(num_threads);
	for (size_t i = 0; i < num_threads; i++) {
		threads.push_back(std::thread([this] {
			threadRun();
		}));
	}
}
void cppu::ThreadPool::stop() {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		should_terminate = true;
	}
	mutex_condition.notify_all();
	for (auto& thread : threads) {
		thread.join();
	}
	threads.clear();
}

bool cppu::ThreadPool::busy() {
	bool busy;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		busy = !jobs.empty();
	}
	return busy;
}
bool cppu::ThreadPool::running() const {
	return threads.size() > 0;
}

void cppu::ThreadPool::addJob(const std::function<void(void)>& job) {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		jobs.push(job);
	}
	mutex_condition.notify_one();
}

void cppu::ThreadPool::threadRun() {
	while (true) {
		std::function<void(void)> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			if (should_terminate) {
				return;
			}

			mutex_condition.wait(lock, [this] {
				return !jobs.empty() || should_terminate;
			});
			if (should_terminate) {
				return;
			}
			job = jobs.front();
			jobs.pop();
		}
		job();
	}
}

bool cppu::ThreadPool::shouldStop() const {
	return should_terminate;
}


// #######################################################



cppu::DynamicThreadPool::DynamicThreadPool(size_t max_num_threads_, float thread_shutdown_time) : max_num_threads(max_num_threads_), thread_shutdown_time(thread_shutdown_time) {
	if(max_num_threads == (decltype(max_num_threads)::value_type)-1)
		max_num_threads = std::thread::hardware_concurrency();
}
cppu::DynamicThreadPool::~DynamicThreadPool() {
	stop();
}

void cppu::DynamicThreadPool::addThreads(size_t n) {
	//std::unique_lock<std::mutex> lock(queue_mutex);
	for (size_t i = 0; i < n; i++) {
		num_currently_running++;
		//printf("++: %" CU_PRIuSIZE "\n", num_currently_running.load());
		std::thread([this] {
			threadRun();
		}).detach();
	}
}

void cppu::DynamicThreadPool::start() {
	if (num_currently_running != 0) // already running
		return;

	should_terminate = false;
}
void cppu::DynamicThreadPool::stop() {
	should_terminate = true;
	mutex_condition.notify_all();

	while(num_currently_running > 0) {  // TODO maybe improve
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
	}
}

bool cppu::DynamicThreadPool::amINotNeeded() const {
	return num_currently_running > max_num_threads;
}


bool cppu::DynamicThreadPool::busy() {
	bool busy;
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		busy = !jobs.empty();
	}
	return busy;
}
bool cppu::DynamicThreadPool::running() {
	return num_currently_running > 0;
}

void cppu::DynamicThreadPool::addJob(const std::function<void(void)>& job) {
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		jobs.push(job);

		if (num_currently_running < max_num_threads) {
			addThreads(1);
		}
	}
	
	mutex_condition.notify_one();
}

void cppu::DynamicThreadPool::threadRun() {
	while (true) {
		std::function<void(void)> job;
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			if (should_terminate || amINotNeeded()) {
				goto stop_myself;
			}

			const std::function<bool()> cond_func = [this] {
				return !jobs.empty() || should_terminate || amINotNeeded();
			};

			if (thread_shutdown_time > 0) {
				const auto tst = std::chrono::duration<float, std::ratio<1,1>>(thread_shutdown_time);
				const bool ret = mutex_condition.wait_for(lock, tst, cond_func);

				if (!ret) goto stop_myself; // timeout
			}
			else {
				mutex_condition.wait(lock, cond_func);
			}
			

			if (should_terminate || amINotNeeded()) {
				goto stop_myself;
			}
			job = jobs.front();
			jobs.pop();
		}
		job();
	}

stop_myself:
	num_currently_running--;
	//printf("--: %" CU_PRIuSIZE "\n", num_currently_running.load());
	return;
}

size_t cppu::DynamicThreadPool::getMaxNumThreads() const {
	return max_num_threads;
}
void cppu::DynamicThreadPool::setNumThreads(size_t n) {
	if(n == (decltype(n))-1)
		n = std::thread::hardware_concurrency();

	const size_t old_num_threads = max_num_threads;

	max_num_threads = n;

	if(should_terminate) // not started yet
		return;

	if (n < old_num_threads) {
		// wake up threads so they can stop themselfs
		for (size_t i = 0; i < old_num_threads - n; i++) {
			mutex_condition.notify_one();
		}
	}
}

bool cppu::DynamicThreadPool::shouldStop() const {
	return should_terminate;
}
