#include "Request.h"
#include <string>
#include <string.h>
#include <functional>
#include <pthread.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "logging/Log.h"


//静态成员初始化
pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::map<std::string, std::string> MimeType::mime;

const std::string ok_200_title = "OK";
const std::string error_400_title = "Bad Request";
const std::string error_400_content = "Your request has bad syntax or is inherently impossible to satisfy";
const std::string error_403_title = "Forbidden";
const std::string error_403_content = "You don't have permission to get file form this serve";
const std::string error_404_title = "Not Found";
const std::string error_404_content = "The requested file was not found on this serve";
const std::string error_500_title = "Internal Error";
const std::string error_500_content = "There was some problem serving the requested file";
const std::string error_505_title = "HTTP Version Not Supported";
const std::string error_505_content = "The server not supported the used HTTP version";

void MimeType::init() {
	mime[".html"] = "text/html";
	mime[".avi"] = "video/x-msvideo";
	mime[".bmp"] = "image/bmp";
	mime[".c"] = "text/plain";
	mime[".doc"] = "application/msword";
	mime[".gif"] = "image/gif";
	mime[".gz"] = "application/x-gzip";
	mime[".htm"] = "text/html";
	mime[".ico"] = "image/x-icon";
	mime[".jpg"] = "image/jpeg";
	mime[".png"] = "image/png";
	mime[".txt"] = "text/plain";
	mime[".mp3"] = "audio/mp3";
	mime[".ppt"] = "application / vnd.ms - powerpoint";
	mime[".pdf"] = "application/pdf";
	mime["default"] = "text/html";
}


std::string MimeType::getMime(const std::string& str) {
	pthread_once(&once_control, init);
	if (mime.find(str) == mime.end())
		return mime["default"];
	else
		return mime[str];
}

HttpData::HttpData(int connfd) :
	fd_(connfd),
	method_(GET),
	version_(HTTP_11),
	keepAlive_(false),
	now_index_(0),
	read_index_(0)
{	
	LOG << "Init one HttpData";
}

//从状态机，用于解析出一行
LINE_STATUS HttpData::parse_line(std::string& str) {
	char tmp;
	int start = now_index_;
	for (; now_index_ < read_index_; ++now_index_) {
		tmp = ReadBuffer[now_index_];
		//如果当前字节是'\r',说明有可能读取到一个完整的行
		if (tmp == '\r') {
			if (now_index_ + 1 == read_index_)
				return LINE_OPEN;
			else if (ReadBuffer[now_index_ + 1] == '\n') {
				str = ReadBuffer.substr(start, now_index_);
				ReadBuffer[now_index_++] = '\0';
				ReadBuffer[now_index_++] = '\0';
				return LINE_OK;
			}
		}
		else if (tmp == '\n') {
			if (ReadBuffer[now_index_ - 1] == '\r') {
				str = ReadBuffer.substr(start, now_index_ - 1);
				ReadBuffer[now_index_ - 1] = '\0';
				ReadBuffer[now_index_++] = '\0';
				return LINE_OK;
			}
			else
				return LINE_BAD;
		}
	}
	ReadBuffer.clear();
	now_index_ = read_index_ = 0;
	return LINE_OPEN;
}


//分析HTTP报文请求行
HTTP_CODE HttpData::parse_request_line(std::string& request_line) {
	if (request_line.empty())
		return NO_REQUEST;
	//请求方法解析
	size_t pos = request_line.find(" ");
	std::string strMethod = request_line.substr(0, pos);
	if (strMethod.compare("GET") == 0)
		method_ = GET;
	else
		return BAD_REQUEST;
	//路径名以及文件名解析
	size_t index_left = request_line.find("/", pos);
	pos = index_left;
	pos = request_line.find(" ", pos);
	path_ = request_line.substr(index_left, pos);

	//协议版本
	if (request_line.find("HTTP/1.1", pos) > 0)
		version_ = HTTP_11;
	else
		return BAD_REQUEST;
	//得到请求行的完整解析
	return GET_REQUEST;
}

//分析头部字段
HTTP_CODE HttpData::parse_headers(std::string& header) {
	std::string key;
	std::string value;
	if (header[0] == '\0') {
		if (method_ == HEAD)
			return GET_REQUEST;
		else
			return NO_REQUEST;
	}
	else {
		size_t pos = header.find(":");
		key = header.substr(0, pos);
		pos += 2;
		value = header.substr(pos, header.size());
		headers_[key] = value;
	}
	return GET_REQUEST;
}

//因为此处只支持GET方法，GET方法对应的实体体为空，故没有实体体的解析
HTTP_CODE HttpData::parse_content(std::string& content) {
	return GET_REQUEST;
}

