#ifndef __MSGQUEUEH__
#define __MSGQUEUEH__
#include <mutex>
#include "MyQueue.h"
#include <queue>

template<typename T>
class SafeQueue:public IQueue<T> {
private:
	std::mutex _mu;
	MyQueue<T> _queue;
	QueueNode<T>* _it;
public:
	SafeQueue(int poolSize):_it(nullptr), _queue(poolSize){

	}
	~SafeQueue() {

	}
	void push(T t) {
		_mu.lock();
		_queue.push(t);
		_mu.unlock();
	}
	T pop() {
		_mu.lock();
		T t = _queue.pop();
		_mu.unlock();
		return t;
	}
	T front() {
		return _queue.front();
	}
	bool hasData() {
		return _queue.size() > 0;
	}
	void dump() {
		MyQueue_Dump(&_queue);
	}
	void begin() {
		_it = _queue.getHead();
	}
	bool end() {
		return _it == nullptr;
	}
	bool next(){
		if (!_it) {
			return false;
		}
		_it = _it->_pNext;
		return true;
	}
	T getVal() {
		if (!_it) {
			return T();
		}
		return _it->_data;
	}
	void clear() {
		_queue.clear();
	}
	void lock() {
		_mu.lock();
	}
	void unlock() {
		_mu.unlock();
	}
	void manualSize(int size) {
		_queue.manualSize(size);
	}
	size_t size() {
		return _queue.size();
	}
	void GetMemInfo(MemInfo* memInfo) {
		_mu.lock();
		_queue.GetMemInfo(memInfo);
		_mu.unlock();
	}
};
#endif