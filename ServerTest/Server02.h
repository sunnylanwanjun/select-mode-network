#ifndef __SERVER02H__
#define __SERVER02H__
#include "SelectServer_Thread.h"
#include "Timestamp.h"
class Server02 {
public:
private:
	class ServerHandle :public SelectServer_Thread {
	public:
		ServerHandle();
		void OnMsgHandle(IMsgSend* msgSender,MsgHead* msg);
	};

	bool _runing;
	void inputThread();
public:
	Server02();
	~Server02();
};
#endif