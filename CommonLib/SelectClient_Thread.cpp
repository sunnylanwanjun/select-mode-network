#include "SelectClient_Thread.h"
#include "SocketHeader.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "Utils.h"
#include "IClient.h"
#include <thread>
#include <mutex>
#include "SafeQueue.h"
#include "VarMemPool.h"
#include "Factory.h"

/*
多线程读写socket线程安全，无需加锁，但多线程关闭socket
必须加锁，因为关闭之后，需要对socket赋值为INVALID_SOCKET
而这个赋值过程需要加锁保护
tcp有发送缓冲区，作用就是用来保证tcp的可靠连接，数据不会丢。
因为只有当对方的接收缓冲区里面存放了发送的数据，该发送端的
对应的发送缓冲区才会清空，而如果对方的接收缓冲一直没有清空，
也就是应用层没有把数据从接收缓冲区读走，那么发送端调用send就
会阻塞，所以这就是为什么发送时需要使用多线程进行发送。
为什么一个一个包发送，到了当收端也会变成n个包连在一起呢？
因为虽然一个一个包发送，接收端当第一个包到达的时候，立刻调用
read把包读下来，这个时候还要作各种关于切包，组包的业务逻辑，还
要抛给应用层，如果这个过程还是单线程的，那么接收缓冲区就很有可能
会满，这也解释了，为什么接收也应该开一个线程，这样读取缓冲区
的业务逻辑就会少一些，不会受到业务逻辑的干扰，减慢读取的速度。
至于有没有必要搞个缓冲区用来装发送消息包，然后再把这个缓冲区拷贝
到tcp的发送缓冲区，减少内核调用次数会不会性能更高？？？
UDP没有发送缓冲区，是因为它不需要保证数据的发送可靠性。
但UDP有接收缓冲区，因为既然数据到了接收端，那么就操作系统内核就
应该把数据暂存起来。
*/
int SelectClient_Thread::ProcessorSend() {
	//判断当前能否发送数据
	fd_set fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(_sock, &fdWrite);
	timeval t = { 0,10 };
	int n = select(_sock + 1, 0, &fdWrite, 0, &t);
	if (n < 0) {
		LOG("ERROR:select write failed socket:%d\n", _sock);
		Close();
		return -1;
	}
	if (FD_ISSET(_sock, &fdWrite)) {
		FD_CLR(_sock, &fdWrite);
		int totalLen = _ioSend.canRead();
		int sendLen = send(_sock, _sendBuf, totalLen, 0);
		if (sendLen <= 0) {
			LOG("ERROR:send msg failed socket:%d\n", _sock);
			Close();
			return -1;
		}
		_ioSend.moveReadPos(sendLen);
		_ioSend.moveDataToFront();
		return 1;
	}
	return 0;
}

int SelectClient_Thread::SendThread() {
	if (_sock == INVALID_SOCKET)
		return -1;
	do{
		do{
			//先判断发送缓冲区有没有残留数据没发出去
			int canWrite = _ioSend.canWrite();
			if (canWrite <= 0) {
				//尝试发送数据
				int n = ProcessorSend();
				if (n < 0)return -1;//网络出错
				//暂时不能写
				if (n == 0) {
					if (_sendQueue->size()>CLIENT_SEND_QUEUE_MAX_ACCUMULATE_COUNT) {
						LOG("error 111111:client %d send queue accumulate to many,size is:%d", _sock, _sendQueue->size());
						Close();
						return -1;
					}
					return 0;
				}
			}
			//再判断当前是否还有数据在OBuffer里面
			int canRead = _oMsg.canRead();
			if (canRead <= 0){
				//OBuffer里如果没有数据，说明第一次进来这里，不用释放资源
				if (_oMsg.getBuffer()!=nullptr) {

#ifdef DEBUG_CLIENT_PERFORMANCE
					if (_testSendQueueSize) {
						(*_testSendQueueSize)--;
					}
#endif

					QMSend qmSend = _sendQueue->pop();
					_memoryPool.Free(qmSend.data, qmSend.size);
					_oMsg.setBuffer(0, 0);
				}
				break;
			}
			int writeLen = canRead < canWrite ? canRead : canWrite;
			_oMsg.readBytes(&_ioSend, writeLen);
		} while (true);
		if (!_sendQueue->hasData())
			break;
		QMSend qmSend = _sendQueue->front();
		_oMsg.setBuffer((char*)qmSend.data, qmSend.size);
	} while (true);

	//当缓冲区有数据时，把数据发送出去，走到这，说明消息队列里面的消息都被取出
	int canRead = _ioSend.canRead();
	if (canRead > 0) {
		int n = ProcessorSend();
		if (n < 0)return -1;//网络出错
		//暂时不能写
		if (n == 0) {
			if (_sendQueue->size()>CLIENT_SEND_QUEUE_MAX_ACCUMULATE_COUNT) {
				LOG("error 22222:client %d send queue accumulate to many,size is:%d", _sock, _sendQueue->size());
				Close();
				return -1;
			}
			return 0;
		}
	}
	return 0;
}

