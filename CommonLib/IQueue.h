#ifndef __IQUEUEH__
#define __IQUEUEH__
#include "IMemPool.h"

template<typename T>
struct QueueNode {
	T _data;
	QueueNode* _pNext;
};

template<typename T>
class IQueue {
public:
	virtual ~IQueue() {}
	virtual bool hasData() = 0;
	virtual void manualSize(int size) = 0;
	virtual void clear() = 0;
	virtual void push(T newData) = 0;
	virtual T front() = 0;
	virtual T pop() = 0;
	virtual size_t size() = 0;
	virtual void dump() = 0;
	virtual void GetMemInfo(MemInfo* memInfo) = 0;
};
#endif