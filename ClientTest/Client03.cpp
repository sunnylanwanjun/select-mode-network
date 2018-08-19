#include "Client03.h"
#include "Macro.h"
#include <thread>
#include <fstream>
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <mutex>
#include "Timestamp.h"
#include <atomic>

/*
即使主线程结束，初分离的线程依然能执行完毕
*/
void Client03::runThread1() {
	std::ofstream out("ThreadTest.txt");
	for (int i = 0; i<100; i++) {
		printf("hello thread1:%d\n", i);
		out << "hello thread1:" << i << std::endl;
	}
	out.close();
}

void Client03::runMain1() {
	std::thread t(&Client03::runThread1, this);
	t.detach();
	for (int i = 0; i<5; i++) {
		printf("hello main1:%d\n", i);
	}
}

void Client03::runThread2() {
	printf("hello thread2\n");
}

void Client03::runMain2() {
	//从源码可以看出一个线程，在析构前必须调用detach或者join，否则
	//在析构的时候，thread会调用joinable判断_Thr是否为空，若不为空，则
	//表示该thread没有被detach或者join，则抛出异常
	//那为何拷贝构造函数没有关系呢，被拷贝的对象也没有去调用detach或者join
	//为什么没事呢，因为里边调用了MoveThread方法，把被拷贝的对象的Thr直接赋
	//给了目标thread，把被拷贝的thread的Thr赋空，这样当析构被拷贝对象的时候
	//就不会抛出异常了
	std::thread t;
	t = std::thread(&Client03::runThread2, this);
	t.join();
	sleep(3000);
	printf("hello main2\n");
}

#define CALCTIMES 10000000
void Client03::runThread3() {
	for (int i = 0; i<CALCTIMES; i++) {
		mu.lock();
		sum += i;
		mu.unlock();
	}
}
void Client03::runThread33() {
	mu.lock();
	for (int i = 0; i<CALCTIMES; i++) {
		sum += i;
	}
	mu.unlock();
}
void Client03::runThread333() {
	for (int i = 0; i<CALCTIMES; i++) {
		sum += i;
	}
}
void Client03::runThread3333() {
	atomSum = 0;
	for (int i = 0; i<CALCTIMES; i++) {
		atomSum += i;
	}
}
void Client03::runMain3() {
	/*
	runThread3 的执行时间最长
	runThread33 runThread333 次之
	主线程中的执行时间最短
	因为3中使用了锁，33和333使用了线程，但，33貌似比333要快一些
	这是为什么呢？？？33使用了锁，333没有使用？？这其中有什么猫腻
	现在暂时还不知道,使用原子操作比锁要快上1秒多
	*/
	printf("calc times:%d\n", CALCTIMES);
	Timestamp t;

	std::thread th(&Client03::runThread3, this);
	th.join();
	printf("sum is:%llu,time is:%lld\n", sum, t.GetWS());

	t.Update();
	sum = 0;
	std::thread th33(&Client03::runThread33, this);
	th33.join();
	printf("sum is:%llu,time is:%lld\n", sum, t.GetWS());

	t.Update();
	sum = 0;
	std::thread th333(&Client03::runThread333, this);
	th333.join();
	printf("sum is:%llu,time is:%lld\n", sum, t.GetWS());

	t.Update();
	sum = 0;
	std::thread th3333(&Client03::runThread3333, this);
	th3333.join();
	sum = atomSum;
	printf("atomsum is:%llu,time is:%lld\n", sum, t.GetWS());

	t.Update();
	sum = 0;
	for (int i = 0; i<CALCTIMES; i++) {
		sum += i;
	}
	printf("sum is:%llu,time is:%lld\n", sum, t.GetWS());
}

Client03::Client03() {
	runMain3();
}

Client03::~Client03() {
}