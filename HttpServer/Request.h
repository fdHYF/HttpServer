#pragma once
#include <map>
#include <memory>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include "Timer.h"

#include "Channel.h"
const int FILENAME_LIN = 200;		//����ļ�������
const int READ_BUFFER_SIZE = 1024;
const int WRITE_BUFFER_SIZE = 2048;

//HTTP��������֧��GET
enum METHOD { GET = 0, POST, HEAD, TRACE, PUT, DELETE, CONNECT, PATCH, OPTIONS };
//��״̬��״̬�����������С�����ͷ���ֶ�
enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER };
//��״̬����״̬�����еĶ�ȡ״̬����ȡ��һ���������С��г�����������в�����
enum LINE_STATUS { LINE_OK, LINE_BAD, LINE_OPEN };
enum HTTP_CODE {
	NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST,
	INTERNAL_ERROR, CLOSED_CONNECTION
};
/**
 * NO_REQUEST:��������
 * GET_REQUEST:�õ�һ�������Ŀͻ�����
 * BAD_REQUEST:�����﷨����
 * FORBIDDEN_REQUEST:����Դ�ķ���Ȩ�޲���
 * INTERNAL_ERROR:�������ڲ�����
 * CLOSED_CONNECTION:�ͻ��ر�����
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
	int now_index_;			//����������ǰ���ڷ������ֽ�
	int read_index_;		//���������пͻ����ݵ���һ���ֽ�

	//��״̬��������һ��
	LINE_STATUS parse_line(std::string& str);
	//����������
	HTTP_CODE parse_request_line(std::string& request_line);
	HTTP_CODE parse_headers(std::string& header);
	HTTP_CODE parse_content(std::string& content);
	bool do_request();

	ssize_t readFromFd();
	ssize_t writeToFd();
};
