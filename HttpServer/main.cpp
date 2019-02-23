#include "Server.h"
#include "logging/Log.h"
#include "Channel.h"
#include "Timer.h"
#include "Request.h"
#include "Epoll.h"
#include "ThreadPool.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cassert>

extern const int TIME_WAIT;
int main() {
	int port = 8080;
	handle_for_sigpipe();

	//初始化套接字，开始监听
	int listenfd = socket_bind_listen(port);
	//将监听套接字设置为非阻塞
	if (set_socket_nonblocking(listenfd) != 0) {
		perror("set socket nonblocking failed");
	}

	std::shared_ptr<Epoll> epoller(new Epoll);
	HttpData client_data(listenfd);
	std::shared_ptr<Channel> channel = client_data.get_channel();
	assert(channel);
	epoller->epoll_add(channel, TIME_WAIT, EPOLLIN | EPOLLET);
	//创建线程池,采用默认值
	static ThreadPool pool;
	std::vector<std::shared_ptr<Channel>> active_channel;
	while (true) {
		active_channel.clear();
		int num = epoller->epoll();
		assert(num > 0);
		active_channel = epoller->handle_event(num, listenfd);
		for (auto &tmp : active_channel) {
			RequestTask task;
			task.func = std::bind(&Channel::handleEvent, tmp, std::placeholders::_1);
			task.args = nullptr;
			pool.append(&task);
		}
	}
	printf("hello world!\n");
	return 0;
}
