#include "LogWrite.h"
#include <cassert>
LogWriting::LogWriting(const std::string& filename, const size_t& flush_frequency) :
	filename_(filename),
	flush_frequency_(flush_frequency),
	running_(false),
	thread_(std::bind(&LogWriting::logFunc, this), "Logging"),
	mutex_(),
	cond_(mutex_),
	current_buffer_(new Buffer),
	next_buffer_(new Buffer),
	buffers_()
{
	assert(!filename.empty());
	current_buffer_->bzero();
	next_buffer_->bzero();
}

//前端调用append追加日志到current_buffer_
void LogWriting::append(const char* logline, const size_t len) {
	MutexLockGuard lock(mutex_);
	if (current_buffer_->avail() >= len)
		current_buffer_->append(logline, len);
	else {
		buffers_.push_back(current_buffer_);
		current_buffer_.reset();
		if (next_buffer_)
			current_buffer_ = std::move(next_buffer_);
		else
			current_buffer_.reset(new Buffer);
		current_buffer_->append(logline, len);
	}
}


void LogWriting::logFunc() {
	assert(running_ == true);
	LogFile output(filename_);
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector BufferToWrite;
	while (running_) {
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		{
			MutexLockGuard lock(mutex_);
			if (buffers_.empty()) {
				cond_.timewait(flush_frequency_);
			}
			buffers_.push_back(current_buffer_);
			current_buffer_.reset();

			current_buffer_ = std::move(newBuffer1);
			BufferToWrite.swap(buffers_);
			if (!next_buffer_)
				next_buffer_ = std::move(newBuffer2);
		}
		assert(!BufferToWrite.empty());
		for (size_t i = 0; i < BufferToWrite.size(); ++i) {
			output.append(BufferToWrite[i]->data(), BufferToWrite[i]->length());
		}
		if (BufferToWrite.size() > 2)
			BufferToWrite.resize(2);
		if (!newBuffer1) {
			assert(!BufferToWrite.empty());
			newBuffer1 = BufferToWrite.back();
			BufferToWrite.pop_back();
			newBuffer1->reset();
		}
		if (!newBuffer2) {
			assert(!BufferToWrite.empty());
			newBuffer2 = BufferToWrite.back();
			BufferToWrite.pop_back();
			newBuffer2->reset();
		}
		BufferToWrite.clear();
		output.flush();
		output.rollfile();
	}
	output.flush();
}