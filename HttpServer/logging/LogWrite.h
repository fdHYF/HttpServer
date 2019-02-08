#pragma once
#include "LogFile.h"
#include "LogStream.h"
#include "../lib/Mutex.h"
#include "../lib/Condition.h"
#include "../lib/Thread.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class LogWriting {
public:
	using Buffer = FixedBuffer<maxBufferSize>;
	using BufferPtr = std::shared_ptr<Buffer>;
	using BufferVector = std::vector<BufferPtr>;

	LogWriting(const std::string& filename, const size_t& flush_frequency = 3);
	~LogWriting() {
		if (running_)
			stop();
	}

	void append(const char* logline, size_t len);

	void start() {
		running_ = true;
		thread_.start();

	}

	void stop() {
		running_ = false;
		thread_.join();
	}
private:
	void logFunc();
private:
	std::string filename_;
	size_t flush_frequency_;
	bool running_;
	Thread thread_;
	MutexLock mutex_;
	Condition cond_;
	BufferPtr current_buffer_;
	BufferPtr next_buffer_;
	BufferVector buffers_;
};

