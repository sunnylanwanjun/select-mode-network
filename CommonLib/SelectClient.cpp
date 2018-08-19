#include "SelectClient.h"
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

int SelectClient::processorRecv() {
	int nlen = (int)recv(_sock, _recvBuf + _retainSize, sizeof(_recvBuf) - _retainSize, 0);
	if (nlen <= 0) {
		LOG("ERROR:recv msg failed sock:%d\n", _sock);
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
			//LOG("111111111111111111 processorRecv recv buff no enough len:%d,need len:%lu\n", canReadSize, sizeof(MsgHead));
			break;
		}
		unsigned short  msgLen = _oRecv.readUShort();
		assert(msgLen <= PACKAGE_MAX_SIZE);
		_oRecv.moveReadPos(-2);
		//不够读取一个完整包
		if (msgLen > canReadSize) {
			//LOG("222222222222222222 processorRecv recv buff no enough len:%d,need len:%d\n", canReadSize, msgLen);
			break;
		}
		memset(_package, 0, sizeof(_package));
		_oRecv.readBytes(_package, msgLen);
		OnRecvMsg((MsgHead*)_package);
		packageNum++;
	}
	//将剩余数据往前挪动
	_oRecv.moveDataToFront();
	_retainSize = _oRecv.canRead();
	//LOG("eeeeeeeeeeeeeeeeee processorRecv handle package num is:%d,retian msg len:%d\n", packageNum, _retainSize);

	return 0;
}

SelectClient::SelectClient() :
	_sock(INVALID_SOCKET), 
	_retainSize(0) {

}

SelectClient::~SelectClient() {
	Close();
}

int SelectClient::Connect(const char* ip, unsigned short port) {
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

void SelectClient::Close() {
	if (INVALID_SOCKET != _sock) {
		closesocket(_sock);
		_sock = INVALID_SOCKET;
#ifdef _WIN32
		WSACleanup();
#endif
	}
}

bool SelectClient::IsRun() {
	return _sock != INVALID_SOCKET;
}

int SelectClient::OnRun() {
	if (_sock == INVALID_SOCKET)
		return -1;

	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExcept;
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExcept);
	FD_SET(_sock, &fdRead);
	timeval t = { 0,0 };
	int n = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);
	if (n < 0) {
		LOG("ERROR:select failed\n");
		Close();
		return -1;
	}
	if (FD_ISSET(_sock, &fdRead)) {
		FD_CLR(_sock, &fdRead);
		if (-1 == processorRecv()) {
			Close();
			return -1;
		}
	}
	return 0;
}

void SelectClient::OnRecvMsg(MsgHead* msg) {

}

int SelectClient::SendData(char* data,USHORT dataLen) {
	if (_sock == INVALID_SOCKET) {
		return -1;
	}

	//LOG("SendMsg msgId:%d,msgSize:%d,msgCode:%d\n", msg->msgId, msg->size, msg->code);
	int offset = 0;
	while (dataLen > 0) {
		int sendLen = send(_sock, data + offset, dataLen, 0);
		if (sendLen <= 0) {
			LOG("ERROR:send msg failed\n");
			return -1;
		}
		dataLen -= sendLen;
		offset += sendLen;
	}
	return 0;
}

int SelectClient::SendMsg(MsgHead* msg) {
	return SendData((char*)msg,msg->size);
}