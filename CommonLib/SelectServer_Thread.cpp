#include "SelectServer_Thread.h"
#include "IServer.h"
#include "ClientSocket.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "Utils.h"
#include "IServer.h"
#include "SafeQueue.h"
#include <list>
#include <map>
#include <thread>
#include "VarMemPool.h"
#include <atomic>
#include <fstream>
#include "ConfigHelp.h"
#include "Timestamp.h"
#include <iostream>

SelectClientHandle::SelectClientHandle(int handleID,
	SafeQueue<QMClientState>& clientStateQueue,
	IMsgHandle* msgHandle) :
#ifdef DEBUG_SERVER_PERFORMANCE
	_testMsgTimes(0),
	_testClientCount(0),
	_testSendQueueSize(0),
	_testSendTimes(0),
	_testRecvTimes(0),
#endif
	_isRunging(true),
	_isHandleStop(false),
	_clientHandleID(handleID),
	_clientStateQueue(clientStateQueue),
	_hasChangeClient(true),
	_maxSock(INVALID_SOCKET),
	_msgHandle(msgHandle),
	_sendMsgPool(ThreadSafeType::UnSafe)
{
	FD_ZERO(&_fdRead_backup);
	std::thread thRecv(&SelectClientHandle::HandleThread, this);
	thRecv.detach();
}

SelectClientHandle::~SelectClientHandle() {
	_isRunging = false;
	while (!_isHandleStop);
	Close();
	LOG("SelectClientHandle %d has finished\n", _clientHandleID);
}

void SelectClientHandle::Close() {
	for (auto pClient : _clientSockArr) {
		delete pClient;
	}
	_clientSockArr.clear();
}

void SelectClientHandle::HandleThread() {
	while (_isRunging) {
		if (_clientStateQueue.hasData()) {
			
			_clientStateQueue.lock();
			for (_clientStateQueue.begin(); !_clientStateQueue.end(); _clientStateQueue.next()) {
				QMClientState recvNode = _clientStateQueue.getVal();
				switch (recvNode.type) {
					case EClientState::Join:{
						SOCKET clientSock = recvNode.clientSock;
						_clientStateQueue.manualSize(-1);
						
#ifdef DEBUG_SERVER_PERFORMANCE
						_testClientCount++;
#endif
						ClientSocket* pClient = new ClientSocket(clientSock, _msgHandle, _sendMsgPool);

#ifdef DEBUG_SERVER_PERFORMANCE
						pClient->_testMsgTimes = &_testMsgTimes;
						pClient->_testSendTimes = &_testSendTimes;
						pClient->_testSendQueueSize = &_testSendQueueSize;
						pClient->_testRecvTimes = &_testRecvTimes;
#endif
						_clientSockArr.push_back(pClient);
						//LOG("SelectClientHandle:recv queue join sock:%d\n", pClient->_clientSock);
						break;
					}
					case EClientState::Leave: {
						for (auto it = _clientSockArr.begin(); it != _clientSockArr.end();) {
							if ((*it)->_clientSock == recvNode.clientSock) {
#ifdef DEBUG_SERVER_PERFORMANCE
								_testClientCount--;
#endif
								delete *it;
								_clientSockArr.erase(it);
								LOG("SelectClientHandle:recv queue leave sock:%d\n", recvNode.clientSock);
								break;
							}
						}
						break;
					}
					default:
						LOG("SelectClientHandle:recv queue unknow type:%d\n",recvNode.type);
						break;
				}
				
			}
			_clientStateQueue.clear();
			_clientStateQueue.unlock();
			_hasChangeClient = true;
			//LOG("GGGGGG new client:%d,threadId:%d,totalClient:%lu\n", pClientIn->clientSock, _clientHandleID, _clientSockArr.size());
		}

		if (_clientSockArr.size() <= 0) {
			std::chrono::milliseconds sleepTime(1);
			std::this_thread::sleep_for(sleepTime);
			continue;
		}

		//积压的数据包过大，需要暂停
		/*if (_sub2MainQueue.size() >= MsgQueue_Max){
		std::chrono::milliseconds sleepTime(1);
		std::this_thread::sleep_for(sleepTime);
		continue;
		}*/

		fd_set _fdRead;
		//fd_set _fdWrite;
		//fd_set _fdExcept;

		FD_ZERO(&_fdRead);
		//FD_ZERO(&_fdWrite);
		//FD_ZERO(&_fdExcept);

		if (_hasChangeClient) {
			FD_ZERO(&_fdRead_backup);
			//LOG("fd set server socket:%d\n", _serverSock);
			//把客户端的socket都放在fdRead集合中
			_maxSock = _clientSockArr.front()->_clientSock;
			for (auto pClient : _clientSockArr) {
				//添加的时候，会把元素往后添加
				//LOG("fd set client socket:%d\n",*it);
				FD_SET(pClient->_clientSock, &_fdRead_backup);
				if (_maxSock<pClient->_clientSock) {
					_maxSock = pClient->_clientSock;
				}
			}
			_hasChangeClient = false;
		}
		memcpy(&_fdRead, &_fdRead_backup, sizeof(_fdRead_backup));

		//timeval t = { 0,10 };
		//int n = select(_maxSock + 1, &_fdRead, &_fdWrite, &_fdExcept, &t);
		//int n = select(_maxSock + 1, &_fdRead,0, 0, &t);
		//在大量连接进来后，但没有发数据时，子线程不处理连接，于是可以让主线程快速处理请求
		//不与主线程争抢资源
		//select之后，fdRead中只会保留有数据读取的sock
		int n = select(_maxSock + 1, &_fdRead, 0, 0, nullptr);
		if (n < 0) {
			Close();
			continue;
		}

		for (auto it = _clientSockArr.begin(); it != _clientSockArr.end();) {
			if ( (FD_ISSET((*it)->_clientSock, &_fdRead) && (*it)->ProcessorRecv() == -1) ||
				((*it)->ProcessorSend() == -1)
			   ) {
#ifdef DEBUG_SERVER_PERFORMANCE
				_testClientCount--;
#endif
				delete *it;
				it = _clientSockArr.erase(it);
				_hasChangeClient = true;
				continue;
			}
			it++;
		}
	}

	_isHandleStop = true;
}

