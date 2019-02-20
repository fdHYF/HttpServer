#include "Thread.h"
#include <sys/prctl.h>
#include <cassert>
#include <iostream>

Thread::Thread(const Func& func, const std::string& thread_name) :
	thread_id_(0),
	thread_name_(thread_name),
	thread_func_(func),
	started_(false),
	joined_(false)
{
	setThreadName();
}

Thread::~Thread() {
	if (started_ && !joined_) {
		pthread_detach(thread_id_);
	}
}

void Thread::setThreadName() {
	const char* name = thread_name_.empty() ? "DefaultName" : thread_name_.c_str();
	prctl(PR_SET_NAME, name);
}

void* Thread::entry(void* arg) {
	Thread* thread_ptr = (Thread*)arg;
	thread_ptr->thread_func_();
	return nullptr;
}

void Thread::start() {
	assert(!started_);
	started_ = true;
	if (pthread_create(&thread_id_, nullptr, entry, (void*)this) != 0) {
		started_ = false;
	}
	else {
		std::cout << "Create thread success" << std::endl;
	}
}

int Thread::join() {
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(thread_id_, nullptr);
}