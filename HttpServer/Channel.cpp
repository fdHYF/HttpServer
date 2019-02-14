#include "Channel.h"

const int Channel::NoneEvent = 0;
const int Channel::ReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::WriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd) :
	loop_(loop),
	fd_(fd),
	events_(0)
{}

void Channel::update() {




}

void Channel::handleEvent() {
	if (events_ & EPOLLERR) {
		if (errorCallBack_)
			errorCallBack_();
	}
	if (events_ & (EPOLLIN | EPOLLPRI | EPOLLHUP)) {
		if (readCallBack_)
			readCallBack_();
	}
	if (events_ & EPOLLOUT) {
		if (writeCallBack_)
			writeCallBack_();
	}
}