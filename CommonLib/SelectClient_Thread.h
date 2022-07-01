#ifndef __SELECTCLIENT_THREAD_H__
#define __SELECTCLIENT_THREAD_H__
#include "SocketHeader.h"
#include <stdio.h>
#include <cstdlib>
#include "IOBuffer.h"
#include "Msg.h"
#include "Macro.h"
#include "IClient.h"
#include <mutex>
#include "SafeQueue.h"
#include "VarMemPool.h"
#include <atomic>
class DLL_API SelectClient_Thread :public IClient {
protected:
	VarMemPool& _memoryPool;

	SOCKET _sock;
	std::mutex _sockMutex;
	IQueue<MsgHead*>* _recvQueue;
	IQueue<QMSend>* _sendQueue;
	
	IOBuffer _ioSend;
	OBuffer _oMsg;
	char _sendBuf[CLIENT_SEND_BUFFER_SIZE];

	OBuffer _oRecv;
	int _retainSize;
	char _recvBuf[CLIENT_RECV_BUFFER_SIZE];
	char _package[PACKAGE_MAX_SIZE];

	ThreadSafeType _safeType;

#ifdef DEBUG_CLIENT_PERFORMANCE
public:
	//测试使用
	int *_testMsgTimes;
	int *_testRecvQueueSize;
	int *_testSendTimes;
	int *_testSendQueueSize;
#endif

private:
	int ProcessorSend();
	int ProcessorRecv();
	void ClearQueue();
	void ClearBuffer();
public:
	int RecvThread();
	int SendThread();
public:
	SelectClient_Thread(VarMemPool& memoryPool);
	virtual ~SelectClient_Thread();
	int Connect(const char* ip, unsigned short port);
	void Close();
	bool IsRun();
	int OnRun();
	void OnRecvMsg(MsgHead* msg);
	int SendMsg(MsgHead* msg);
	int SendData(char* data, USHORT dataLen);
};
#endif // !__SELECTCLIENT_H__
