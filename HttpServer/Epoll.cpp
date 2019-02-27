#include "Epoll.h"
#include <cassert>
#include <cstdio>
#include "logging/Log.h"
#include "Timer.h"
#include "ThreadPool.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

extern int set_socket_nonblocking(int sockfd);
//创建线程池,采用默认值
static ThreadPool pool;

Epoll::Epoll() :
	epollfd_(epoll_create1(EPOLL_CLOEXEC)),
	events_(MAXEVENTS)
{
	assert(epollfd_ > 0);
}

int Epoll::epoll_add(sp_httpdata data, int timeout, int events) {
	if (timeout > 0)
		add_timer(data, timeout);
	struct epoll_event event;
	int fd = data->fd();
	event.data.fd = fd;
	event.events = events;
	datas_[fd] = data;
	if ((epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event)) < 0) {
		datas_[fd].reset();
		return -1;
	}
	return 0;
}

int Epoll::epoll_mod(sp_httpdata data, int timeout, int events) {
	if (timeout > 0)
		add_timer(data, timeout);
	int fd = data->fd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if ((epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event)) < 0) {
		datas_[fd].reset();
		return -1;
	}
	return 0;
}

int Epoll::epoll_del(sp_httpdata data, int events) {
	int fd = data->fd();
	struct epoll_event event;
	event.data.fd = fd;
	event.events = events;
	if ((epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event)) < 0) {
		return -1;
	}
	datas_[fd].reset();
	return 0;
}

int Epoll::epoll() {
	int num = epoll_wait(epollfd_, &*events_.begin(), maxfds, TIME_WAIT);
	return num;
}

void Epoll::handle_event(int listenfd, int num) {
	for (int i = 0; i < num; ++i) {
		int fd = events_[i].data.fd;
		sp_httpdata data = datas_[fd];
		if (fd == listenfd) {
			struct sockaddr_in client_addr;
			memset(&client_addr, 0, sizeof(struct sockaddr_in)); 
			socklen_t len = sizeof(client_addr);
			int accept_fd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
			if (accept_fd == -1)
				perror("accept:");
			LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
				<< ntohs(client_addr.sin_port);
			set_socket_nonblocking(accept_fd);
			if (accept_fd > maxfds) {
				close(fd);
				continue;
			}
			sp_httpdata tmp = std::make_shared<HttpData>(accept_fd);
			epoll_add(tmp, TIME_WAIT, (EPOLLIN | EPOLLET | EPOLLONESHOT));
		}
		else {
			//有事件发生的描述符是连接描述符
			//排除错误事件
			if ((events_[fd].events & EPOLLERR) || (events_[fd].events & EPOLLHUP) ||
				(!(events_[fd].events & EPOLLIN))) {
				//epoll_del(data, events_[fd].events);
				close(fd);
				continue;
			}
			RequestTask task;
			if (events_[i].events & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
				task.func = std::bind(&HttpData::handleRead, data);
			else if (events_[i].events & EPOLLOUT)
				task.func = std::bind(&HttpData::handleWrite, data);
			pool.append(&task);
		}
	}
}