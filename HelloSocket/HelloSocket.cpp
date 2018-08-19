// HelloSocket.cpp : Defines the entry point for the console application.
//

#include "SocketHeader.h"
#include <stdio.h>
#include <string.h>
#include "IOBuffer.h"
#include "Msg.h"
#include "Utils.h"
#include "Macro.h"
#include "MemoryPool.h"
#include "SafeQueue.h"
#include "VarMemPool.h"
#include "ConfigHelp.h"
#include <fstream>

struct tt {
	int a;
	int b;
};

struct tt2 {
	int a;
	int b;
	tt2(int aa, int bb) {}
};

void test() {
	tt t = {};
	LOG("%d,%d\n", t.a, t.b);
	tt t1 = { 1,2 };
	LOG("%d,%d\n", t1.a, t1.b);
	//g++不支持非pod类型采用列表初始化
	//tt2 t2 = {3,3};
	tt2 t2(3, 3);
	LOG("%d,%d\n", t2.a, t2.b);
	char msgBuf[] = "hello,I'm Server";
	LOG("strlen is:%d\n", (int)strlen(msgBuf));
	LOG("sizeof is:%lu\n", sizeof(msgBuf));
	/*
	0,0
	1,2
	-858993460,-858993460
	strlen is:16
	sizeof is:17
	大括号初始化方法，对于pod类型来说比较直观，就是挨个赋值，但对于
	有构造函数的结构体来说，会优先查找符合参数的构造函数，如果没有，才会
	挨个赋值
	*/
}

class A {
public:
	char test[10];
	A() {
		LOG("test size is:%d\n", sizeof(test));
	}
};

struct TestMemPool {
	int a;
	int b;
	char c;
};

struct TestSize0 {
	short s0;
};

