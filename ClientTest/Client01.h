#ifndef __CLIENT01H__
#define __CLIENT01H__
#include "SelectClient_Thread.h"
#include "VarMemPool.h"
class Client01 {
private:
	class ClientHandle :public SelectClient_Thread {
	public:
		ClientHandle(VarMemPool& memoryPool);
		void OnRecvMsg(MsgHead* msg);
	};
	ClientHandle* _client;
	VarMemPool _memoryPool;
public:
	Client01();
	~Client01();
	void inputThread(ClientHandle* client);
	int processorSend(char* inputBuf, ClientHandle* client);
	void SendThread();
	void RecvThread();
};
#endif