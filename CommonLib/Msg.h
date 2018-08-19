#ifndef __MSG__
#define __MSG__
#include <string>
#include "SocketHeader.h"
enum MsgID {
	Error,
	Info,
	Unknow,
	Login,
	Logout,
	NewClientLogin,
	NewClientConn,
	Test,
};

struct MsgHead {
	unsigned short size;
	unsigned short msgId;
	unsigned short code;
	MsgHead() :size(sizeof(MsgHead)),msgId(MsgID::Error),code(0){
	}
	MsgHead(unsigned short id, unsigned short c=0):msgId(id), size(sizeof(MsgHead)), code(c) {
	}
};

struct MsgNewClientLoginResp :public MsgHead {
	char userId[256];
	MsgNewClientLoginResp(const char* id) {
		size = sizeof(MsgNewClientLoginResp);
		msgId = MsgID::NewClientLogin;
		memcpy(userId, id, strlen(id) + 1);
	}
};

struct MsgNewClientConnectResp:public MsgHead {
	SOCKET sock;
	MsgNewClientConnectResp(SOCKET s) {
		size = sizeof(MsgNewClientConnectResp);
		msgId = MsgID::NewClientConn;
		sock = s;
	}
};

//struct MsgTestReq : public MsgHead {
//	char test1[100];
//	int  reqTimes;
//	MsgTestReq(const char* t1,int times) {
//		Init(t1,times);
//	}
//	void Init(const char* t1, int times) {
//		size = sizeof(MsgTestReq);
//		msgId = MsgID::Test;
//		strcpy(test1, t1);
//		reqTimes = times;
//	}
//};

struct MsgLoginReq :public MsgHead{
	char userId[256];
	char passwd[256];
	MsgLoginReq() {
		size = sizeof(MsgLoginReq);
		msgId = MsgID::Login;
	}
	MsgLoginReq(const char* id,const char* psw) {
		size = sizeof(MsgLoginReq);
		msgId = MsgID::Login;
		strcpy(userId, id);
		strcpy(passwd, psw);
	}
};

struct MsgLoginResp :public MsgLoginReq {
	MsgLoginResp(const char* id, const char* psw):
		MsgLoginReq(id, psw) {
		size = sizeof(MsgLoginResp);
		msgId = MsgID::Login;
	}
};

struct MsgLogoutResp :public MsgHead {

};

struct MsgLogoutReq :public MsgHead{
	char userId[256];
	MsgLogoutReq(const char* id) {
		size = sizeof(MsgLogoutReq);
		msgId = MsgID::Logout;
		strcpy(userId, id);
	}
};

struct MsgInfoResp :public MsgHead{
	int age;
	char name[128];
	MsgInfoResp(int a,const char *n) {
		size = sizeof(MsgInfoResp);
		msgId = MsgID::Info;
		age = a;
		strcpy(name, n);
	}
};

struct MsgUnknowResp :public MsgHead{
	char content[128];
	MsgUnknowResp(const char* n) {
		size = sizeof(MsgUnknowResp);
		msgId = MsgID::Unknow;
		strcpy(content, n);
	}
};

#endif