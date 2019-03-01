#pragma once
#include <memory>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>

int set_socket_nonblocking(int sockfd) {
	int flag = fcntl(sockfd, F_GETFL, 0);
	if (flag == -1)
		return -1;
	flag |= O_NONBLOCK;
	if (fcntl(sockfd, F_SETFL, flag) == -1)
		return -1;
	return 0;
}

int socket_bind_listen(int port) {
	if (port < 0 || port > 65535)
		return -1;

	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		return -1;
	//设置服务器IP和port，与listenfd进行绑定
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		return -1;
	if (listen(listen_fd, 2048) == -1)
		return -1;
	if (listen_fd == -1) {
		close(listen_fd);
		return -1;
	}
	return listen_fd;
}

void shut_down(int sockfd, int how) {
	shut_down(sockfd, how);
	//how:SHUT_RD,关闭读端
	//how:SHUT_WR,关闭写端
}

void handle_for_sigpipe() {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE, &sa, NULL))
		return;
}