//返回读取字节总数
ssize_t HttpData::readFromFd() {
	ssize_t nread = 0;
	ssize_t readsum = 0;
	while (true) {
		char buffer[READ_BUFFER_SIZE];
		if ((nread = read(fd_, buffer, READ_BUFFER_SIZE)) < 0) {
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN)
				return readsum;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		readsum += nread;
		read_index_ += (int)nread;
		ReadBuffer += std::string(buffer, buffer + nread);
	}
	return readsum;
}

void HttpData::handleRead() {
	LINE_STATUS line_state = LINE_OK;
	CHECK_STATE check_state = CHECK_STATE_REQUESTLINE;
	HTTP_CODE http_code = GET_REQUEST;
	int readsum = (int)readFromFd();
	if (readsum < 0) {
		handleError(fd_, 400, "Bad Request");
	}
	while (true) {
		std::string str;
		if ((line_state = parse_line(str)) == LINE_BAD) {
			//HTTP请求语法错误
			handleError(fd_, 400, "Bad Request");
		}
		else if (line_state == LINE_OK) {
			//得到一个完整的行
			switch (check_state)
			{
			case CHECK_STATE_REQUESTLINE:
			{
				if ((http_code = parse_request_line(str)) == BAD_REQUEST)
					handleError(fd_, 400, "Bad Request");
				else {

				}
				check_state = CHECK_STATE_HEADER;
				break;
			}
			case  CHECK_STATE_HEADER:
			{
				if ((http_code = parse_headers(str)) == BAD_REQUEST)
					handleError(fd_, 400, "Bad Request");
			}
			default:
				break;
			}
		}
		else if (line_state == LINE_OPEN && str.empty())
			break;
	}
	do_request();
}

ssize_t HttpData::writeToFd() {
	ssize_t nwrite = 0;
	ssize_t writesum = 0;
	const char* tmp = WriteBuffer.c_str();
	WriteBuffer.clear();
	while (true) {
		size_t len = strlen(tmp);
		if ((nwrite = write(fd_, tmp, len) < 0)) {
			if (errno == EINTR)
				continue;
			else if (errno == EAGAIN)
				return writesum;
			else
				return -1;
		}
		else if (nwrite == 0)
			break;
		writesum += nwrite;
		tmp += nwrite;
	}
	return writesum;
}

void HttpData::handleWrite() {
	if (writeToFd() < 0) {
		LOG << "writeToFd error";
	}
	
}

bool HttpData::do_request() {
	std::string header;
	header += "HTTP/1.1 200 OK\r\n";
	if (headers_.find("Connection") != headers_.end() && (headers_["Connection"] == "keep-Alive" ||
		headers_["Connection"] == "keep-alive")) {
		keepAlive_ = true;
		header += "Connection: keep-Alive\r\n";
	}
	else {
		keepAlive_ = false;
		header += "Connection: close\r\n";
	}
	//确定文件类型
	size_t pos = path_.find(".");
	std::string file_type;
	if (pos < 0)
		file_type = MimeType::getMime("default");
	else
		file_type = MimeType::getMime(path_.substr(pos));

	struct stat buf;
	if (stat(path_.c_str(), &buf) < 0) {
		header.clear();
		handleError(fd_, 404, "Not Found");
		return false;
	}

	header += "Content-Type: " + file_type + "\r\n";
	header += "Content-Length: " + std::to_string(buf.st_size) + "\r\n";
	header += "Server: WangTian/1.0 (Ubuntu)\r\n";
	header += "\r\n";
	WriteBuffer += header;

	if (method_ == HEAD)
		return true;
	int file_fd = open(path_.c_str(), O_RDONLY, 0);
	if (file_fd < 0) {
		WriteBuffer.clear();
		handleError(fd_, 404, "Not Found");
		return false;
	}

	//使用存储映射IO读取文件
	void* address = mmap(nullptr, buf.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
	close(file_fd);
	if (address == MAP_FAILED) {
		//关闭映射存储区
		munmap(address, buf.st_size);
		WriteBuffer.clear();
		handleError(fd_, 404, "Not Found");
		return false;
	}
	char* file_addr = static_cast<char*>(address);
	WriteBuffer += std::string(file_addr, file_addr + buf.st_size);
	munmap(address, buf.st_size);
	return true;
}

void HttpData::handleError(int fd, int err_num, std::string msg) {
	//char buffer[WRITE_BUFFER_SIZE];
	//响应头缓冲和数据缓冲
	std::string header, body;
	body += "<html><title>哎呀~出错了</title>";
	body += "<body bgcolor=\"ffffff\">";
	body += std::to_string(err_num) + " " + msg;
	body += "<hr><em> WangTian Web Server</em>\n</body></html>";

	header += "HTTP/1.1 " + std::to_string(err_num) + " " + msg + "\r\n";
	header += "Content-Type: text/html\r\n";
	header += "Connection: close\r\n";
	header += "Server: WangTian\r\n";
	header += "Content-Length: " + std::to_string(body.size()) + "\r\n";
	header += "\r\n";

	WriteBuffer += header;
	writeToFd();
	WriteBuffer += body;
	writeToFd();
}
