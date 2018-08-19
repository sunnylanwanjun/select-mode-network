#include "Client04.h"
#include "Utils.h"
#include "ConfigHelp.h"
#include "IOBuffer.h"
#include "Timestamp.h"
#include <iostream>
#include "Common.h"

/*
如果机器是4核心的，那么把客户端和服务器都放在一起，最好是服务器开2个线程
客户端开2个线程，这样连接的速度是最快，否则cpu需要不断的切换时间片，导致性
能下降，当然，如果机器上只放置服务器，那么可以开满cpu的核心
*/

Client04::ClientMgr::ClientMgr(
	int clientNum,
	const std::string& serverIP,
	int port,
	const char* msgData,
	int msgLen):
	_clientNum(clientNum), 
	_clientArr(nullptr),
	_threadStop(false),
	_msgData(0),
	_msgLen(msgLen),
	_serverIP(serverIP),
	_port(port),
	_runing(true),
	_memPool(ThreadSafeType::UnSafe){

#ifdef DEBUG_CLIENT_PERFORMANCE
	_testMsgTimes = 0;
	_testRecvQueueSize = 0;
	_testSendTimes = 0;
	_testSendQueueSize = 0;
	_testClientCount = 0;
#endif

	_msgData = new char[_msgLen];
	memcpy(_msgData, msgData, _msgLen);
}

void Client04::ClientMgr::Init() {

	_clientArr = new ClientHandle*[_clientNum];
	memset(_clientArr, 0, sizeof(ClientHandle*)*_clientNum);

	int suceessNum = 0;
	for (int i = 0; i < _clientNum; i++) {
		if (_runing) {
			_clientArr[i] = new ClientHandle(_memPool);

#ifdef DEBUG_CLIENT_PERFORMANCE
			_clientArr[i]->_testMsgTimes = &_testMsgTimes;
			_clientArr[i]->_testRecvQueueSize = &_testRecvQueueSize;
			_clientArr[i]->_testSendTimes = &_testSendTimes;
			_clientArr[i]->_testSendQueueSize = &_testSendQueueSize;
#endif

			if (-1 == _clientArr[i]->Connect(_serverIP.c_str(), _port)) {
				LOG("client connect failed index:%d\n", i);
			}
			else {
				++suceessNum;
#ifdef DEBUG_CLIENT_PERFORMANCE
				_testClientCount++;
#endif
			}
		}
		else {
			break;
		}
	}
	LOG("create client success num:%d,total num:%d\n", suceessNum, _clientNum);
}

void Client04::ClientMgr::Start() {
	std::thread th(&ClientMgr::HandleThread, this);
	th.detach();
}

Client04::ClientMgr::~ClientMgr() {
	_runing = false;
	while (!_threadStop);
	for (int i = 0; i < _clientNum; i++) {
		if (_clientArr[i]) {
			delete _clientArr[i];
			_clientArr[i] = nullptr;
		}
	}
	delete[] _clientArr;
	_clientArr = nullptr;
	delete[] _msgData;
	_msgData = nullptr;
}

void Client04::ClientMgr::HandleThread() {
	while (_runing) {
		int failedNum = 0;
		for (int i = 0; i < _clientNum; i++) {
			if (_clientArr[i] == nullptr) {
				failedNum++;
				continue;
			}
			if (!_clientArr[i]->IsRun()) {
				failedNum++;
				//LOG("client socket invalid index:%d\n", i);
				delete _clientArr[i];
				_clientArr[i] = nullptr;
#ifdef DEBUG_CLIENT_PERFORMANCE
				_testClientCount--;
#endif
				continue;
			}
			_clientArr[i]->SendData(_msgData, _msgLen);
			_clientArr[i]->SendThread();
			_clientArr[i]->RecvThread();
			_clientArr[i]->OnRun();
		}
		if (failedNum == _clientNum) {
			LOG("client send all msg failed\n");
			break;
		}
	}
	LOG("thread stop\n");
	_threadStop = true;
}

void Client04::inputThread() {
	char inputBuf[512] = {};
	while (true) {
		memset(inputBuf, 0, sizeof(inputBuf));
		scanf("%s", inputBuf);
		_runing = false;
		break;
	}
}

