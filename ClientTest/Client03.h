#ifndef __CLIENT03H__
#define __CLIENT03H__
#include <mutex>
#include <atomic>
class Client03 {
	/*
	即使主线程结束，初分离的线程依然能执行完毕
	*/
	void runThread1();
	void runMain1();
	void runThread2();
	void runMain2();
	unsigned long long sum = 0;
	std::mutex mu;
	std::atomic<long long> atomSum;
	void runThread3();
	void runThread33();
	void runThread333();
	void runThread3333();
	void runMain3();
public:
	Client03();
	~Client03();
};
#endif