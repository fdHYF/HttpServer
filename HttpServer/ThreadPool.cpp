#include "ThreadPool.h"
#include <exception>
#include <cassert>
#include <iostream>
ThreadPool::ThreadPool(const int& thread_number, const int& request_size) :
	thread_number_(thread_number),
	request_szie_(request_size),
	request_number_(0),
	stopped_(false),
	mutex_(),
	cond_(mutex_)
{
	if (thread_number_ < 0 || thread_number_ > MAX_THREAD_NUM ||
		request_size_ < 0 || request_size_ > MAX_REQUEST_NUM) {
		//如果线程池线程数量和请求数量不合规范采用默认值
		thread_number_ = 4;
		request_size_ = 10000;
	}
	std::vector<pthread_t> ini_thread(thread_number_);
	threads = std::move(ini_thread);
	for (int i = 0; i < thread_number_; ++i) {
		if (pthread_create(&threads[i], nullptr, worker, this) != 0) {
			throw std::exception();
		}
		if (pthread_detach(threads[i]) != 0) {
			throw std::exception();
		}
	}
}

void* ThreadPool::worker(void* arg) {
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	pool->run();
	return nullptr;
}

void ThreadPool::append(RequestTask request) {
	//MutexLockGuard lock(mutex_);
	//while (request_number_ > MAX_REQUEST_NUM)
	//	cond_.wait();
	//以上如果request_number_ > MAX_REQUEST_NUM会造成死锁
	while (request_number_ > MAX_REQUEST_NUM) {
		cond_.wait();
	}
	MutexLockGuard lock(mutex_);
	requests.push_back(request);
	++request_number_;
    	cond_.notifyAll();
}

void ThreadPool::run() {
    stopped_ = false;
	while (!stopped_) {
       		RequestTask req;
		if (requests.empty())
			continue;
   		{
			MutexLockGuard lock(mutex_);
			req = requests.front();
			requests.pop_front();
			--request_number_;
		}
    	(req.func)(req.data, req.event);
	}
}
