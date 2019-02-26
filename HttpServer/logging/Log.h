#pragma once
#include <string>
#include <cstdio>
#include "LogStream.h"
#include "LogWrite.h"

class Logger {
public:
	Logger(const char* filename, const int line);
	~Logger();
	LogStream& stream() { return stream_; }
	static std::string LogFilename() { return log_filename_; }
private:
	static std::string log_filename_;
	void formatTime();
	LogStream stream_;
	const char* filename_;
	int line_;
};

#define LOG Logger(__FILE__, __LINE__).stream()