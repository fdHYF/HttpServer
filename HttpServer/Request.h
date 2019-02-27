#pragma once
#include <map>
#include <memory>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include "Timer.h"

#include "Channel.h"
const int FILENAME_LIN = 200;		//最大文件名长度
const int READ_BUFFER_SIZE = 1024;
const int WRITE_BUFFER_SIZE = 2048;

//HTTP方法仅仅支持GET
enum METHOD { GET = 0, POST, HEAD, TRACE, PUT, DELETE, CONNECT, PATCH, OPTIONS };
//主状态机状态：分析请求行、分析头部字段
enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER };
//从状态机的状态，即行的读取状态：读取到一个完整的行、行出错和行数据尚不完整
enum LINE_STATUS { LINE_OK, LINE_BAD, LINE_OPEN };
enum HTTP_CODE {
	NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST,
	INTERNAL_ERROR, CLOSED_CONNECTION
};
/**
 * NO_REQUEST:请求不完整
 * GET_REQUEST:得到一个完整的客户请求
 * BAD_REQUEST:请求语法错误
 * FORBIDDEN_REQUEST:对资源的访问权限不足
 * INTERNAL_ERROR:服务器内部错误
 * CLOSED_CONNECTION:客户关闭连接
 *
 */
enum HTTP_VERSION { HTTP_10 = 1, HTTP_11 };

class MimeType {
public:
	MimeType() = default;
	static std::string getMime(const std::string& str);
private:
	static pthread_once_t once_control;
	static void init();
	static std::map<std::string, std::string> mime;
};

class HttpData {
public:
	HttpData(int connfd);
	~HttpData() { close(fd_); }

	std::shared_ptr<TimerNode> get_node() { return timer_; }
	//void link_timer(std::shared_ptr<TimerNode> timer) { timer_ = timer; }
	int fd() { return fd_; }

	void handleRead();
	void handleWrite();
	void handleError(int fd, int err_num, std::string msg);
private:
	int fd_;
	std::string ReadBuffer;
	std::string WriteBuffer;

	METHOD method_;
	HTTP_VERSION version_;
	std::string filename_;
	std::string path_;
	std::map<std::string, std::string> headers_;

	bool keepAlive_;
	std::shared_ptr<TimerNode> timer_;
	int now_index_;			//读缓冲区当前正在分析的字节
	int read_index_;		//读缓冲区中客户数据的下一个字节

	//从状态机，解析一行
	LINE_STATUS parse_line(std::string& str);
	//分析请求行
	HTTP_CODE parse_request_line(std::string& request_line);
	HTTP_CODE parse_headers(std::string& header);
	HTTP_CODE parse_content(std::string& content);
	bool do_request();

	ssize_t readFromFd();
	ssize_t writeToFd();
};
