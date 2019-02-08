#include "LogStream.h"
#include <algorithm>

const char array[] = "9876543210123456789";
const char* zero = array + 9;

template<typename T>
size_t convert(char buf[], T value) {
	T i = value;
	char* p = buf;
	do {
		int tmp = static_cast<int>(i % 10);
		i /= 10;
		*p++ = zero[tmp];
	} while (i != 0);
	if (value < 0)
		*p++ = '-';
	*p = '\0';
	std::reverse(buf, p);
	return p - buf;
}

template<typename T>
void LogStream::formatInteger(T value) {
	if (buffer_.avail() >= static_cast<size_t>(MaxNumericSize)) {
		size_t len = convert(buffer_.current(), value);
		buffer_.add(len);
	}
}

void LogStream::formatInteger(long long value) {
	if (buffer_.avail() >= 64) {
		size_t len = convert(buffer_.current(), value);
		buffer_.add(len);
	}
}

void LogStream::formatInteger(unsigned long long value) {
	if (buffer_.avail() >= 64) {
		size_t len = convert(buffer_.current(), value);
		buffer_.add(len);
	}
}

LogStream& LogStream::operator<<(short value) {
	*this << static_cast<int>(value);
	return *this;
}

LogStream& LogStream::operator<<(unsigned short value) {
	*this << static_cast<unsigned int>(value);
	return *this;
}

LogStream& LogStream::operator<<(int value) {
	formatInteger(value);
	return *this;
}

LogStream& LogStream::operator<<(unsigned value) {
	formatInteger(value);
	return *this;
}

LogStream& LogStream::operator<<(long value) {
	formatInteger(value);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long value) {
	formatInteger(value);
	return *this;
}

LogStream& LogStream::operator<<(long long value) {
	formatInteger(value);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long long value) {
	formatInteger(value);
	return *this;
}

LogStream& LogStream::operator<<(float value) {
	*this << static_cast<double>(value);
	return *this;
}

LogStream& LogStream::operator<<(double value) {
	if (buffer_.avail() >= 64) {
		int len = snprintf(buffer_.current(), 64, "%.12g", value);
		buffer_.add(len);
	}
	return *this;
}

LogStream& LogStream::operator<<(long double value) {
	if (buffer_.avail() >= 128) {
		int len = snprintf(buffer_.current(), 128, "%.12Lg", value);
		buffer_.add(len);
	}
	return *this;
}

LogStream& LogStream::operator<<(char value) {
	if (buffer_.avail() >= 1)
		buffer_.append(&value, 1);
	return *this;
}

LogStream& LogStream::operator<<(const char* str) {
	if (str == nullptr) {
		buffer_.append("nullptr", 7);
	}
	else {
		buffer_.append(str, strlen(str));
	}
	return *this;
}

LogStream& LogStream::operator<<(const std::string& str) {
	buffer_.append(str.c_str(), str.size());
	return *this;
}