SelectServer_Thread::SelectServer_Thread() :
	_CLIENT_HANDLE_THREAD(0),
	_serverSock(INVALID_SOCKET){
}

SelectServer_Thread::~SelectServer_Thread() {
	Close();
}

int SelectServer_Thread::InitServer(int threadNum) {
	_CLIENT_HANDLE_THREAD = threadNum;
	if (_CLIENT_HANDLE_THREAD < 1) {
		LOG("ServerConfig ThreadNum Format illegal 222222 \n");
		return -1;
	}
	printf("Server Create Thread Num:%d\n", _CLIENT_HANDLE_THREAD);

	for (int i = 0; i < _CLIENT_HANDLE_THREAD; i++) {
		_clientStateQueueArr.push_back(new SafeQueue<QMClientState>(SERVER_CLIENT_STATE_QUEUE_SIZE));
		SelectClientHandle* clientHandle = new SelectClientHandle(i, *_clientStateQueueArr[i],this);
		_clientHandleArr.push_back(clientHandle);
	}
	return 0;
}

int SelectServer_Thread::Bind(unsigned long addr, unsigned short port) {
	LOG("fd_set size is:%lu,FD_SETSIZE:%d\n", sizeof(fd_set), FD_SETSIZE);
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	_serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _serverSock) {
		LOG("ERROR:create socket failed\n");
#ifdef _WIN32
		WSACleanup();
#endif
		return -1;
	}
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);//host to net sequense
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = addr;
#else
	_sin.sin_addr.s_addr = INADDR_ANY;
#endif
	char* ip = inet_ntoa(_sin.sin_addr);
	LOG("bind addr:%s port:%d\n", ip, port);

	if (SOCKET_ERROR == ::bind(_serverSock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
		LOG("ERROR,bind port failed\n");
		Close();
		return -1;
	}
	else {
		LOG("bind port success\n");
	}
	return 0;
}

