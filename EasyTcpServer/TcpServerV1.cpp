// HelloSocket.cpp : Defines the entry point for the console application.
//
#include "SocketHeader.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Utils.h"
#include "Macro.h"

void broadcast(const std::vector<SOCKET>& sockArr, const MsgHead* msg) {
	static char _sendBuf[4096] = {};
	for (auto it = sockArr.begin(); it != sockArr.end(); it++) {
		int sendSize = writeBuffer(_sendBuf, sizeof(_sendBuf), msg);
		if (send(*it, _sendBuf, sendSize, 0) <= 0) {
			LOG("send error client %d has close\n", *it);
		}
	}
}

int processor(SOCKET _clientSock) {
	static char _recvBuf[4096] = {};
	static char _sendBuf[4096] = {};
	OBuffer ioBuf;

	int sendSize = 0;

	int nLen = (int)recv(_clientSock, _recvBuf, sizeof(_recvBuf), 0);
	if (nLen <= 0) {
		LOG("recv error client %d has close\n", _clientSock);
		return -1;
	}

	ioBuf.setBuffer(_recvBuf, nLen);
	unsigned short packSize = ioBuf.readUShort();
	unsigned short msgId = ioBuf.readUShort();
	LOG("recv client %d msg size=%d,msgId=%d\n", _clientSock, packSize, msgId);

	sendSize = 0;
	ioBuf.reset();
	switch (msgId) {
		case MsgID::Info: {
			MsgInfoResp info(80, "lwj");
			sendSize = writeBuffer(_sendBuf, sizeof(_sendBuf), &info);
			break;
		}
		case MsgID::Login: {
			MsgLoginReq* login = (MsgLoginReq*)ioBuf.readBytes(sizeof(MsgLoginReq));
			LOG("client %d userId:%s,pwd:%s req login\n", _clientSock, login->userId, login->passwd);
			MsgHead head(MsgID::Login);
			sendSize = writeBuffer(_sendBuf, sizeof(_sendBuf), &head);
			break;
		}
		case  MsgID::Logout: {
			MsgLogoutReq* logout = (MsgLogoutReq*)ioBuf.readBytes(sizeof(MsgLogoutReq));
			LOG("client %d userId:%s req logout\n", _clientSock, logout->userId);
			MsgHead head(MsgID::Logout);
			sendSize = writeBuffer(_sendBuf, sizeof(_sendBuf), &head);
			break;
		}
		default: {
			MsgUnknowResp info("what do you want to do");
			sendSize = writeBuffer(_sendBuf, sizeof(_sendBuf), &info);
			break;
		}
	}
	int sendLen = (int)send(_clientSock, _sendBuf, sendSize, 0);
	if (sendLen <= 0) {
		LOG("send error client %d has close\n", _clientSock);
		return -1;
	}
	LOG("send to client %d size is:%d\n", _clientSock, sendLen);

	if (msgId == MsgID::Logout) {
		return -1;
	}

	return 0;
}

int run() {
	int retVal = 0;
	SOCKET _sock = INVALID_SOCKET;
	std::vector<SOCKET> clientSockArr;
	clientSockArr.reserve(50);

	do {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(4567);//host to net sequense
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = INADDR_ANY;
#else
		_sin.sin_addr.s_addr = INADDR_ANY;
#endif
		if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
			LOG("ERROR,bind port failed\n");
			retVal = -1;
			break;
		}
		else {
			LOG("bind port success\n");
		}
		if (SOCKET_ERROR == listen(_sock, 5)) {
			LOG("ERROR,listen port failed\n");
			retVal = -1;
			break;
		}
		else {
			LOG("listen port success\n");
		}

		while (true) {
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExcept;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcept);

			FD_SET(_sock, &fdRead);
			//把客户端的socket都放在fdRead集合中
			SOCKET maxSock = _sock;
			for (auto it = clientSockArr.begin(); it != clientSockArr.end(); it++) {
				//添加的时候，会把元素往后添加
				FD_SET(*it, &fdRead);
				if (maxSock<*it) {
					maxSock = *it;
				}
			}

			timeval t = { 0,0 };
			int n = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);
			if (n < 0) {
				retVal = -1;
				break;
			}

			if (FD_ISSET(_sock, &fdRead)) {
				//把这个服务器socket从队列里面移除，否则下面遍历客户端是否有数据过来的时候，会判断错误
				//移除的时候，会把后面的数值往前移
				FD_CLR(_sock, &fdRead);
				sockaddr_in clientAddr = {};
				int nAddrLen = sizeof(clientAddr);
#ifdef _WIN32
				SOCKET _clientSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
				SOCKET _clientSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
				if (_clientSock == INVALID_SOCKET) {
					LOG("ERROR,accept client socket invalid\n");
				}
				else {
					MsgNewClientConnectResp newClient((int)_clientSock);
					broadcast(clientSockArr, &newClient);
					clientSockArr.push_back(_clientSock);
					LOG("accept client socket success,socket is:%d,client ip is:%s\n", _clientSock, inet_ntoa(clientAddr.sin_addr));
				}
			}
			for (auto it = clientSockArr.begin(); it != clientSockArr.end();) {
				if (FD_ISSET(*it, &fdRead) && processor(*it) == -1) {
					LOG("clear %d socket from container\n", *it);
					closesocket(*it);
					it = clientSockArr.erase(it);
				}
				else {
					it++;
				}
			}
		}

	} while (false);

	for (auto it = clientSockArr.begin(); it != clientSockArr.end(); it++) {
		closesocket(*it);
	}
	clientSockArr.clear();

	closesocket(_sock);
#ifdef _WIN32
	WSACleanup();
#endif
	return retVal;
}

//int main()
//{
//	int ret = run();
//	if (ret == -1) {
//		LOG("ERROR:run server error\n");
//	}
//	system("pause");
//	return 0;
//}
