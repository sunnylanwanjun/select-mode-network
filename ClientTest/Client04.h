#ifndef __CLIENT04H__
#define __CLIENT04H__
#include "SelectClient_Thread.h"
#include <thread>
#include <fstream>
#include <atomic>
class Client04 {
private:
	class ClientHandle :public SelectClient_Thread {
	public:
		ClientHandle(VarMemPool& memPool):SelectClient_Thread(memPool){

		}
	};
	class ClientMgr {
	private:
		void HandleThread();
	public:
		ClientMgr(int clientNum,
			const std::string& serverIP,
			int port,
			const char* msgData,
			int msgLen);
		~ClientMgr();
		void Init();
		void Start();
		//测试使用
		void GetMemInfo(MemInfo* memInfo) {
			_memPool.GetInfo(memInfo);
		}
#ifdef DEBUG_CLIENT_PERFORMANCE
	public:
		//测试使用
		int _testMsgTimes;
		int _testRecvQueueSize;
		int _testSendTimes;
		int _testSendQueueSize;
		int _testClientCount;
#endif
	private:
		ClientHandle** _clientArr;
		int _clientNum;
		bool _threadStop;
		bool _runing;
		std::string _serverIP;
		int  _port;
		char* _msgData;
		int   _msgLen;
		VarMemPool _memPool;
	};

	bool _runing;
	ClientMgr** _clientArr;
	void inputThread();
public:
	Client04();
	~Client04();
};
#endif