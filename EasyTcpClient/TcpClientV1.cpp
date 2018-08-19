// EasyTcpClient.cpp : Defines the entry point for the console application.
//
#include "SocketHeader.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "Utils.h"
#include <mutex>
#include <thread>

char sendBuf[4096] = {};
bool threadRuning = true;
int processorSend(SOCKET _sock);
void inputThread(SOCKET _sock) {
	while(true){
		memset(sendBuf, 0, sizeof(sendBuf));
		scanf("%s", sendBuf);
		if (-1 == processorSend(_sock)) {
			threadRuning = false;
			return;
		}
	}
}

int processorSend(SOCKET _sock) {
	int sendSize = 0;
	if (0 == strcmp_ex(sendBuf, "exit")) {
		LOG("client close by user\n");
		return -1;
	}
	else if (0 == strcmp_ex(sendBuf, "login")) {
		std::vector<std::string> res = strsplit(sendBuf, "|");
		if (res.size() < 3) {
			LOG("please input login|xx|xx format\n");
			return 0;
		}
		MsgLoginReq login(res[1].c_str(),res[2].c_str());
		sendSize = writeBuffer(sendBuf, sizeof(sendBuf), &login);
	}
	else if (0 == strcmp_ex(sendBuf, "logout")) {
		std::vector<std::string> res = strsplit(sendBuf, "|");
		if (res.size() < 2) {
			LOG("please input logout|xx format\n");
			return 0;
		}
		MsgLogoutReq logout(res[1].c_str());
		sendSize = writeBuffer(sendBuf, sizeof(sendBuf), &logout);
	}
	else if (0 == strcmp_ex(sendBuf, "getInfo")) {
		MsgHead info(MsgID::Info);
		sendSize = writeBuffer(sendBuf, sizeof(sendBuf), &info);
	}
	else {
		MsgHead info(MsgID::Unknow);
		sendSize = writeBuffer(sendBuf, sizeof(sendBuf), &info);
	}

	if (sendSize > 0) {
		int sendLen = (int)send(_sock, sendBuf, sendSize, 0);
		if (sendLen <= 0) {
			LOG("ERROR:send msg failed\n");
			return -1;
		}
		LOG("send size is:%d\n", sendLen);
	}

	return 0;
}

int processorRecv(SOCKET _sock) {
	static char recvBuf[4096] = {};
	OBuffer ioBuf;
	int nlen = (int)recv(_sock, recvBuf, sizeof(recvBuf), 0);
	if (nlen <= 0) {
		LOG("ERROR:recv msg failed\n");
		return -1;
	}

	ioBuf.setBuffer(recvBuf, nlen);
	int msgSize = ioBuf.readUShort();
	int msgId = ioBuf.readUShort();
	int msgCode = ioBuf.readUShort();
	LOG("recv msg len:%d,msgId:%d,msgCode:%d\n", nlen, msgId, msgCode);
	ioBuf.reset();
	switch (msgId) {
		case MsgID::Info: {
			MsgInfoResp *info = (MsgInfoResp*)ioBuf.readBytes(sizeof(MsgInfoResp));
			LOG("recv MsgInfo age:%d,name:%s\n", info->age, info->name);
			break;
		}
		case MsgID::Unknow: {
			MsgUnknowResp *unknow = unknow = (MsgUnknowResp*)ioBuf.readBytes(sizeof(MsgUnknowResp));
			LOG("recv MsgUnknow name:%s\n", unknow->content);
			break;
		}
		case MsgID::Login: {
			if (msgCode == 0) {
				LOG("login server success\n");
			}
			else {
				LOG("login server failed\n");
			}
			break;
		}
		case MsgID::Logout: {
			if (msgCode == 0) {
				LOG("logout server success\n");
			}
			else {
				LOG("logout server failed\n");
			}
			break;
		}
		case MsgID::NewClientConn: {
			MsgNewClientConnectResp *newClient = (MsgNewClientConnectResp*)ioBuf.readBytes(sizeof(MsgNewClientConnectResp));
			LOG("new client sock is:%d\n", newClient->sock);
		}
		default:
			break;
	}
	return 0;
}

int run() {
	int retVal = 0;
	SOCKET _sock = INVALID_SOCKET;
	do {
#ifdef _WIN32
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			LOG("ERROR:create socket failed\n");
			retVal = -1;
			break;
		}
		else {
			LOG("create socket success\n");
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(4567);//host to net sequense
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
		_sin.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
		if (SOCKET_ERROR == ret) {
			LOG("ERROR:connect server failed\n");
			retVal = -1;
			break;
		}
		else {
			LOG("connect server success\n");
		}

		std::thread inputHandle(inputThread,_sock);
		inputHandle.detach();
		while (threadRuning) {
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExcept;
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExcept);
			FD_SET(_sock,&fdRead);
			timeval t = {0,0};
			int n = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);
			if (n < 0) {
				LOG("ERROR:select failed\n");
				break;
			}
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock,&fdRead);
				if (-1 == processorRecv(_sock)) {
					break;
				}
			}
		}
	} while (false);
	closesocket(_sock);
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}

//int main()
//{
//	int ret = run();
//	if (ret == -1) {
//		LOG("ERROR:run client error\n");
//	}
//#ifdef __WIN32
//	system("pause");
//#endif
//	return 0;
//}
