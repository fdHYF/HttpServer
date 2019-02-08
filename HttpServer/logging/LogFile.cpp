#include "LogFile.h"
#include "../lib/Mutex.h"



LogFile::LogFile(const std::string& log_filename, const size_t flush_everyn) :
	ini_filename_(log_filename),
	log_filename_(ini_filename_ + std::to_string(log_fileorder_)),
	log_fileorder_(0),
	fp_(fopen(log_filename_.c_str(), "ae")),
	mutex_(),
	flush_everyn_(flush_everyn),
	count_(0),
	log_size_(0)
{
	//��ȡ�ļ�������ʱ��
	struct timeval now;
	gettimeofday(&now, nullptr);
	log_ctime_.tv_sec = now.tv_sec;
	log_ctime_.tv_usec = now.tv_usec;
	//�Զ��建����
	setbuffer(fp_, buffer_, sizeof(buffer_));
}

void LogFile::flush() {
	MutexLockGuard lock(mutex_);
	fflush(fp_);
}

size_t LogFile::write(const char* logline, size_t len) {
	//C���Ե�IO�������̰߳�ȫ�ģ�Ϊ�������̰߳�ȫ�������ڲ�������
	//�ڱ�ֻ֤��һ���̷߳��ʵ�����£�ʹ���������������ִ���ٶ�
	return fwrite_unlocked(logline, 1, len, fp_);
}

void LogFile::appendn(const char* logline, size_t len) {
	size_t nwrite = write(logline, len);
	size_t nremain = len - nwrite;
	while (nremain > 0) {
		size_t tmp = write(logline + nwrite, nremain);
		if (tmp == 0) {
			int err = ferror(fp_);
			if (err)
				fprintf(stderr, "LogFile::append() failed!\n");
			break;
		}
		nwrite += tmp;
		nremain -= tmp;
	}
	log_size_ += nwrite;
}

void LogFile::append(const char* logline, size_t len) {
	MutexLockGuard lock(mutex_);
	appendn(logline, len);
	++count_;
	if (count_ > flush_everyn_) {
		flush();
		count_ = 0;
	}
}

void LogFile::rollfile() {
	struct timeval now;
	gettimeofday(&now, nullptr);
	int days = static_cast<int>((now.tv_sec - log_ctime_.tv_sec) / (24 * 60 * 60));
	if (log_size_ >= 1024 * 1024 * 1024 || days >= 3) {
		fclose(fp_);
		++log_fileorder_;
		log_ctime_.tv_sec = now.tv_sec;
		log_ctime_.tv_usec = now.tv_usec;
		log_size_ = 0;
		log_filename_ = ini_filename_ + std::to_string(log_fileorder_);
		fp_ = fopen(log_filename_.c_str(), "ae");
	}
}