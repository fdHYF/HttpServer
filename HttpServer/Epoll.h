#pragma once
#include <sys/epoll.h>
#include "Request.h"
#include "Timer.h"
#include <vector>
#include <memory>
#include <stdint.h>

const int MAXEVENTS = 1024;
const int TIME_WAIT = 10000;
static const int maxfds = 10000;
using sp_httpdata = std::shared_ptr<HttpData>;

class Epoll {
public:
	Epoll();
	~Epoll() {};

	int epoll_add(sp_httpdata data, int timeout, int events);
	int epoll_mod(sp_httpdata data, int timeout, int events);
	int epoll_del(sp_httpdata data, int events);

	int epoll();
	void handle_event(int fd, int num);
	void handle_expired() { timer_.handle_expired(); }
	int get_epollfd() { return epollfd_; }
	void add_timer(sp_httpdata data, int timeout) { 
		timer_.add_node(data, timeout); 
	}
    void thread_func(sp_httpdata data, int event);
private:
	int epollfd_;
	std::vector<struct epoll_event> events_;
	std::vector<sp_httpdata> datas_{ maxfds };
	Timer timer_;
};
