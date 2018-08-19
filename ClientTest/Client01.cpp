#include "Client01.h"
#include "Utils.h"
#include <thread>
#include "ConfigHelp.h"
#include "Common.h"

Client01::ClientHandle::ClientHandle(VarMemPool& memoryPool):SelectClient_Thread(memoryPool){

}

void Client01::ClientHandle::OnRecvMsg(MsgHead* msg) {
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

Client01::Client01():_memoryPool(ThreadSafeType::Safe), _client(nullptr){

	_client = new ClientHandle(_memoryPool);

	ConfigHelp configHelp;
	configHelp.init("ClientConfig.txt");

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

	_client->Connect(_serverIP.c_str(), _port);
	//这里如果要传引用，只能通过std::ref来传，老版本如果不使用std::ref会直接传值
	//比较坑，新版本，如果不使用std::ref来传，会编译不通过
	std::thread inputHandle(&Client01::inputThread, this, _client);
	inputHandle.detach();

	std::thread thRecv(&Client01::RecvThread, this);
	thRecv.detach();
	std::thread thSend(&Client01::SendThread, this);
	thSend.detach();

	while (_client->IsRun()) {
		_client->OnRun();
	}
}

void Client01::SendThread() {
	while (_client->IsRun()) {
		_client->SendThread();
	}
}

void Client01::RecvThread() {
	while (_client->IsRun()) {
		_client->RecvThread();
	}
}

Client01::~Client01() {
	delete _client;
	_client = nullptr;
}

void Client01::inputThread(ClientHandle* client) {
	char inputBuf[512] = {};
	while (true) {
		memset(inputBuf, 0, sizeof(inputBuf));
		scanf("%s", inputBuf);
		if (client->IsRun()) {
			if (-1 == processorSend(inputBuf, client)) {
				break;
			}
		}
		else {
			break;
		}
	}
}

int Client01::processorSend(char* inputBuf, ClientHandle* client) {
	if (0 == strcmp_ex(inputBuf, "exit")) {
		LOG("client close by user\n");
		client->Close();
		return -1;
	}
	else if (0 == strcmp_ex(inputBuf, "loginall")) {
		MsgLoginReq login;
		for (int i = 0; i<10000; i++) {
			sprintf(login.userId, "lwj%d", i);
			sprintf(login.passwd, "psw%d", i);
			client->SendMsg(&login);
		}
	}
	else if (0 == strcmp_ex(inputBuf, "login")) {
		std::vector<std::string> res = strsplit(inputBuf, "|");
		if (res.size() < 3) {
			LOG("please input login|xx|xx format\n");
			return 0;
		}
		MsgLoginReq login(res[1].c_str(), res[2].c_str());
		client->SendMsg(&login);
	}
	else if (0 == strcmp_ex(inputBuf, "logout")) {
		std::vector<std::string> res = strsplit(inputBuf, "|");
		if (res.size() < 2) {
			LOG("please input logout|xx format\n");
			return 0;
		}
		MsgLogoutReq logout(res[1].c_str());
		client->SendMsg(&logout);
	}
	else if (0 == strcmp_ex(inputBuf, "getInfo")) {
		MsgHead info(MsgID::Info);
		client->SendMsg(&info);
	}
	else {
		MsgHead info(MsgID::Unknow);
		client->SendMsg(&info);
	}

	return 0;
}