/*
想要清楚的解释backlog，就必须先了解tcp ip的三次握手
当服务器绑定完端口以后，开始监听端口，这时，有客户端发起连接，过程如下

1 客户端发起connect连接请求，客户端阻塞，客户端状态为syn_send

2 服务器收到syn1包，将该连接放入未完成连接队列中

3 当服务器accept时，该连接放入完成连接队列中，并发给客户端一个ack1包，
并同时发送一个syn2包，然后服务器的accept阻塞，此时服务器进入syn_recv状态

4 客户端收到ack1包后，客户端connect返回，向服务器发送针对syn2包的ack2包，
客户端状态为established，表示已经确定连接

5 服务器收到ack2包后，accept返回，并把连接从完成连接队列中移除，服务器状态为
established

结束：3次握手的过程结束。而这个backlog就是完成连接队列的大小。
*/
int SelectServer_Thread::Listen(int backlog) {
	LOG("listen backlog is:%d\n", backlog);

	if (SOCKET_ERROR == listen(_serverSock, backlog)) {
		LOG("ERROR,listen port failed\n");
		Close();
		return -1;
	}
	else {
		LOG("listen port success\n");
		return 0;
	}
}

void SelectServer_Thread::Close() {
	if (_serverSock != INVALID_SOCKET) {
		closesocket(_serverSock);
		_serverSock = INVALID_SOCKET;
#ifdef _WIN32
		WSACleanup();
#endif
		//删除客户端处理
		for (auto pClient : _clientHandleArr) {
			if (pClient != nullptr) {
				delete pClient;
			}
		}
		_clientHandleArr.clear();
		//清空剩余消息队列
		for (auto pMsgQueue : _clientStateQueueArr) {
			delete pMsgQueue;
		}
	}
}

bool SelectServer_Thread::IsRun() {
	return _serverSock != INVALID_SOCKET;
}

