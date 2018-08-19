#ifndef __MYQUEUEH__
#define __MYQUEUEH__
#include "MemoryPool.h"
#include "QueueMsg.h"
#include "IQueue.h"

template<typename T>
class MyQueue : public IQueue<T> {
private:
	size_t _size;
	QueueNode<T> *head;
	QueueNode<T> *tail;
	MemoryPool memoryPool;
public:
	MyQueue(int poolSize):
		head(nullptr),
		tail(nullptr),
		memoryPool(sizeof(QueueNode<T>), poolSize, poolSize),
		_size(0){

	}
	virtual ~MyQueue() {

	}
	void manualSize(int size) {
		_size += size;
	}
	void clear() {
		_size = 0;
		head = tail = nullptr;
		memoryPool.FreeAll();
	}
	QueueNode<T>* getHead() {
		return head;
	}
	void push(T newData) {
		_size++;
		QueueNode<T>* newNode = (QueueNode<T>*)memoryPool.Alloc();
		newNode->_data = newData;

		if (head != nullptr) {
			tail->_pNext = newNode;
			newNode->_pNext = nullptr;
			tail = newNode;
		}
		else {
			head = newNode;
			head->_pNext = nullptr;
			tail = head;
		}
	}
	T front() {
		if (head == nullptr) {
			return T();
		}
		else {
			return head->_data;
		}
	}
	T pop() {
		if (head == nullptr)
			return T();
		_size--;
		QueueNode<T>* oldHead = head;
		head = head->_pNext;
		if (head == nullptr) {
			tail = nullptr;
		}
		T data = oldHead->_data;
		memoryPool.Free((void*)oldHead);
		return data;
	}
	size_t size() {
		return _size;
	}
	bool hasData() {
		return _size > 0;
	}
	void dump() {
		LOG("MyQueueMyQueueMyQueueMyQueueMyQueueMyQueue\n");
		LOG("_size:%d\n",_size);
		QueueNode<T>* curNode = head;
		while(curNode) {
			LOG("node addr:%x\n",curNode);
			LOG("data val:%x\n", curNode->_data);
			LOG("------------\n");
			curNode = curNode->_pNext;
		}
		LOG("***MyQueueMyQueueMyQueueMyQueueMyQueueMyQueue***\n");
		memoryPool.dump();
	}
	void GetMemInfo(MemInfo* memInfo) {
		memoryPool.GetInfo(memInfo);
	}
};
#endif