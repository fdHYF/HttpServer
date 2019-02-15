#include "EventLoop.h"
#include "Server.h"
#include "logging/log.h"
#include "Channel.h"
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cassert>

int main(int argc, char* argv[]) {
	if (argc <= 2)
		printf("Usage: %s ip_address port_number\n", basename(argv[0]));
	const char* ip = argv[1];
	int port = atoi(argv[2]);
	handle_for_sigpipe();

	//初始化套接字，开始监听
	int listenfd = socket_bind_listen(port);
	//将监听套接字设置为非阻塞
	if (set_socket_nonblocking(listenfd) < 0) {
		perror("set socket nonblocking failed");
		abort();
	}

	std::shared_ptr<Epoll> epoller;
	Channel channel(listenfd);
	epoller->epoll_add(Channel, TIME_WAIT, EPOLLIN | EPOLLET);
	//创建线程池
	ThreadPool pool;
	std::vector<std::shared_ptr<Channel>> active_channel;
	while (true) {
		active_channel.clear();
		int num = epoller->epoll();
		assert(num > 0);
		active_channel = epoller->handle_event(num, listenfd);
		for (auto &tmp : active_channel) {
			RequestTask task;
			task.func = std::bind(&Channel::handleEvent, this, nullptr);
			task.args = nullptr;
			poll.append(&task);
		}
	}
	return 0;
}