int SelectServer_Thread::OnRun() {
	if (_serverSock == INVALID_SOCKET)
		return -1;

	////////////////////////////////////////////////////
	//测试收包次数
#ifdef DEBUG_SERVER_PERFORMANCE
	double goTime = time.GetS();
	if (goTime >= 1.0) {

		static MemInfo memInfo;
		static int _testMsgTimes;
		static int _testSendTimes;
		static int _testSendQueueSize;
		static int _testClientCount;
		static int _testRecvTimes;

		memset(&memInfo, 0, sizeof(MemInfo));
		_testMsgTimes = 0;
		_testSendTimes = 0;
		_testSendQueueSize = 0;
		_testClientCount = 0;
		_testRecvTimes = 0;

		for (auto pClient : _clientHandleArr) {
			pClient->GetSendMsgMemInfo(&memInfo);
			_testMsgTimes += pClient->_testMsgTimes;
			_testRecvTimes += pClient->_testRecvTimes;
			_testSendTimes += pClient->_testSendTimes;
			_testSendQueueSize += pClient->_testSendQueueSize;
			_testClientCount += pClient->_testClientCount;

			pClient->_testRecvTimes = 0;
			pClient->_testMsgTimes = 0;
			pClient->_testSendTimes = 0;
		}

		std::cout << "recv:" << _testRecvTimes;// (int)(_testRecvTimes / goTime);
		std::cout << ",msg:" << _testMsgTimes;// (int)(_testMsgTimes / goTime);
		std::cout << ",send:" << _testSendTimes;// (int)(_testSendTimes / goTime);
		std::cout << ",sendSize:" << _testSendQueueSize;
		
		std::cout << ",sendMem:" << memInfo.freeMem / 1000 << "k" << "/" << memInfo.totalMem / 1000 << "k";
		std::cout << ",client:" << _testClientCount;
		std::cout << ",time:" << goTime << std::endl;
		time.Update();
	}
#endif
	////////////////////////////////////////////////////

	fd_set _fdRead;
	//fd_set _fdWrite;
	//fd_set _fdExcept;

	FD_ZERO(&_fdRead);
	//FD_ZERO(&_fdWrite);
	//FD_ZERO(&_fdExcept);

	//LOG("fd set server socket:%d\n", _serverSock);
	FD_SET(_serverSock, &_fdRead);

	//不果丝毫不等待，那么由于主线程的主要任务就是接收连接请求，那很有可能没有命中，
	//而白白浪费了一次cpu时间，由于有子线程的存在，以及错过一次，就要重新调用函数OnRun
	//代价反而更大了。
	timeval t = { 0,10 };
	//int n = select(_serverSock + 1, &_fdRead, &_fdWrite, &_fdExcept, &t);
	int n = select(_serverSock + 1, &_fdRead, 0, 0, &t);
	if (n < 0) {
		Close();
		return -1;
	}

	if (FD_ISSET(_serverSock, &_fdRead)) {
		//把这个服务器socket从队列里面移除，否则下面遍历客户端是否有数据过来的时候，会判断错误
		//移除的时候，会把后面的数值往前移
		FD_CLR(_serverSock, &_fdRead);
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(clientAddr);
#ifdef _WIN32
		SOCKET _clientSock = accept(_serverSock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		SOCKET _clientSock = accept(_serverSock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
		if (_clientSock == INVALID_SOCKET) {
			LOG("ERROR,accept client socket invalid\n");
		}
		else {

			//assert(_clientHandleArr.size()>0);
			int minId = 0;
			int minClient = _clientHandleArr.front()->GetClientNum();

			for (auto pClient : _clientHandleArr) {
				if (minClient > pClient->GetClientNum()) {
					minId = pClient->GetHandleID();
					minClient = pClient->GetClientNum();
				}
			}

			_clientStateQueueArr[minId]->push({EClientState::Join,_clientSock});
			//LOG("accept new client:%d,threadId:%d,totalClient:%d\n", _clientSock, minId, _totalClientNum);
		}
	}
	return 0;
}

//void SelectServer_Thread::OnQMClient(QMHead* pQMHead) {
//	switch (pQMHead->handleType) {
//		case QMType::ClientOut: {
//			QMClientOut* pClientOut = (QMClientOut*)pQMHead;
//			SOCKET clientSock = pClientOut->clientSock;
//			_sock2IDMap.erase(clientSock);
//			_eachClientNum[pClientOut->handleId]--;
//			_totalClientNum--;
//			//LOG("handle qm handleID:%d,threadClientNum:%d\n", pClientOut->handleId, pClientOut->clientNum);
//			assert(_eachClientNum[pClientOut->handleId] >= 0);
//			assert(_totalClientNum >= 0);
//			break;
//		}
//		case QMType::ClientMsg: {
//			QMClientMsg* pClientMsg = (QMClientMsg*)pQMHead;
//			memcpy(_package, pClientMsg->msg, pClientMsg->msg->size);
//			OnRecvMsg(pClientMsg->clientSock, (MsgHead*)_package, pClientMsg->handleId, pClientMsg->clientNum, _sub2MainQueue.size());
//			_memoryPool.Free(pClientMsg->msg, pClientMsg->msg->size);
//			break;
//		}
//	}
//}

//int SelectServer_Thread::SendMsg(SOCKET clientSock, MsgHead* msg) {
//	if (_serverSock == INVALID_SOCKET || clientSock == INVALID_SOCKET) {
//		return -1;
//	}
//	LOG("SendMsg clientSock:%d msgId:%d,msgSize:%d,msgCode:%d\n", clientSock, msg->msgId, msg->size, msg->code);
//	int totalLen = msg->size;
//	int offset = 0;
//	while (totalLen > 0) {
//		int sendLen = send(clientSock, (char*)msg + offset, totalLen, 0);
//		if (sendLen <= 0) {
//			LOG("ERROR:send msg failed\n");
//			return -1;
//		}
//		totalLen -= sendLen;
//		offset += sendLen;
//	}
//	return 0;
//}

void SelectServer_Thread::OnMsgHandle(IMsgSend* msgSender,MsgHead* msg) {
	/*
	QMClientMsg* pQMClient = (QMClientMsg*)_memoryPool.Alloc(sizeof(QMClientMsg));
	pQMClient->Init();
	pQMClient->clientNum = _clientSockArr.size();
	pQMClient->handleId = _clientHandleID;
	pQMClient->clientSock = clientSock;
	pQMClient->msg = (MsgHead*)_memoryPool.Alloc(msg->size);
	memcpy(pQMClient->msg, msg, msg->size);
	_sub2MainQueue.push(pQMClient);
	*/
}