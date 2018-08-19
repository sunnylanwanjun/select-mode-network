#ifndef __ICLIENTH__
#define __ICLIENTH__
#include "Msg.h"
#include "Macro.h"
class DLL_API IClient {
public:
	virtual ~IClient(){}
	virtual int Connect(const char* ip, unsigned short port) = 0;
	virtual void Close() = 0;
	virtual bool IsRun() = 0;
	virtual int OnRun() = 0;
	virtual void OnRecvMsg(MsgHead* msg) = 0;
	virtual int SendMsg(MsgHead* msg) = 0;
};
#endif // !__ICLIENTH__
