#pragma once
#include <cstdio>
#include <string>
#include <sys/time.h>
#include <time.h>
#include "../lib/Mutex.h"

class LogFile {
public:
	LogFile(const std::string& file_name, const size_t flush_everyn_ = 5);
	~LogFile() { fclose(fp_); }

	void append(const char* logline, size_t len);
	void flush();
	std::string filename() { return log_filename_; }
	size_t size() const { return log_size_; }
	FILE* fp() { return fp_; }
	void rollfile();
private:
	std::string ini_filename_;
	std::string log_filename_;
	size_t log_fileorder_;
	FILE* fp_;					//file ptr
	MutexLock mutex_;

	const size_t flush_everyn_;
	size_t count_;
	size_t log_size_;				//log file size
	struct timeval log_ctime_;		//log file create time

	size_t write(const char* logline, size_t len);
	void appendn(const char* logline, size_t len);
	char buffer_[64 * 1024];
};