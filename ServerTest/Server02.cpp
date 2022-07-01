#include "Server02.h"
#include "ConfigHelp.h"
Server02::ServerHandle::ServerHandle() {
	
}

void Server02::ServerHandle::OnMsgHandle(IMsgSend* msgSender, MsgHead* msg){
	//LOG("OnRecvMsg clientSock:%d msgId:%d,msgSize:%d,msgCode:%d\n", clientSock, msg->msgId, msg->size, msg->code);
	double goTime;
	switch (msg->msgId) {
		case MsgID::Test:{
			msgSender->SendMsg(msg);
			break;
		}
		case MsgID::Info: {
			MsgInfoResp info(80, "lwj");
			msgSender->SendMsg(&info);
			break;
		}
		case MsgID::Login: {
			MsgLoginReq* login = (MsgLoginReq*)msg;
			LOG("userId:%s,pwd:%s req login\n",login->userId, login->passwd);
			MsgLoginResp loginResp(login->userId, login->passwd);
			msgSender->SendMsg(&loginResp);

			/*
			MsgNewClientResp newClient((int)clientSock);
			std::map<SOCKET, bool> excluteSock;
			excluteSock[clientSock] = true;
			broadcast(&newClient, &excluteSock);
			*/

			break;
		}
		case  MsgID::Logout: {
			MsgLogoutReq* logout = (MsgLogoutReq*)msg;
			LOG("userId:%s req logout\n",logout->userId);
			MsgHead head(MsgID::Logout);
			msgSender->SendMsg(&head);
			break;
		}
		case MsgID::Broadcast: {
			std::map<IMsgSend*, bool> excluteSock;
			excluteSock[msgSender] = true;
			Broadcast(msg, excluteSock);
			break;
		}
		default: {
			MsgUnknowResp info("what do you want to do");
			msgSender->SendMsg(&info);
			break;
		}
	}
}

void Server02::inputThread() {
	char inputBuf[512] = {};
	while (true) {
		memset(inputBuf, 0, sizeof(inputBuf));
		scanf("%s", inputBuf);
		_runing = false;
		break;
	}
}

Server02::Server02() :_runing(true) {
	ConfigHelp configHelp;
	configHelp.init("ServerConfig.txt");

	ServerHandle server;
	if (-1 == server.InitServer(configHelp.getInt("ThreadNum"))) {
		return;
	}

	if (-1 == server.Bind(INADDR_ANY, configHelp.getInt("Port"))) {
		return;
	}

	if (-1 == server.Listen(configHelp.getInt("Backlog"))) {
		return;
	}

	std::thread inputHandle(&Server02::inputThread, this);
	inputHandle.detach();

	while (_runing&&server.IsRun()) {
		server.OnRun();
	}
}

Server02::~Server02() {

}