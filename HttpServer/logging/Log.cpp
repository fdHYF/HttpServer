#include "Log.h"
#include <pthread.h>
#include <ctime>
#include <sys/time.h>

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static LogWriting* LogWriter;

std::string Logger::log_filename_ = "./WangTian.log";

void once_init() {
	LogWriter = new LogWriting(Logger::LogFilename());
	LogWriter->start();
}

void output(const char* msg, size_t len) {
	pthread_once(&once_control, once_init);
	LogWriter->append(msg, len);
}

void Logger::formatTime() {
	struct timeval tv;
	time_t time;
	char str_t[26] = { 0 };
	gettimeofday(&tv, NULL);
	time = tv.tv_sec;
	struct tm* p_time = localtime(&time);
	strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
	stream_ << str_t;
}

Logger::Logger(const char* filename, const int line) :
	stream_(),
	filename_(filename),
	line_(line)
{
	formatTime();
}

Logger::~Logger() {
	stream_ << "--" << filename_ << ':' << line_ << '\n';
	const LogStream::Buffer& buf(stream().buffer());
	output(buf.data(), buf.length());
}
