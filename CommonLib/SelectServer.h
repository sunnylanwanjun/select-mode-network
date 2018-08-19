#ifndef __SELECTSERVERH__
#define __SELECTSERVERH__
#include "ClientSocket.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "IServer.h"
#include <vector>
#include <map>
#include "Timestamp.h"
#include "IMsgHandle.h"

class DLL_API SelectServer:public IServer,public IMsgHandle{
private:
	std::vector<ClientSocket*> _clientSockArr;
	SOCKET _serverSock;
	Timestamp time;
	VarMemPool _memoryPool;
	SOCKET _maxSock;
	fd_set _fdReadBackup;
	bool _clientChange;
#ifdef DEBUG_SERVER_PERFORMANCE
public:
	//测试使用
	int _testMsgTimes;
	int _testSendTimes;
	int _testSendQueueSize;
	int _testClientCount;
	int _testRecvTimes;
#endif
public:
	SelectServer();
	~SelectServer();
	int getClientCount();
	void OnMsgHandle(IMsgSend* msgSender, MsgHead* msg) {}

//implement interface
public:
	int Bind(unsigned long addr, unsigned short port);
	int Listen(int backlog);
	void Close();
	bool IsRun();
	int OnRun();
	void broadcast(MsgHead* msg, std::map<IMsgSend*, bool>* excludeClient);
};
#endif
