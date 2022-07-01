#include "Server01.h"
#include "ConfigHelp.h"

Server01::ServerHandle::ServerHandle() {
	
}

void Server01::ServerHandle::OnMsgHandle(IMsgSend* msgSender, MsgHead* msg) {
	//LOG("OnRecvMsg clientSock:%d msgId:%d,msgSize:%d,msgCode:%d\n", clientSock, msg->msgId, msg->size, msg->code);
	switch (msg->msgId) {
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

			MsgNewClientLoginResp newClient(login->userId);
			std::map<IMsgSend*, bool> excluteSock;
			excluteSock[msgSender] = true;
			Broadcast(&newClient, excluteSock);

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
			// excluteSock[msgSender] = true;
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

void Server01::inputThread() {
	char inputBuf[512] = {};
	while (true) {
		memset(inputBuf, 0, sizeof(inputBuf));
		scanf("%s", inputBuf);
		_runing = false;
		break;
	}
}

Server01::Server01() :_runing(true) {
	std::thread inputHandle(&Server01::inputThread, this);
	inputHandle.detach();

	ConfigHelp configHelp;
	configHelp.init("ServerConfig.txt");

	ServerHandle server;

	if (-1 == server.Bind(INADDR_ANY, configHelp.getInt("Port"))) {
		return;
	}

	if (-1 == server.Listen(configHelp.getInt("Backlog"))) {
		return;
	}

	while (_runing) {
		server.OnRun();
	}
}

Server01::~Server01() {

}