#ifndef __CLIENT02H__
#define __CLIENT02H__
#include "SelectClient.h"
#include "Timestamp.h"
#include <thread>
class Client02 {
private:
	class ClientHandle :public SelectClient {
	public:
		void OnRecvMsg(MsgHead* msg);
	};
	int _CLIENTNUM;
	bool _runing;
	void inputThread();
public:
	Client02();
	~Client02();
};
#endif