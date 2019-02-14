#pragma once
#include <queue>
#include <memory>
#include <unistd.h>
#include <vector>
#include "lib/Mutex.h"
class HttpData;

class TimerNode {
public:
	TimerNode(const std::shared_ptr<HttpData> client_data, 
		const size_t& expiredtime);
	~TimeNode() {}
	TimeNode(const TimeNode& rhs);
	TimeNode& operator=(const TimeNode& rhs);

	void update(size_t updatetime);
	void setDeleted() { deleted_ = true; }
	bool isValid();
	bool isDeleted() { return deleted_ == true; }
private:
	bool deleted_;
	std::shared_ptr<HttpData> client_data_;
	size_t expiredtime_;
};

//�º������Ƚ϶�ʱ���ڵ�ĳ�ʱʱ��
struct TimeNodeComp {
	bool operator() (const std::shared_ptr<TimerNode>& a,
		const std::shared_ptr<TimerNode>& b) {
		return a->expiredtime_ > b->expiredtime_;
	}
};

class Timer {
public:
	using sp_node = std::shared_ptr<TimeNode>;

	Timer();
	~Timer();
	Timer(const Timer& rhs) = delete;
	Timer& operator=(const Timer& rhs) = delete;

	//����µ�ʱ��ڵ�
	void add_node(std::shared_ptr<HttpData> client_data, size_t timeout);
	//���ö���ɾ���ķ�ʽ����ʱ�ڵ�
	void handle_expired();

private:
	std::priority_queue<sp_node, std::vector<sp_node>, TimeNodeComp> TimerQueue;
};
