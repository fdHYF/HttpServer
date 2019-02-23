#pragma once
#include <sys/epoll.h>
#include "Channel.h"
#include "Request.h"
#include "Timer.h"
#include <vector>
#include <map>
#include <memory>
#include <stdint.h>

const int MAXEVENTS = 1024;
const int TIME_WAIT = 10000;
static const int maxFds_ = 100000;

class Epoll {
public:
	using sp_channel = std::shared_ptr<Channel>;

	Epoll();
	~Epoll() {};
	int epoll_add(sp_channel channel, size_t timeout, int events);	//Ìí¼ÓĞÂµÄÃèÊö·û
	int epoll_mod(sp_channel channel, size_t timeout, int events);	//ĞŞ¸ÄÃèÊö·û
	int epoll_del(sp_channel channel, int events);					//É¾³ıÃèÊö

	int epoll();
	void add_timer(sp_channel, size_t timeout);
	std::vector<sp_channel> handle_event(int num, int listenfd);

	int getEpollFd() { return epollfd_; }
	Timer get_timer() { return timer_; }
private:
	int epollfd_;
	std::vector<struct epoll_event> events_;
	Timer timer_;
	//std::vector<std::shared_ptr<HttpData>> datas_;
	//std::vector<sp_channel> channels_;
    //std::map<int, sp_channel> channels_;
	//std::shared_ptr<Channel> channels_[maxFds_];
};