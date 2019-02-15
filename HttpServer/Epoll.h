#pragma once
#include <sys/epoll.h>
#include "Channel.h"
#include "Request.h"
#include "Timer.h"
#include <vector>
#include <memory>
#include <stdint.h>

const int EVENTSUM = 4096;
const int TIME_WAIT = 10000;

class Epoll {
public:
	Epoll();
	~Epoll() {};
	int epoll_add(std::shared_ptr<Channel> channel, size_t timeout, uint32_t events);	//����µ�������
	int epoll_mod(std::shared_ptr<Channel> channel, size_t timeout, uint32_t events);	//�޸�������
	int epoll_del(std::shared_ptr<Channel> channel, uint32_t events);					//ɾ������

	int epoll();
	void add_timer(std::shared_ptr<Channel> channel, size_t timeout);
	std::vector<std::shared_ptr<Channel>> handle_event(int num, int listenfd);

	void getEpollFd() { return epollfd_; }
private:
	static const int maxFds_ = 100000;
	int epollfd_;
	std::vector<epoll_event> events_;
	Timer timer_;
	std::shared_ptr<HttpData> datas_[maxFds_];
	std::shared_ptr<Channel> channels_[maxFds_];
};