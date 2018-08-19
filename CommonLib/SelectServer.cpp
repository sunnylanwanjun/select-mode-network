#include "SelectServer.h"
#include "ClientSocket.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "Utils.h"
#include "IServer.h"
#include "SafeQueue.h"
#include <vector>
#include <map>
#include <iostream>

SelectServer::SelectServer() :
	_serverSock(INVALID_SOCKET),
	_testMsgTimes(0),
	_testClientCount(0),
	_testSendQueueSize(0),
	_testSendTimes(0),
	_testRecvTimes(0),
	_memoryPool(ThreadSafeType::UnSafe),
	_maxSock(INVALID_SOCKET),
	_clientChange(true){
	_clientSockArr.reserve(100);
	memset(&_fdReadBackup, 0, sizeof(_fdReadBackup));
}

SelectServer::~SelectServer() {
	Close();
}

int SelectServer::getClientCount() {
	return _clientSockArr.size();
}

int SelectServer::Bind(unsigned long addr, unsigned short port) {
	LOG("fd_set size is:%lu,FD_SETSIZE:%d\n", sizeof(fd_set),FD_SETSIZE);
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

int SelectServer::Listen(int backlog) {
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
void SelectServer::Close() {
	if (_serverSock != INVALID_SOCKET) {
		for (auto it = _clientSockArr.begin(); it != _clientSockArr.end(); it++) {
			delete *it;
		}
		_clientSockArr.clear();
		closesocket(_serverSock);
		_serverSock = INVALID_SOCKET;
#ifdef _WIN32
		WSACleanup();
#endif
	}
}

bool SelectServer::IsRun() {
	return _serverSock != INVALID_SOCKET;
}

int SelectServer::OnRun() {
	if (_serverSock == INVALID_SOCKET)
		return -1;

	////////////////////////////////////////////////////
	//测试收包次数
	double goTime = time.GetS();
	if (goTime >= 1.0) {
		std::cout << "recv:" << _testRecvTimes;// (int)(_testMsgTimes / goTime);
		std::cout << ",msg:" << _testMsgTimes;// (int)(_testMsgTimes / goTime);
		std::cout << ",send:" << _testSendTimes;// (int)(_testSendTimes / goTime);
		std::cout << ",sendSize:" << _testSendQueueSize;
		static MemInfo memInfo;
		memset(&memInfo, 0, sizeof(MemInfo));
		_memoryPool.GetInfo(&memInfo);
		std::cout << ",sendMem:" << memInfo.freeMem / 1000 << "k" << "/" << memInfo.totalMem / 1000 << "k";
		std::cout << ",client:" << _testClientCount;
		std::cout << ",time:" << goTime << std::endl;
		time.Update();
		_testRecvTimes = 0;
		_testMsgTimes = 0;
		_testSendTimes = 0;
	}
	////////////////////////////////////////////////////

	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExcept;
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExcept);

	if (_clientChange) {
		//LOG("fd set server socket:%d\n", _serverSock);
		FD_SET(_serverSock, &_fdReadBackup);
		//把客户端的socket都放在fdRead集合中
		for (auto it = _clientSockArr.begin(); it != _clientSockArr.end(); it++) {
			//添加的时候，会把元素往后添加
			//LOG("fd set client socket:%d\n",*it);
			FD_SET((*it)->_clientSock, &_fdReadBackup);
			if (_maxSock<(*it)->_clientSock) {
				_maxSock = (*it)->_clientSock;
			}
		}
		_clientChange = false;
	}
	memcpy(&fdRead, &_fdReadBackup, sizeof(_fdReadBackup));

	timeval t = { 0,0 };
	int n = select(_maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);
	if (n < 0) {
		LOG("ERROR:select return %d,maxSock:%d,_serverSock:%d\n",n, _maxSock, _serverSock);
		Close();
		return -1;
	}

	if (FD_ISSET(_serverSock, &fdRead)) {
		//把这个服务器socket从队列里面移除，否则下面遍历客户端是否有数据过来的时候，会判断错误
		//移除的时候，会把后面的数值往前移
		FD_CLR(_serverSock, &fdRead);
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
			ClientSocket* pClient = new ClientSocket(_clientSock,this, _memoryPool);
#ifdef DEBUG_SERVER_PERFORMANCE
			pClient->_testMsgTimes = &_testMsgTimes;
			pClient->_testSendTimes = &_testSendTimes;
			pClient->_testSendQueueSize = &_testSendQueueSize;
			pClient->_testRecvTimes = &_testRecvTimes;
#endif
			_clientSockArr.push_back(pClient);
			_testClientCount++;
			_clientChange = true;
			//LOG("accept client socket success,socket is:%d,client ip is:%s,client num:%d\n", _clientSock, inet_ntoa(clientAddr.sin_addr),_clientSockArr.size());
		}
	}

	for (auto it = _clientSockArr.begin(); it != _clientSockArr.end();) {
		if ( (FD_ISSET((*it)->_clientSock, &fdRead) && (*it)->ProcessorRecv() == -1)
			 || ((*it)->ProcessorSend() == -1)
		   ){
#ifdef DEBUG_SERVER_PERFORMANCE
			_testClientCount--;
#endif
			delete *it;
			it = _clientSockArr.erase(it);
			_clientChange = true;
			continue;
		}
		it++;
	}
	//if (_clientSockArr.size() == 0) {
	//	LOG("all client has close\n");
	//}
	return 0;
}

void SelectServer::broadcast(MsgHead* msg, std::map<IMsgSend*, bool>* excludeClient) {
	if (_serverSock == INVALID_SOCKET)
		return;
	for (auto it = _clientSockArr.begin(); it != _clientSockArr.end(); it++) {
		if (excludeClient&&excludeClient->find(*it) != excludeClient->end()) {
			continue;
		}
		(*it)->SendMsg(msg);
	}
}