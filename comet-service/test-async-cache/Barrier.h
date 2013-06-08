#pragma  once
#include <mutex>
#include <condition_variable>

struct Barrier {
	std::mutex m_mutex;
	std::condition_variable m_cond_variable;
	int count;
	Barrier(int number)
		:count(number-1)
	{}

	std::unique_lock<std::mutex> makeLock() {
		return std::unique_lock<std::mutex>(m_mutex);
	}

	void notify_one() {
		count--;
		m_cond_variable.notify_one();
	}

	void wait(std::unique_lock<std::mutex>& locker) {
		m_cond_variable.wait(locker, [&] () -> bool {return count == 0;});
	}
};