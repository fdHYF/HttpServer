#include "Epoll.h"
#include <cassert>
#include <cstdio>
#include "logging/Log.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


Epoll::Epoll() :
	epollfd_(epoll_create1(EPOLL_CLOEXEC)),
	events_(MAXEVENTS)
{
	assert(epollfd_ > 0);
}

int Epoll::epoll_add(sp_channel channel, size_t timeout, uint32_t events) {
	int fd = channel->fd();
	if (timeout > 0)
		add_timer(channel, timeout);
	struct epoll_event event;
	event.data.ptr = static_cast<void*>(channel.get());
	event.events = events;
	int ret = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event);
	if (ret == -1) {
		LOG << "epoll_add error";
		return -1;
	}
	return 0;
}

int Epoll::epoll_mod(sp_channel channel, size_t timeout, uint32_t events) {
	if (timeout > 0)
		add_timer(channel, timeout);
	int fd = channel->fd();
	struct epoll_event event;
	event.data.ptr = static_cast<void*>(channel.get());
	event.events = events;
	if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0) {
		LOG << "epoll_mod error";
		return -1;
	}
	return 0;
}

int Epoll::epoll_del(sp_channel channel, uint32_t events) {
	int fd = channel->fd();
	struct epoll_event event;
	event.data.ptr = static_cast<void*>(channel.get());
	event.events = events;
	if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event) < 0) 
		return -1;
	return 0;
}

int Epoll::epoll() {
	int count = epoll_wait(epollfd_, &*events_.begin(), MAXEVENTS, TIME_WAIT);
	if (count < 0) {
		return -1;
	}
	timer_.handle_expired();
	assert(count >= 0);
	return count;
}

std::vector<std::shared_ptr<Channel>> Epoll::handle_event(int num, int listenfd) {
	std::vector<std::shared_ptr<Channel>> ret;
	for (int i = 0; i < num; ++i) {
		sp_channel channel((Channel*)events_[i].data.ptr);
		int fd = channel->fd();
		if (fd == listenfd) {
			struct sockaddr_in client_address;
			socklen_t len = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &len);
			if (connfd < 0) {
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
	}
	return ret;
}

void Epoll::add_timer(sp_channel channel, size_t timeout) {
	std::shared_ptr<HttpData> tmp(new HttpData(channel->fd()));
	if (tmp)
		timer_.add_node(tmp, timeout);
	else
		LOG << "the add node is error";
}