Client04::Client04() :
	_runing(true){

	ConfigHelp configHelp;
	configHelp.init("ClientConfig.txt");

	//读取客户端数量
	int _CLIENTNUM = configHelp.getInt("ClientNum");
	if (_CLIENTNUM < 1) {
		LOG("ClientConfig ClientNum Format illegal\n");
		return;
	}

	//读取线程数量
	int _THREADNUM = configHelp.getInt("ThreadNum");
	if (_THREADNUM < 1) {
		LOG("ClientConfig ThreadNum Format illegal \n");
		return;
	}

	//读取服务器IP
	std::string _serverIP = configHelp.getString("ServerIP");
	if (_serverIP == "" || _serverIP.empty()) {
		LOG("ClientConfig ServerIP Format illegal \n");
		return;
	}

	//读取服务器端口
	int _port = configHelp.getInt("Port");
	if (_port < 0) {
		LOG("ClientConfig Port Format illegal \n");
		return;
	}

	//读取消息长度
	int msgLen = configHelp.getInt("MsgLen");
	if (msgLen <= 0) {
		LOG("ClientConfig MsgLen Format illegal \n");
		return;
	}

	//读取消息数量
	int msgNum = configHelp.getInt("MsgNum");
	if (msgNum <= 0) {
		LOG("ClientConfig MsgNum Format illegal \n");
		return;
	}

	//读取消息内容
	std::string msgContent = configHelp.getString("MsgContent");
	if (msgContent == "") {
		LOG("ClientConfig MsgContent Format illegal \n");
		return;
	}

	const char* words = msgContent.c_str();

	int _msgTotalLen = msgLen * msgNum;
	char* _msgData = new char[_msgTotalLen];
	memset(_msgData, 0, _msgTotalLen);
	
	IBuffer iBuffer;
	iBuffer.setBuffer(_msgData, _msgTotalLen);
	for (int i = 0; i < msgNum; i++) {
		iBuffer.seek(i*msgLen);
		iBuffer.writeUShort(msgLen);
		iBuffer.writeUShort(MsgID::Test);
		iBuffer.writeUShort(0);
		iBuffer.writeBytes(words,strlen(words)+1);
	}

	int _eachClientNum = _CLIENTNUM / _THREADNUM;
	LOG("will create client num:%d\nthread num:%d\neachClientNum:%d\nserverIP:%s\nport:%d\nMsgLen:%d\nMsgNum:%d\nMsgContent:%s\n", _CLIENTNUM, _THREADNUM, _eachClientNum,_serverIP.c_str(),_port,msgLen,msgNum,words);

#ifdef _WIN32
	system("Pause");
#endif

	std::thread inputHandle(&Client04::inputThread, this);
	inputHandle.detach();

	_clientArr = new ClientMgr*[_THREADNUM];
	for (int i = 0; i < _THREADNUM; i++) {
		_clientArr[i] = new ClientMgr(_eachClientNum,_serverIP,_port,_msgData, _msgTotalLen);
		_clientArr[i]->Init();
	}

	for (int i = 0; i < _THREADNUM; i++) {
		_clientArr[i]->Start();
	}

	Timestamp time;
	while (_runing) {
		////////////////////////////////////////////////////
		//测试收包次数
#ifdef DEBUG_CLIENT_PERFORMANCE
		double goTime = time.GetS();
		if (goTime >= 1.0) {
			
			static int _testMsgTimes;
			static int _testRecvQueueSize;
			static int _testSendTimes;
			static int _testSendQueueSize;
			static int _testClientCount;
			static MemInfo memInfo;

			memset(&memInfo, 0, sizeof(MemInfo));
			_testMsgTimes = 0;
			_testRecvQueueSize = 0;
			_testSendTimes = 0;
			_testSendQueueSize = 0;
			_testClientCount = 0;
			for (int i = 0; i < _THREADNUM; i++) {
				if (_clientArr[i]) {
					_clientArr[i]->GetMemInfo(&memInfo);
					_testMsgTimes += _clientArr[i]->_testMsgTimes;
					_testRecvQueueSize += _clientArr[i]->_testRecvQueueSize;
					_testSendTimes += _clientArr[i]->_testSendTimes;
					_testSendQueueSize += _clientArr[i]->_testSendQueueSize;
					_testClientCount += _clientArr[i]->_testClientCount;

					_clientArr[i]->_testMsgTimes = 0;
					_clientArr[i]->_testSendTimes = 0;
				}
			}

			std::cout << "recv:" << _testMsgTimes;// (int)(_testMsgTimes / goTime);
			std::cout << ",recvSize:" << _testRecvQueueSize;

			std::cout << ",send:" << _testSendTimes;// (int)(_testSendTimes / goTime);
			std::cout << ",sendSize:" << _testSendQueueSize;

			std::cout << ",memInfo:" << memInfo.freeMem / 1000 << "k" << "/" << memInfo.totalMem / 1000 << "k";

			std::cout << ",client:" << _testClientCount;
			std::cout << ",time:" << goTime << std::endl;
			
			time.Update();
		}
#endif
		////////////////////////////////////////////////////
	}

	for (int i = 0; i < _THREADNUM; i++) {
		if (_clientArr[i]) {
			delete _clientArr[i];
			_clientArr[i] = nullptr;
		}
	}
	delete[] _clientArr;
	_clientArr = nullptr;
}

Client04::~Client04() {
	
}