#pragma once
#include <pthread.h>
//RAII手法对互斥锁的封装，避免重复解锁或忘记解锁
//C++11的delete关键字复制操作

class MutexLock {
public:
	//采用默认属性的互斥锁，在Linux中被映射为PTHREAD_MUTEX_NORMAL(非递归)
	MutexLock() { pthread_mutex_init(&mutex_, nullptr); }
	~MutexLock() { pthread_mutex_destroy(&mutex_); }
	MutexLock(const MutexLock& rhs) = delete;
	MutexLock& operator=(const MutexLock& rhs) = delete;

	void lock() { pthread_mutex_lock(&mutex_); }
	void unlock() { pthread_mutex_unlock(&mutex_); }
	pthread_mutex_t* getMutex() { return &mutex_; }
private:
	pthread_mutex_t mutex_;
};

class MutexLockGuard {
public:
	explicit MutexLockGuard(MutexLock& mutex) :
		mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLockGuard() { mutex_.unlock(); }

	MutexLockGuard(const MutexLockGuard& rhs) = delete;
	MutexLockGuard& operator=(const MutexLockGuard& rhs) = delete;
private:
	MutexLock& mutex_;
};