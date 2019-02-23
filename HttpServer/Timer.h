#pragma once
#include <queue>
#include <memory>
#include <unistd.h>
#include <vector>
#include "lib/Mutex.h"
class HttpData;

class TimerNode {
public:
	TimerNode(const std::shared_ptr<HttpData>& client_data, 
		const size_t& expiredtime);
	~TimerNode() {}
	TimerNode(const TimerNode& rhs);
	TimerNode& operator=(const TimerNode& rhs);

	void update(size_t updatetime);
	void setDeleted() { deleted_ = true; }
	bool isValid();
	bool isDeleted() { return deleted_ == true; }
	size_t get_expired() const { return expiredtime_; }
private:
	bool deleted_;
	std::shared_ptr<HttpData> client_data_;
	size_t expiredtime_;
};

//仿函数，比较定时器节点的超时时间
struct TimeNodeComp {
	bool operator() (const std::shared_ptr<TimerNode>& a,
		const std::shared_ptr<TimerNode>& b) {
		return a->get_expired() > b->get_expired();
	}
};

class Timer {
public:
	using sp_node = std::shared_ptr<TimerNode>;

	Timer() = default;
	~Timer() = default;
	//Timer(const Timer& rhs) = delete;
	//Timer& operator=(const Timer& rhs) = delete;

	//添加新的时间节点
	void add_node(std::shared_ptr<HttpData> client_data, const size_t timeout);
	//采用惰性删除的方式处理超时节点
	void handle_expired();

private:
	std::priority_queue<sp_node, std::vector<sp_node>, TimeNodeComp> TimerQueue;
};
