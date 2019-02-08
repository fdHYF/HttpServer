#pragma once
#include <pthread.h>
//RAII�ַ��Ի������ķ�װ�������ظ����������ǽ���
//C++11��delete�ؼ��ָ��Ʋ���

class MutexLock {
public:
	//����Ĭ�����ԵĻ���������Linux�б�ӳ��ΪPTHREAD_MUTEX_NORMAL(�ǵݹ�)
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