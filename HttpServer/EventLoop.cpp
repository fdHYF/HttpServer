#include "EventLoop.h"
#include "logging/log.h"
#include <cassert>

thread_local EventLoop* loopInThisThread = nullptr;

EventLoop::EventLoop() :
	looping_(false),
	pid_(pthread_self())
{
	LOG << "EventLoop created in thread " << pid_;
	assert(loopInThisThread == nullptr);
	loopInThisThread = this;
}

EventLoop::~EventLoop() {
	assert(!looping_);
	loopInThisThread = nullptr;
}

void EventLoop::loop() {
	assert(!looping_);
	assert(loopInThisThread == this);
	looping_ = true;

	LOG << "EventLoop " << this << "stop looping";
	looping_ = false;
}