#include "Client02.h"
#include "ConfigHelp.h"

void Client02::ClientHandle::OnRecvMsg(MsgHead* msg) {
	LOG("recv msg len:%d,msgId:%d,msgCode:%d\n", msg->size, msg->msgId, msg->code);
	switch (msg->msgId) {
	case MsgID::Info: {
		MsgInfoResp *info = (MsgInfoResp*)msg;
		LOG("recv MsgInfo age:%d,name:%s\n", info->age, info->name);
		break;
	}
	case MsgID::Unknow: {
		MsgUnknowResp *unknow = unknow = (MsgUnknowResp*)msg;
		LOG("recv MsgUnknow name:%s\n", unknow->content);
		break;
	}
	case MsgID::Login: {
		MsgLoginResp* login = (MsgLoginResp*)msg;
		if (msg->code == 0) {
			LOG("login server success userId:%s,psw:%s\n", login->userId, login->passwd);
		}
		else {
			LOG("login server failed\n");
		}
		break;
	}
	case MsgID::Logout: {
		if (msg->code == 0) {
			LOG("logout server success\n");
		}
		else {
			LOG("logout server failed\n");
		}
		break;
	}
	case MsgID::NewClientLogin: {
		MsgNewClientLoginResp *newClient = (MsgNewClientLoginResp*)msg;
		LOG("new client userId is:%s\n", newClient->userId);
	}
	default:
		break;
	}
}

void Client02::inputThread() {
	char inputBuf[512] = {};
	while (true) {
		memset(inputBuf, 0, sizeof(inputBuf));
		scanf("%s", inputBuf);
		_runing = false;
		break;
	}
}

Client02::Client02() :
	_runing(true),
	_CLIENTNUM(0)
{	
	ConfigHelp configHelp;
	configHelp.init("ClientConfig.txt");

	//读取客户端数量
	_CLIENTNUM = configHelp.getInt("ClientNum");
	if (_CLIENTNUM < 1) {
		LOG("ClientConfig ClientNum Format illegal\n");
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
		iBuffer.writeBytes(words, strlen(words) + 1);
	}

	LOG("will create client num:%d\nserverIP:%s\nport:%d\nMsgLen:%d\nMsgNum:%d\nMsgContent:%s\n", _CLIENTNUM, _serverIP.c_str(), _port, msgLen, msgNum, words);

	ClientHandle** _clientArr = new ClientHandle*[_CLIENTNUM];

	std::thread inputHandle(&Client02::inputThread, this);
	inputHandle.detach();

	Timestamp time;

	int clientNum = 1;
	for (int i = 0; i < _CLIENTNUM; i++) {
		if (_runing) {
			_clientArr[i] = new ClientHandle();
			if (-1 == _clientArr[i]->Connect(_serverIP.c_str(), _port)) {
				LOG("client connect failed index:%d\n", i);
			}
			else {
				LOG("client create num:%d\n", ++clientNum);
			}
		}
	}
	LOG("create client total num:%d\n", clientNum);

	long long secondCount = 0;

	while (_runing) {
		int failedNum = 0;
		for (int i = 0; i < _CLIENTNUM; i++) {
			if (_clientArr[i] == nullptr) {
				//LOG("client handle is null idx:%d\n", i);
				failedNum++;
				continue;
			}
			if (!_clientArr[i]->IsRun()) {
				failedNum++;
				LOG("client socket invalid index:%d\n", i);
				delete _clientArr[i];
				_clientArr[i] = nullptr;
				continue;
			}
			_clientArr[i]->OnRun();
			//从日志可以看出，当FD_SETSIZE为100，却有200个客户端时
			//第101个客户端发送完1447896字节后，就无法再发送数据，说明
			//第个socket缓冲区的大小为1447896。
			//LOG("client %d begin to send msg,has send:%d\n", i, _clientSendSize[i]);
			if (-1 == _clientArr[i]->SendData(_msgData,_msgTotalLen)) {
				LOG("client send msg failed %d\n", i);
				failedNum++;
			}
			else {
				secondCount++;
				double sec = time.GetS();
				if (sec >= 1.0) {
					LOG("send count %lld,time:%f\n", secondCount, sec);
					secondCount = 0;
					time.Update();
				}
				//LOG("send times %d\n", login.reqTimes);
			}
		}
		if (failedNum == _CLIENTNUM) {
			LOG("client send all msg failed\n");
			break;
		}
		//if (times == 1000) {
		//WSAStarup多少次，WSACleanup就执行多少次
		//如果WSACleanup执行次数多了，那么所有的socket就关了
		//WSACleanup();
		//WSACleanup();
		//LOG("wsacleanup\n");
		//break;
		//}
	}

	for (int i = 0; i < _CLIENTNUM; i++) {
		if (_clientArr[i] != nullptr) {
			delete _clientArr[i];
			_clientArr[i] = nullptr;
		}
	}
	delete[] _clientArr;
	_clientArr = nullptr;
	delete[] _msgData;
	_msgData = nullptr;
}

Client02::~Client02() {

}
