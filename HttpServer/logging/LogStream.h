#pragma once
#include <cassert>
#include <cstring>
#include <string>

const int minBufferSize = 4096;
const int maxBufferSize = 4096 * 1024;

//�û��Զ��建������С
template<int SIZE>
class FixedBuffer {
public:
	FixedBuffer() :
		cur_(data_)
	{}
	~FixedBuffer() {}
	//��ֹ��������
	FixedBuffer(const FixedBuffer& rhs) = delete;
	FixedBuffer& operator=(const FixedBuffer& rhs) = delete;

	const char* data() const { return data_; }
	size_t length() const { return static_cast<int>(cur_ - data_); }
	size_t avail() const { return static_cast<int>(end() - cur_); }
	char* current() { return cur_; }
	void add(size_t len) { cur_ += len; }
	void bzero() { memset(data_, 0, sizeof(data_)); }
	void reset() { cur_ = data_; }

	void append(const char* buf, size_t len) {
		if (avail() > len) {
			memcpy(cur_, buf, len);
			add(len);
		}
	}
private:
	const char* end() const { return data_ + sizeof(data_); }
	char data_[SIZE];
	char* cur_;
};

class LogStream {
public:
	using Buffer = FixedBuffer<minBufferSize>;
	LogStream() = default;		//����Ĭ�Ϲ��캯��
	LogStream(const LogStream& rhs) = delete;
	LogStream& operator=(const LogStream& rhs) = delete;

	LogStream& operator<<(bool value) {
		buffer_.append(value ? "1" : "0", 1);
		return *this;
	}
	LogStream& operator<<(short value);
	LogStream& operator<<(unsigned short value);
	LogStream& operator<<(int value);
	LogStream& operator<<(unsigned int value);
	LogStream& operator<<(long value);
	LogStream& operator<<(unsigned long value);
	LogStream& operator<<(long long value);
	LogStream& operator<<(unsigned long long value);

	LogStream& operator<<(float value);
	LogStream& operator<<(double value);
	LogStream& operator<<(long double value);

	LogStream& operator<<(char value);
	LogStream& operator<<(const char* str);
	LogStream& operator<<(const std::string& str);

	void append(const char* data, size_t len) { buffer_.append(data, len); }
	const Buffer& buffer() const { return buffer_; }
	void reset() { buffer_.reset(); }
private:
	template<typename T>
	void formatInteger(T);
	void formatInteger(long long value);
	void formatInteger(unsigned long long value);

	Buffer buffer_;
	const int MaxNumericSize = 32;
	//C++11��long longΪ64λ
};