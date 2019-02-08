#pragma once
#include <pthread.h>
#include "Mutex.h"
#include <sys/time.h>

class Condition {
public:
	explicit Condition(MutexLock& mutex) :
		mutex_(mutex)
	{
		pthread_cond_init(&cond_, nullptr);
	}

	~Condition() { pthread_cond_destroy(&cond_); }

	Condition(const Condition& rhs) = delete;
	Condition& operator=(const Condition& rhs) = delete;

	void wait() { pthread_cond_wait(&cond_, mutex_.getMutex()); }
	void timewait(size_t seconds) {
		struct timespec tsp;
		struct timeval now;
		gettimeofday(&now, nullptr);
		tsp.tv_sec = now.tv_sec + seconds;
		tsp.tv_nsec = now.tv_usec * 1000;
		pthread_cond_timedwait(&cond_, mutex_.getMutex(), &tsp);
	}
	void notify() { pthread_cond_signal(&cond_); }
	void notifyAll() { pthread_cond_broadcast(&cond_); }
private:
	MutexLock& mutex_;
	pthread_cond_t cond_;
};
