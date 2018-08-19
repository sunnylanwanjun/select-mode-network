#ifndef __CLIENTSOCKETH__
#define __CLIENTSOCKETH__
#include "IMsgRecv.h"
#include "IMsgSend.h"
#include "IOBuffer.h"
#include "Macro.h"
#include <functional>
#include "SocketHeader.h"
#include "Msg.h"
#include "SafeQueue.h"
#include "VarMemPool.h"
#include <atomic>
#include "IMsgHandle.h"
#include <mutex>

/*
非线程安全
1 ProcessorRecv ProcessorSend都会调用close，所以此处线程不安全
2 _memeryPool由外部传入，所以如果需要保证线程安全，这个池子必须是线程安全的
*/
class DLL_API ClientSocket:public IMsgRecv,public IMsgSend{	
private:
	OBuffer _oRecv;
	int _retainSize;
	char _recvBuf[SERVER_RECV_BUFFER_SIZE];
	IMsgHandle* _msgHandle;
	char _package[PACKAGE_MAX_SIZE];

	IQueue<QMSend>* _sendQueue;//发送消息队列
	VarMemPool& _memeryPool;

	IOBuffer _ioSend;
	OBuffer _oMsg;
	char _sendBuf[SERVER_SEND_BUFFER_SIZE];

	ThreadSafeType _safeType;
	std::mutex _closeMu;
	int _refCount;

private:
	int SendToKernelBuf();
	void ClearQueue();
	void ClearBuffer();
public:
	
#ifdef DEBUG_SERVER_PERFORMANCE
public:
	//测试使用
	int *_testMsgTimes;
	int *_testSendTimes;
	int *_testSendQueueSize;
	int *_testRecvTimes;
#endif
	
public:
	SOCKET _clientSock;

	static ClientSocket* Create(SOCKET sock, IMsgHandle* msgHandle, VarMemPool& memeryPool);
	void Release();

	ClientSocket(SOCKET sock, IMsgHandle* msgHandle, VarMemPool& memeryPool);
	~ClientSocket();
	
	int ProcessorRecv();
	int ProcessorSend();

	int SendData(char* data,USHORT size);
	int SendMsg(MsgHead* msg);
	
	void Close();
};
#endif