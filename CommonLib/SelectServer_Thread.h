#ifndef __SELECTSERVER_THREAD_H__
#define __SELECTSERVER_THREAD_H__
#include "IServer.h"
#include "ClientSocket.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "IServer.h"
#include "SafeQueue.h"
#include <list>
#include <map>
#include <thread>
#include "QueueMsg.h"
#include "VarMemPool.h"
#include <atomic>
#include "Timestamp.h"
#include "IMsgHandle.h"
#include "IMsgSend.h"

class DLL_API SelectClientHandle {	
private:
	bool _isRunging;
	bool _isHandleStop;
	std::list<ClientSocket*> _clientSockArr;
	SafeQueue<QMClientState>& _clientStateQueue;

	int _clientHandleID;
	bool _hasChangeClient;

	fd_set _fdRead_backup;
	SOCKET _maxSock;
	IMsgHandle* _msgHandle;

	VarMemPool _sendMsgPool;

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
	SelectClientHandle(int handleID,
		SafeQueue<QMClientState>& main2SubQueue,
		IMsgHandle* msgHandle);
	~SelectClientHandle();
	void Close();
	void HandleThread();
	void Broadcast(MsgHead* msg, const std::map<IMsgSend*, bool>& excludeClient);
	void GetSendMsgMemInfo(MemInfo* memInfo) {
		_sendMsgPool.GetInfo(memInfo);
	}
	int GetClientNum() {
		return _clientStateQueue.size() + _clientSockArr.size();
	}
	int GetHandleID() {
		return _clientHandleID;
	}
};

class DLL_API SelectServer_Thread:public IServer,public IMsgHandle {
private:
	int _CLIENT_HANDLE_THREAD;

	std::vector<SelectClientHandle*> _clientHandleArr;
	std::vector<SafeQueue<QMClientState>*> _clientStateQueueArr;

	SOCKET _serverSock;

	Timestamp time;
public:
	SelectServer_Thread();
	~SelectServer_Thread();
	int InitServer(int threadNum);
	int Bind(unsigned long addr, unsigned short port);
	int Listen(int backlog);
	void Close();
	bool IsRun();
	int OnRun();
	void OnMsgHandle(IMsgSend* msgSender, MsgHead* msg);
	void Broadcast(MsgHead* msg, const std::map<IMsgSend*, bool>& excludeClient);
};
#endif