int main()
{
	/*
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	WSACleanup();
	*/
	int a = 1;
	int b = 2;
	int c = 3;
	char str[] = {'1','2','3'};
	int d = 4;
	LOG("str len is:%d\n", strlen(str));
#ifdef _WIN32
	LOG("this is win32\n");
#endif
#ifdef __linux__
	LOG("this is linux\n");
#endif
#ifdef __unix__
	LOG("this is unix\n");
#endif
#ifdef __APPLE__
	LOG("this is apple\n");
#endif

#ifdef _MSC_VER
	LOG("this is compile by vs\n");
#endif
/*
用g++编译，下面两个都会打印
this is compile by g++
this is compile by clang*/
#ifdef __GNUC__
	LOG("this is compile by g++\n");
#endif
#ifdef __clang__
	LOG("this is compile by clang\n");
#endif
	//无论指针被强转成了什么类型，编译器都知道要回收多少内存
	//因为在所在堆空间的前面几个字节里，保存了本次new操作所分配的
	//空间的大小
	int size = sizeof(MsgLoginReq);
	char* buf = new char[size];
	memset(buf, 0, size);
	MsgHead* msg = (MsgHead*)buf;
	delete msg;
	LOG("delete finished\n");

	A aa;
	/*
	MemoryPool p(sizeof(TestMemPool),3,2);
	MemoryPool_Dump(&p);
	TestMemPool* tArr[7];
	for(int i=0;i<7;i++){
		LOG("AAAAAAAAAAAAAAAAAAAAAA:%d\n",i+1);
		TestMemPool* t1 = (TestMemPool*)p.Alloc();
		t1->a = 100;
		t1->b = 200;
		t1->c = 'a';
		tArr[i] = t1;
		MemoryPool_Dump(&p);
	}
	for (int i = 0; i < 7; i++) {
		LOG("FFFFFFFFFFFFFFFFFFFFF:%d\n", i+1);
		p.Free(tArr[i]);
		MemoryPool_Dump(&p);
	}
	for (int i = 0; i<7; i++) {
		LOG("aaaaaaaaaaaaaaaaaaaaa:%d\n", i + 1);
		TestMemPool* t1 = (TestMemPool*)p.Alloc();
		t1->a = 100;
		t1->b = 200;
		t1->c = 'a';
		tArr[i] = t1;
		MemoryPool_Dump(&p);
	}
	for (int i = 0; i < 7; i++) {
		LOG("fffffffffffffffffffff:%d\n", i + 1);
		p.Free(tArr[i]);
		MemoryPool_Dump(&p);
	}*/

	/*
	VarMemPool memPool;
	MsgQueue<QMHead*> _queue;
	QMHead* head = _queue.front();
	int qmNum = 10;
	QMHead** qmArr = new QMHead*[qmNum];

	LOG("qm arr init begin >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	VarMemPool_Dump(&memPool);

	for (int i = 0; i < qmNum; i++) {
		if (i % 2 == 0) {
			QMClientIn* qm0 = (QMClientIn*)memPool.Alloc(sizeof(QMClientIn));
			qm0->Init();
			qm0->clientSock = i;
			qmArr[i] = qm0;
		}
		else {
			QMClientMsg* qm3 = (QMClientMsg*)memPool.Alloc(sizeof(QMClientMsg));
			qm3->Init();
			qm3->clientSock = i;
			MsgTestReq* tr = (MsgTestReq*)memPool.Alloc(sizeof(MsgTestReq));
			tr->Init("helloworld",1000);
			qm3->msg = tr;
			qmArr[i] = qm3;
		}
	}

	LOG("qm arr init end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	VarMemPool_Dump(&memPool);

	LOG("queue push begin >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MsgQueue_Dump(&_queue);

	for(int i=0;i<qmNum;i++){
		_queue.push(qmArr[i]);
	}
	
	LOG("queue push end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MsgQueue_Dump(&_queue);

	LOG("queue pop begin >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	for (int i = 0; i<qmNum; i++) {
		QMHead* temp = _queue.front();
		if (temp->handleType == QMType::ClientIn) {
			QMClientIn* clientIn = (QMClientIn*)temp;
			LOG("QMClientIn:clientSocket=%x\n",clientIn->clientSock);
		}
		else if (temp->handleType == QMType::ClientMsg) {
			QMClientMsg* clientMsg = (QMClientMsg*)temp;
			LOG("QMClientMsg:clientSocket=%x\n", clientMsg->clientSock);
			MsgTestReq* msgTestReq = (MsgTestReq*)clientMsg->msg;
			LOG("MsgTestReq:test1:%s\n",msgTestReq->test1);
			memPool.Free(msgTestReq, msgTestReq->size);
		}
		_queue.pop();
		memPool.Free(temp, temp->size);
	}

	LOG("queue pop end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MsgQueue_Dump(&_queue);
	VarMemPool_Dump(&memPool);
	*/
	
	std::vector<TestSize0*> _testVector;
	_testVector.reserve(10);
	printf("vector size is:%d\n", _testVector.size());
	printf("vector capacity is:%d\n", _testVector.capacity());
	for (auto pEle : _testVector) {
		printf("ele is:%lx\n", pEle);
	}
	//会报错，不能使用[]访问比size大的pos
	/*for (int i = 0; i<_testVector.capacity(); i++) {
		printf("ele is:%lx\n", _testVector[i]);
	}*/
	_testVector.clear();
	printf("vector size is:%d\n", _testVector.size());
	printf("vector capacity is:%d\n", _testVector.capacity());
	for (auto pEle : _testVector) {
		printf("ele is:%lx\n", pEle);
	}
	/*
	for (int i = _testVector.capacity() - 1; i >= 0; i--) {
		_testVector[i] = new TestSize0();
		printf("ele is:%lx\n", _testVector[i]);
	}*/
	//_testVector[111] = new TestSize0();

	std::ofstream out("TestFile.txt");
	out << "hello file" << std::endl;
	out.close();

	ConfigHelp cfgHelp;
	cfgHelp.init("ServerConfig.txt");
	cfgHelp.dump();

	SafeQueue<int> _queue(10);

	int qmNum = 10;

	LOG("queue push begin >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MsgQueue_Dump(&_queue);

	for (int i = 0; i<qmNum; i++) {
		_queue.push(i);
	}

	LOG("queue push end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MsgQueue_Dump(&_queue);

	LOG("queue pop begin >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	for (_queue.begin(); !_queue.end(); _queue.next()) {
		LOG("val is:%d\n",_queue.getVal());
	}

	LOG("queue pop end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MsgQueue_Dump(&_queue);

	_queue.clear();

	LOG("queue clear end >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	MsgQueue_Dump(&_queue);

#ifdef _WIN32
	system("pause");
#endif
    return 0;
}

