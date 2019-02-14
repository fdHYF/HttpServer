#pragma once
#include <stdint.h>
#include <functional>
#include <sys/epoll.h>
#include <memory>
#include "Request.h"

class EventLoop;

class Channel {
public:
	using EventCallBack = std::function<void()>;
	Channel(EventLoop* loop, int fd);

	void handleEvent();
	void setReadCallBack(const EventCallBack& cb) {
		readCallBack_ = cb;
	}

	void setWriteCallBack(const EventCallBack& cb) {
		writeCallBack_ = cb;
	}

	void setErrorCallBack(const EventCallBack& cb) {
		errorCallBack_ = cb;
	}

	int fd() const { return fd_; }
	std::shared_ptr<HttpData> getHolder() { return holder_.lock(); }
	uint32_t& events() { return events_; }
	void set_events(uint32_t events) { events_ = events }

private:
	void update();

	static const int NoneEvent;
	static const int ReadEvent;
	static const int WriteEvent;

	EventLoop* loop_;
	const int fd_;
	uint32_t events_;
	std::weak_ptr<HttpData> holder_;

	EventCallBack readCallBack_;
	EventCallBack writeCallBack_;
	EventCallBack errorCallBack_;
};