#include "Server.h"
#include "logging/Log.h"
#include "Timer.h"
#include "Request.h"
#include "Epoll.h"
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

	std::shared_ptr<Epoll> epoller = std::make_shared<Epoll>();
	sp_httpdata client_data = std::make_shared<HttpData>(listenfd);
	epoller->epoll_add(client_data, TIME_WAIT, EPOLLIN | EPOLLET);
	while (true) {
		int num = epoller->epoll();
		//assert(num > 0);
		epoller->handle_expired();
		epoller->handle_event(num, listenfd);
	}
	return 0;
}
