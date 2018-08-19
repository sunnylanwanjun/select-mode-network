#ifndef __SERVER01H__
#define __SERVER01H__
#include "SelectServer.h"
#include "Timestamp.h"
#include <thread>
class Server01 {
private:
	class ServerHandle :public SelectServer {
	public:
		ServerHandle();
		void OnMsgHandle(IMsgSend* msgSender, MsgHead* msg);
	};

	bool _runing;
	void inputThread();
public:
	Server01();
	~Server01();
};
#endif