#pragma once
#include "lib/Mutex.h"
#include "lib/Condition.h"
#include <pthread.h>
#include <vector>
#include <list>
#include <memory>
#include <functional>

const int MAX_THREAD_NUM = 8;
const int MAX_REQUEST_NUM = 65535;

//线程池任务类
struct RequestTask {
	void* func(void*);
	void* args;
};

class ThreadPool {
public:
	explicit ThreadPool(const int& thread_number = 4, const int& request_number = 10000);
	~ThreadPool() { stopped_ = true; }
	void append(RequestTask* request);

private:
	static void* worker(void* arg);
	void run();
private:
	int thread_number_;
	int request_number_;
	bool stopped_;
	MutexLock mutex_;
	Condition cond_;
	std::vector<pthread_t> threads;
	std::list<RequestTask*> requests;
};