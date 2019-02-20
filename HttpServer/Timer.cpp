#include "Timer.h"
#include <sys/time.h>

TimerNode::TimerNode(const std::shared_ptr<HttpData>& client_data,
	const size_t& expired_time) :
	deleted_(false),
	client_data_(client_data)
{
	struct timeval now;
	gettimeofday(&now, nullptr);
	expiredtime_ = now.tv_usec / 1000 + now.tv_sec * 1000 + expired_time;
}

TimerNode::TimerNode(const TimerNode& rhs) {
	deleted_ = rhs.deleted_;
	client_data_ = rhs.client_data_;
	expiredtime_ = rhs.expiredtime_;
}

TimerNode& TimerNode::operator=(const TimerNode& rhs) {
	deleted_ = rhs.deleted_;
	client_data_ = rhs.client_data_;
	expiredtime_ = rhs.expiredtime_;
	return *this;
}

//更新超时时间
void TimerNode::update(size_t updatetime) {
	struct timeval now;
	gettimeofday(&now, nullptr);
	expiredtime_ = now.tv_usec / 1000 + now.tv_sec * 1000 + updatetime;
}

bool TimerNode::isValid() {
	struct timeval now;
	gettimeofday(&now, nullptr);
	size_t tmp = now.tv_usec / 1000 + now.tv_sec * 1000;
	if (tmp < expiredtime_)
		return true;
	else {
		setDeleted();
		return false;
	}
}

void Timer::add_node(std::shared_ptr<HttpData> client_data, size_t timeout){
	sp_node new_node(new TimerNode(client_data, timeout));
	TimerQueue.push(new_node);
}

void Timer::handle_expired() {
	while (!TimerQueue.empty()) {
		auto node = TimerQueue.top();
		if (node->isDeleted())
			TimerQueue.pop();
		else if (node->isValid() == false)
			TimerQueue.pop();
		else
			break;
	}
}