#ifndef __SELECTCLIENTH__
#define __SELECTCLIENTH__
#include "SocketHeader.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "IClient.h"
class DLL_API SelectClient :public IClient {
protected:
	SOCKET _sock;

	OBuffer _oRecv;
	int _retainSize;
	char _recvBuf[CLIENT_RECV_BUFFER_SIZE];
	char _package[PACKAGE_MAX_SIZE];
private:
	int processorRecv();
public:
	SelectClient();
	virtual ~SelectClient();
	int Connect(const char* ip, unsigned short port);
	void Close();
	bool IsRun();
	int OnRun();
	void OnRecvMsg(MsgHead* msg);
	int SendMsg(MsgHead* msg);
	int SendData(char* data, USHORT dataLen);
};
#endif