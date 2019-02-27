#include "ThreadPool.h"
#include <exception>
#include <cassert>

ThreadPool::ThreadPool(const int& thread_number, const int& request_number) :
	thread_number_(thread_number),
	request_number_(request_number),
	stopped_(false),
	mutex_(),
	cond_(mutex_)
{
	if (thread_number_ < 0 || thread_number_ > MAX_THREAD_NUM ||
		request_number < 0 || request_number > MAX_REQUEST_NUM) {
		//����̳߳��߳������������������Ϲ淶����Ĭ��ֵ
		thread_number_ = 4;
		request_number_ = 10000;
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

void ThreadPool::append(RequestTask* request) {
	//MutexLockGuard lock(mutex_);
	//while (request_number_ > MAX_REQUEST_NUM)
	//	cond_.wait();
	//�������request_number_ > MAX_REQUEST_NUM���������
	while (request_number_ > MAX_REQUEST_NUM) {
		cond_.wait();
	}
	MutexLockGuard lock(mutex_);
	requests.push_back(request);
}

void ThreadPool::run() {
	while (!stopped_) {
		RequestTask* req = nullptr;
		{
			while (requests.empty())
				cond_.wait();
			MutexLockGuard lock(mutex_);
			req = requests.front();
			requests.pop_front();
		}
		assert(req != nullptr);
		req->func();
	}
}