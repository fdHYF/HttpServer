#pragma once
#include <pthread.h>

class EventLoop {
public:
	EventLoop();
	~EventLoop();
	EventLoop(const EventLoop& rhs) = delete;
	EventLoop& operator=(const EventLoop& rhs) = delete;

	void loop();
private:
	bool looping_;
	const pid_t pid_;


};