int SelectClient_Thread::ProcessorRecv() {
	int nlen = (int)recv(_sock, _recvBuf + _retainSize, sizeof(_recvBuf) - _retainSize, 0);
	if (nlen <= 0) {
		//LOG("ERROR:recv msg failed sock:%d\n", _sock);
		return -1;
	}

	_oRecv.setBuffer(_recvBuf, nlen + _retainSize);
	//LOG("bbbbbbbbbbbbbbbbbb processorRecv recv buf len is:%d,retain len is:%d,total len is:%d\n", nlen, _retainSize, nlen + _retainSize);
	_retainSize = 0;
	int packageNum = 0;
	while (true) {
		//残留数据不足读取一个包头
		int canReadSize = _oRecv.canRead();
		if (canReadSize < sizeof(MsgHead)) {
			//LOG("111111111111111111 processorRecv recv buff no enough len:%d,need len:%d\n", canReadSize, sizeof(MsgHead));
			break;
		}
		unsigned short  msgLen = _oRecv.readUShort();
		assert(msgLen <= PACKAGE_MAX_SIZE);
		if (msgLen > PACKAGE_MAX_SIZE) {
			Close();
			return -1;
		}
		_oRecv.moveReadPos(-2);
		//不够读取一个完整包
		if (msgLen > canReadSize) {
			//LOG("222222222222222222 processorRecv recv buff no enough len:%d,need len:%d\n", canReadSize, msgLen);
			break;
		}

#ifdef DEBUG_CLIENT_PERFORMANCE
		if (_testMsgTimes) {
			(*_testMsgTimes)++;
		}
		if (_testRecvQueueSize) {
			(*_testRecvQueueSize)++;
		}
#endif

		//为了测试性能，暂时屏蔽推送包体，只是简单的偏移读指针
		#if 0
			_oRecv.moveReadPos(msgLen);
		#else
			char* package = (char*)_memoryPool.Alloc(msgLen);
			_oRecv.readBytes(package, msgLen);
			_recvQueue->push((MsgHead*)package);
		#endif
		packageNum++;

		if (_recvQueue->size()>CLIENT_RECV_QUEUE_MAX_ACCUMULATE_COUNT) {
			LOG("error:client %d recv queue accumulate to many,size is:%d", 
				_sock, _recvQueue->size());
			Close();
			return -1;
		}

	}
	//将剩余数据往前挪动
	_oRecv.moveDataToFront();
	_retainSize = _oRecv.canRead();
	//LOG("eeeeeeeeeeeeeeeeee processorRecv handle package num is:%d,retian msg len:%d\n", packageNum, _retainSize);

	return 0;
}

int SelectClient_Thread::RecvThread() {
	if (_sock == INVALID_SOCKET)
		return -1;
	fd_set fdRead;
	FD_ZERO(&fdRead);
	FD_SET(_sock, &fdRead);
	timeval t = { 0,10 };
	int n = select(_sock + 1, &fdRead, 0, 0, &t);
	if (n < 0) {
		//LOG("ERROR:select failed\n");
		Close();
		return -1;
	}
	if (FD_ISSET(_sock, &fdRead)) {
		FD_CLR(_sock, &fdRead);
		if (-1 == ProcessorRecv()) {
			Close();
			return -1;
		}
	}
	return 0;
}

