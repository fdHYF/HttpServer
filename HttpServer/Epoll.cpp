#include "Epoll.h"
#include <cassert>
#include <cstdio>
#include "logging//Log.h"


Epoll::Epoll() :
	epollfd_(epoll_create1(EPOLL_CLOEXEC)),
	events_(EVENTSUM)
{
	assert(epollfd_ > 0);
}

int Epoll::epoll_add(std::shared_ptr<Channel> channel, size_t timeout, uint32_t events) {
	int fd = channel->fd();
	if (timeout > 0) {
		add_timer(channel, timeout);
		datas_[fd] = channel->getHolder();
	}
	struct epoll_event event;
	event.data.ptr = fd;
	event.events = events;
	channels_[fd] = channel;
	if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) < 0) {
		LOG << "ERROR: epoll add";
		channels_[fd].reset();
		return -1;
	}
	return 0;
}

int Epoll::epoll_mod(std::shared_ptr<Channel> channel, size_t timeout, uint32_t events) {
	int fd = channel->fd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0) {
		LOG << "ERROR: epoll mod";
		channels_[fd].reset();
		return -1;
	}
	return 0;
}

int Epoll::epoll_del(std::shared_ptr<Channel> channel, uint32_t events) {
	int fd = channel->fd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event) < 0) {
		LOG << "ERROR: epoll del";
		return -1;
	}
	datas_[fd].reset();
	channels_[fd].reset();
	return 0;
}

int Epoll::epoll() {
	int count = epoll_wait(epollfd_, &*events_.begin(), events_.size(), TIME_WAIT);
	if (count < 0) {
		LOG << "epoll wait error";
	}
	assert(count >= 0);
	return count;
}

std::vector<std::shared_ptr<Channel>> Epoll::handle_event(int num, int listenfd) {
	std::vector<std::shared_ptr<Channel>> ret;
	for (int i = 0; i < num; ++i) {
		int fd = events_[i].data.fd;
		std::shared_ptr<Channel> channel = datas_[fd];
		if (fd = listenfd) {
			struct sockaddr_in client_address;
			socklen_t len = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &len);
			if (connfd < 0) {
				LOG << "accept error";
				continue;
			}
		}
		else {
			if ((events_[i].events & EPOLLERR) || (events_[i].events & EPOLLHUP)) {
				close(fd);
				continue;
			}
		}
		if (channel) {
			channel->set_events(events_[i].events);
			ret.push_back(channel);
			//将任务加入线程池
			//RequestTask task;
			//task.func = std::bind(&Channel::handleEvent, channel, nullptr);
			//task.args = nullptr;
		}
		else {
			LOG << "This channel not valid";
		}
		return ret;
	}
}

void Epoll::add_timer(std::shared_ptr<Channel> channel, size_t timeout) {
	std::shared_ptr<HttpData> tmp = channel->getHolder();
	if (tmp)
		timer_.add_node(tmp, timeout);
	else
		LOG << "the add node is error";
}