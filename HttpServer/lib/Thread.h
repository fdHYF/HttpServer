#pragma once
#include <pthread.h>
#include <functional>
#include <string>


class Thread {
public:
	using thread_type = pthread_t;
	using Func = std::function<void()>;

	explicit Thread(const Func& thread_func, const std::string& thread_name = std::string());
	~Thread();

	void start();		//�����߳�
	int join();
	bool started() const { return started_; }
	const std::string& name() const { return thread_name_; }
private:
	void setThreadName();			//����Ĭ�ϵ��߳�����
	static void* entry(void* arg);
	thread_type thread_id_;
	const std::string& thread_name_;
	Func thread_func_;
	bool started_;
	bool joined_;
};