SelectClient_Thread::SelectClient_Thread(VarMemPool& memoryPool) :
	_sock(INVALID_SOCKET), 
	_retainSize(0), 
	_recvQueue(nullptr), 
	_sendQueue(nullptr),
	_memoryPool(memoryPool){
	_safeType = _memoryPool.safeType;
	_ioSend.setBuffer(_sendBuf,sizeof(_sendBuf));
	_recvQueue = Factory::createQueue<MsgHead*>(_safeType, CLIENT_SEND_QUEUE_SIZE);
	_sendQueue = Factory::createQueue<QMSend>(_safeType, CLIENT_SEND_QUEUE_SIZE);
}

SelectClient_Thread::~SelectClient_Thread() {
	Close();
	delete _recvQueue;
	_recvQueue = nullptr;
	delete _sendQueue;
	_sendQueue = nullptr;
}

int SelectClient_Thread::Connect(const char* ip, unsigned short port) {
	Close();

#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock) {
		LOG("ERROR:create socket failed\n");
#ifdef _WIN32
		WSACleanup();
#endif
		return -1;
	}
	else {
		//LOG("create socket success\n");
	}
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);//host to net sequense
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif

	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret) {
		LOG("ERROR:connect server failed\n");
		Close();
		return -1;
	}
	else {
		//LOG("connect server success\n");
	}

	return 0;
}

void SelectClient_Thread::Close() {
	if (_safeType == ThreadSafeType::Safe) {
		_sockMutex.lock();
	}
	if (INVALID_SOCKET != _sock) {
		closesocket(_sock);
		_sock = INVALID_SOCKET;
#ifdef _WIN32
		WSACleanup();
#endif
		ClearQueue();
		ClearBuffer();
	}
	if (_safeType == ThreadSafeType::Safe) {
		_sockMutex.unlock();
	}
}

bool SelectClient_Thread::IsRun() {
	return _sock != INVALID_SOCKET;
}

int SelectClient_Thread::OnRun() {
	if (_sock == INVALID_SOCKET)
		return -1;
	while (_recvQueue->hasData()) {

#ifdef DEBUG_CLIENT_PERFORMANCE
		if (_testRecvQueueSize) {
			(*_testRecvQueueSize)--;
		}
#endif

		MsgHead* msg = _recvQueue->pop();
		memset(_package, 0, sizeof(_package));
		memcpy(_package, msg, msg->size);
		_memoryPool.Free(msg, msg->size);
		OnRecvMsg((MsgHead*)_package);
		VarMemPool_SDump(&memeryPool);
	}
	return 0;
}

int SelectClient_Thread::SendData(char* data, USHORT dataLen) {
	if (_sock == INVALID_SOCKET) {
		return -1;
	}

#ifdef DEBUG_CLIENT_PERFORMANCE
	if (_testSendTimes) {
		(*_testSendTimes)++;
	}
	if (_testSendQueueSize) {
		(*_testSendQueueSize)++;
	}
#endif

	char* package = (char*)_memoryPool.Alloc(dataLen);
	memcpy(package, data, dataLen);
	_sendQueue->push({dataLen,(char*)package });
	return 0;
}

int SelectClient_Thread::SendMsg(MsgHead* msg) {
	if (_sock == INVALID_SOCKET) {
		return -1;
	}
	return SendData((char*)msg,msg->size);
}

void SelectClient_Thread::ClearQueue() {
	while (_recvQueue->hasData()) {

#ifdef DEBUG_CLIENT_PERFORMANCE
		if (_testRecvQueueSize) {
			(*_testRecvQueueSize)--;
		}
#endif

		MsgHead* msg = _recvQueue->pop();
		_memoryPool.Free(msg, msg->size);
	}

	while (_sendQueue->hasData()) {

#ifdef DEBUG_CLIENT_PERFORMANCE
		if (_testSendQueueSize) {
			(*_testSendQueueSize)--;
		}
#endif

		QMSend sendData = _sendQueue->pop();
		MsgHead* msg = (MsgHead*)sendData.data;
		_memoryPool.Free(msg, sendData.size);
	}
}

void SelectClient_Thread::ClearBuffer() {
	_ioSend.setBuffer(_sendBuf, sizeof(_sendBuf));
	_oMsg.setBuffer(0, 0);
	_oRecv.setBuffer(0, 0);
}

void SelectClient_Thread::OnRecvMsg(MsgHead* msg) {

}
